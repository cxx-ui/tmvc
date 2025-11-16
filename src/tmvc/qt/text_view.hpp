// Copyright (c) 2025, Alexandr Esilevich
//
// Distributed under the Boost Software License.
// See accompanying file LICENSE for license information.
//

/// \file text_view.hpp
/// Contains definition of the basic_advanced_viewer_view class and associated type aliases.

#pragma once

#include "impl/key_controller.hpp"
#include "impl/string.hpp"
#include "impl/viewport_model.hpp"
#include "../edit_controller.hpp"
#include "../position.hpp"
#include "../signals.hpp"
#include "../text_model.hpp"

#include <memory>

#include <QAbstractScrollArea>
#include <QApplication>
#include <QFrame>
#include <QMenu>
#include <QPainter>
#include <QPlainTextEdit>
#include <QScrollBar>
#include <QTextLayout>
#include <QTimer>


namespace tmvc::qt {


namespace impl {
    template <text_model TextModel>
    struct text_view_max_line_size_signal_connection {
    };

    template <text_model_with_max_line_size TextModel>
    struct text_view_max_line_size_signal_connection<TextModel> {
        scoped_signal_connection max_line_size_changed_con;
    };
}


/// View for displaying and editing content of the text model with controller.
/// Implemented using custom drawing of text and can display very large or infinite text models.
template <
    text_model TextModel,
    selection_model SelectionModel,
    viewport_position_model ViewportPositionModel,
    selection_controller_for<TextModel> Controller
>
class text_view: public QAbstractScrollArea,
                 private impl::text_view_max_line_size_signal_connection<TextModel> {
public:
    /// Type of text model
    using text_model_t = TextModel;

    /// Type of controller
    using controller_t = Controller;

    /// Type of selection model
    using selection_model_t = SelectionModel;

    /// Type of viewport position model
    using viewport_position_model_t = ViewportPositionModel;

    /// Type of character
    using char_t = typename TextModel::char_t;

private:
    /// True if text view is editable
    static constexpr bool is_editable = edit_controller<Controller>;

public:
    /// Constructs text view with specified references to text and selection models,
    /// reference to viewport position model, and reference to controller and reference
    text_view(const TextModel & txt,
              selection_model_t & sel,
              viewport_position_model_t & v_pos,
              Controller & cntrl,
              QWidget * parent = nullptr):
    QAbstractScrollArea{parent},
    text_{txt},
    selection_{sel},
    viewport_pos_{v_pos},
    controller_{cntrl},
    viewport_mdl_{text_, selection_, viewport_pos_, font()} {
        init();
    }


    /// Returns text left margin
    float left_margin() const {
        return viewport_mdl_.left_margin();
    }


    /// Sets text left margin
    void set_left_margin(float margin) {
        viewport_mdl_.set_left_margin(margin);
    }


    /// Returns text right margin
    float right_margin() const {
        return viewport_mdl_.right_margin();
    }


    /// Sets text right margin
    void set_right_margin(float margin) {
        viewport_mdl_.set_right_margin(margin);
    }


    /// Returns text top margin
    float top_margin() const {
        return viewport_mdl_.top_margin();
    }


    /// Sets text right margin
    void set_top_margin(float margin) {
        viewport_mdl_.set_top_margin(margin);
    }



protected:
    /// Handles viewport's paint event and repaints text
    void paintEvent(QPaintEvent * event) override {
        QPainter painter{viewport()};

        auto h_offset = -static_cast<qreal>(horizontalScrollBar()->value());
        QPointF draw_pos{h_offset, 0.0};

        QFontMetrics font_metrics{font()};

        // drawing previously layed out visible lines with cursor and selection
        uint64_t line_idx = viewport_mdl_.vertical_pos();
        for (auto && line_layout : text_layout_) {

            // calculating selections for drawing
            std::vector<std::pair<uint64_t, uint64_t>> line_selections;
            for (auto && sel : selection_.selections()) {
                if (sel.start.line == line_idx) {
                    uint64_t sel_start = sel.start.column;
                    uint64_t sel_end = 0;

                    if (sel.end.line == line_idx) {
                        // the selection is in single line
                        sel_end = sel.end.column;
                    } else {
                        // first line of multiine selection
                        sel_end = text_.line_size(line_idx) + 1;
                    }

                    line_selections.push_back({sel_start, sel_end});

                } else if (sel.end.line == line_idx) {
                    // last line of multiline selection
                    line_selections.push_back({0, sel.end.column});
                } else if (sel.start.line < line_idx && line_idx < sel.end.line) {
                    // middle of selection
                    line_selections.push_back({0, text_.line_size(line_idx) + 1});
                }
            }

            // creating list of formats for selection
            QVector<QTextLayout::FormatRange> formats;
            for (auto && sel : line_selections) {
                assert(sel.first <= sel.second && "invalid selection position");
                assert(sel.first <= text_.line_size(line_idx) && "invalid selection position");
                assert(sel.second <= text_.line_size(line_idx) + 1 && "invalid selection position");
                QTextCharFormat sel_format;
                sel_format.setBackground(palette().highlight());
                sel_format.setForeground(palette().highlightedText());
                QTextLayout::FormatRange sel_fmt_range;
                sel_fmt_range.format = sel_format;
                sel_fmt_range.length = static_cast<int>(sel.second - sel.first);
                sel_fmt_range.start = static_cast<int>(sel.first);
                formats.push_back(sel_fmt_range);
            }

            // adding cursor ranges for overwrite mode
            if (is_cursor_visible_ && hasFocus() && is_overwrite_mode()) {
                for (auto && cursor : selection_.cursors()) {
                    if (cursor.line != line_idx) {
                        continue;
                    }

                    // adding selection for current symbol to emulate overwrite mode
                    QTextCharFormat sel_format;
                    sel_format.setBackground(palette().text());
                    sel_format.setForeground(palette().base());
                    QTextLayout::FormatRange sel_fmt_range;
                    sel_fmt_range.format = sel_format;
                    sel_fmt_range.length = 1;
                    sel_fmt_range.start = static_cast<int>(cursor.column);
                    formats.push_back(sel_fmt_range);
                }
            }

            // drawing line
            line_layout->draw(&painter, draw_pos, formats);

            // drawing cursor for insert mode
            if (is_cursor_visible_ && hasFocus() && !is_overwrite_mode()) {
                for (auto && cursor : selection_.cursors()) {
                    if (cursor.line != line_idx) {
                        continue;
                    }

                    line_layout->drawCursor(&painter, draw_pos, cursor.column, 1);
                }
            }

            ++line_idx;
        }
    }


    /// Handles widget property change events (font, style, etc)
    void changeEvent(QEvent * event) override {
        // updating font in viewport model
        viewport_mdl_.set_font(font());
    }


    void showEvent(QShowEvent * event) override {
        layout_text_and_update();
    }


    void resizeEvent(QResizeEvent * event) override {
        // using entire widget height to calculate max viewport height
        int max_height = height() - frameWidth() * 2;
        if (max_height < 0) {
            max_height = 0;
        }

        // setting viewport size in viewport model
        viewport_mdl_.set_size(viewport()->width(),
                            viewport()->height(),
                            static_cast<float>(max_height));

        // setting viewport height in controller
        controller_.set_viewport_height(viewport_mdl_.visible_lines_count());
    }


    void mouseMoveEvent(QMouseEvent * event) override {
        // saving last mouse move position
        last_mouse_move_pos_ = event->position();

        // processing event with controller
        auto text_pos = viewport_mdl_.text_pos(event->position().x(),
                                            event->position().y(),
                                            is_overwrite_mode());
        auto modifiers = QApplication::keyboardModifiers();
        controller_.do_mouse_move(text_pos,
                                modifiers & Qt::ControlModifier,
                                modifiers & Qt::ShiftModifier);

        event->accept();
    }


    void mousePressEvent(QMouseEvent * event) override {
        // ignoring all mouse buttons except the left button
        if (event->button() != Qt::LeftButton) {
            QAbstractScrollArea::mousePressEvent(event);
            return;
        }

        // Saving last mouse move position. This is used in updating layout to schedule
        // additional mouse move event after text is scrolled. This is needed to
        // perform continuous text scrolling when cursor is moved beyond widget frame
        is_mouse_pressed_ = true;
        last_mouse_move_pos_ = event->position();

        // processing event with controller
        auto text_pos = viewport_mdl_.text_pos(event->position().x(),
                                            event->position().y(),
                                            is_overwrite_mode());
        auto modifiers = QApplication::keyboardModifiers();
        controller_.do_mouse_press(text_pos,
                                modifiers & Qt::ControlModifier,
                                modifiers & Qt::ShiftModifier);

        // resetting cursor visibility
        reset_cursor_visibility();

        // making event as accepted
        event->accept();
    }


    void mouseReleaseEvent(QMouseEvent * event) override {
        // processing event with controller
        auto text_pos = viewport_mdl_.text_pos(event->position().x(),
                                            event->position().y(),
                                            is_overwrite_mode());
        auto modifiers = QApplication::keyboardModifiers();
        controller_.do_mouse_release(text_pos,
                                    modifiers & Qt::ControlModifier,
                                    modifiers & Qt::ShiftModifier);

        is_mouse_pressed_ = false;
    }


    void keyPressEvent(QKeyEvent * event) override {
        bool event_processed = false;
        if constexpr (edit_controller<Controller>) {
            event_processed = impl::process_edit_key_event(controller_, event);
        } else {
            event_processed = impl::process_selection_key_event(controller_, event);
        }

        if (event_processed) {
            reset_cursor_visibility();
            return;
        }

        bool ctrl = (event->modifiers() & Qt::ControlModifier) != 0;
        bool shift = (event->modifiers() & Qt::ShiftModifier) != 0;

        // processing page up / page down keys
        switch (event->key()) {
        case Qt::Key_PageUp:
            viewport_mdl_.do_page_up(shift);
            controller_.do_page_up(ctrl, shift);
            reset_cursor_visibility();
            event->accept();
            return;
        case Qt::Key_PageDown:
            viewport_mdl_.do_page_down(shift);
            controller_.do_page_down(ctrl, shift);
            reset_cursor_visibility();
            event->accept();
            return;
        default:
            // doing nothing for now
            break;
        }

        event->ignore();
    }


    void contextMenuEvent(QContextMenuEvent * event) override {
        QMenu menu{this};
        add_copy_action(&menu);
        menu.addSeparator();
        add_select_all_action(&menu);
        menu.exec(event->globalPos());
        event->accept();
    }


    /// Handles focus event. Resets cursor blinking
    void focusInEvent(QFocusEvent * event) override {
        reset_cursor_visibility();
        viewport()->update();
    }


protected:
    /// Returns viewport size hint for models with known maximum line size
    QSize viewportSizeHint() const override {
        if constexpr (text_model_with_max_line_size<TextModel>) {
            // calculating size hint for models with max line size
            QFontMetrics metrics{font()};
            QString test_line = QString{"0"}.repeated(text_.max_line_size());
            auto width = metrics.horizontalAdvance(test_line) +
                         viewport_mdl_.left_margin() +
                         viewport_mdl_.right_margin();

            return QSize{static_cast<int>(width), static_cast<int>(width)};
        } else {
            return QAbstractScrollArea::viewportSizeHint();
        }
    }   


    QSize minimumSizeHint() const override {
        auto res = QAbstractScrollArea::minimumSizeHint();

        // don't set minimum width hint if vertical scroll bar is not visible
        if (this->verticalScrollBarPolicy() == Qt::ScrollBarAlwaysOff) {
            res.setWidth(0);
        }

        return res;
    }


private:
    /// Initializes text view
    void init() {
        viewport_mdl_.set_left_margin(4.0f);
        viewport_mdl_.set_right_margin(4.0f);
        viewport_mdl_.set_top_margin(4.0f);

        horizontalScrollBar()->setSingleStep(20);
        viewport()->setCursor(Qt::IBeamCursor);
        setFocusPolicy(Qt::StrongFocus);

        // redrawing text after updating text cursor position
        pos_changed_con_ = selection_.changed.connect([this]() {
            viewport()->update();
            reset_cursor_visibility();
        });

        // listening for layout update in viewport model
        viewport_layout_updated_con_ = viewport_mdl_.layout_updated.connect([this] {
            layout_text_and_update();
        });

        // listening for scrolling vertical scroll bar
        connect(verticalScrollBar(), &QScrollBar::valueChanged, [this](int value) {
            on_vertical_scroll_moved(value);
        });

        // listening for scrolling horizontal scroll bar
        connect(horizontalScrollBar(), &QScrollBar::valueChanged, [this](int value) {
            on_horizontal_scroll_moved(value);
        });

        // creating timer for cursor visibility
        if constexpr (is_editable) {
            cursor_timer_ = new QTimer{this};
            connect(cursor_timer_, &QTimer::timeout, [this] {
                is_cursor_visible_ = !is_cursor_visible_;
                viewport()->update();
            });
            reset_cursor_visibility();
        }

        if constexpr (text_model_with_max_line_size<TextModel>) {
            setSizeAdjustPolicy(QAbstractScrollArea::AdjustToContents);

            this->max_line_size_changed_con = text_.max_line_size_changed.connect([this] {
                this->updateGeometry();
            });
        }
    }


    /// Adds Copy action into  menu
    QAction * add_copy_action(QMenu * menu) const {
        auto copy = menu->addAction("Copy");
        copy->setIcon(QIcon::fromTheme("edit-copy"));
        copy->setShortcut(QKeySequence{QKeySequence::Copy});
        copy->setEnabled(controller_.can_copy());
        connect(copy, &QAction::triggered, [this] { do_copy(); });
        return copy;
    }


    /// Adds Select all action into menu
    QAction * add_select_all_action(QMenu * menu) const {
        auto sel_all = menu->addAction("Select all");
        sel_all->setIcon(QIcon::fromTheme("edit-select-all"));
        sel_all->setShortcut(QKeySequence{QKeySequence::SelectAll});
        connect(sel_all, &QAction::triggered, [this] { controller_.select_all(); });
        return sel_all;
    }


    /// Lays out visible text
    void layout_text() {
        QFontMetricsF font_metrics{font()};
        auto line_height = font_metrics.lineSpacing() + 1.0;

        text_layout_.clear();
        text_layout_.reserve(viewport_mdl_.max_visible_line_index() - viewport_mdl_.vertical_pos() + 1);

        uint64_t line_idx = viewport_mdl_.vertical_pos();
        qreal curr_height = (viewport_mdl_.vertical_pos() == 0) ? viewport_mdl_.top_margin() : 0.0;

        for (auto last_idx = viewport_mdl_.max_visible_line_index(); line_idx <= last_idx; ++line_idx) {
            QPointF pos{viewport_mdl_.left_margin(), curr_height};
            auto line_layout = layout_text_line(line_str(text_, line_idx), font(), pos);
            text_layout_.push_back(std::move(line_layout));
            curr_height += line_height;
        }

        // setting range and value for vertical scroll bar
        verticalScrollBar()->setRange(0, viewport_mdl_.max_vertical_pos());
        verticalScrollBar()->setValue(viewport_mdl_.vertical_pos());

        // setting range and value for horizontal scroll bar
        horizontalScrollBar()->setRange(0, static_cast<int>(viewport_mdl_.max_horizontal_pos()));
        horizontalScrollBar()->setPageStep(viewport_mdl_.text_width());
        horizontalScrollBar()->setValue(static_cast<int>(viewport_mdl_.horizontal_pos()));

        // Scheduling another mouse move event after delay. This is required to continuous
        // text scrolling when cursor is moved beyond the text frame
        if (is_mouse_pressed_) {
            QTimer::singleShot(200, [this] {
                auto text_pos = viewport_mdl_.text_pos(last_mouse_move_pos_.x(),
                                                    last_mouse_move_pos_.y(),
                                                    is_overwrite_mode());
                auto modifiers = QApplication::keyboardModifiers();
                controller_.do_mouse_move(text_pos,
                                        modifiers & Qt::ControlModifier,
                                        modifiers & Qt::ShiftModifier);
            });
        }
    }


    /// Lays out visible text and updates viewport
    void layout_text_and_update() {
        layout_text();
        viewport()->update();
    }


    /// Called when viewport position changed in viewport model
    void on_viewport_pos_changed() {
        // laying out text and updating viewport widget
        layout_text_and_update();

        // updating values of scroll bars
        horizontalScrollBar()->setValue(viewport_mdl_.horizontal_pos());
        verticalScrollBar()->setValue(viewport_mdl_.vertical_pos());
    }


    /// Called after vertical scroll bar was moved
    void on_vertical_scroll_moved(int value) {
        viewport_pos_.set_pos(value);
    }


    /// Called after horizontal scroll bar was moved
    void on_horizontal_scroll_moved(int value) {
        if (viewport_mdl_.horizontal_pos() != static_cast<float>(value)) {
            viewport_mdl_.set_horizontal_pos(static_cast<float>(value));
        }
    }


    /// Copies selected text to clipboard
    void do_copy() const {
        auto clip = QGuiApplication::clipboard();
        clip->setText(impl::std_string_to_qstring(controller_.copy()));
    }


    /// Resets cursor visibility
    void reset_cursor_visibility() {
        if constexpr (is_editable) {
            is_cursor_visible_ = true;
            viewport()->update();
            cursor_timer_->start(QApplication::cursorFlashTime() / 2);
        }
    }


    /// Returns true if view is in overwrite mode
    bool is_overwrite_mode() const {
        if constexpr (edit_controller<Controller>) {
            return controller_.is_overwrite_mode();
        } else {
            return false;
        }
    }


    /// Lays out line of text with specified font and position
    static std::unique_ptr<QTextLayout> layout_text_line(const std::basic_string<char_t> & str,
                                                         const QFont & fnt,
                                                         const QPointF & pos) {

        // creating text layout for line of text
        auto line_layout = std::make_unique<QTextLayout>();
        line_layout->setFont(fnt);
        line_layout->setText(impl::std_string_to_qstring(str));
        QTextOption opt;
        opt.setWrapMode(QTextOption::NoWrap);
        line_layout->setTextOption(opt);

        line_layout->beginLayout();

        // creating single line in text layout
        auto line_layout_line = line_layout->createLine();
        assert(line_layout_line.isValid() && "single text layout line is not valid");
        line_layout_line.setPosition(pos);
        
        // checking that there is no more lines in layout
        auto line2 = line_layout->createLine();
        assert(!line2.isValid() && "text layout must have single line");
        
        line_layout->endLayout();

        return line_layout;
    }


    const text_model_t & text_;                 ///< Reference to text model
    selection_model_t & selection_;             ///< Reference to selection model
    viewport_position_model_t & viewport_pos_;  ///< Reference to viewport position model
    Controller & controller_;                   ///< Reference to controller

    /// Viewport model for this view
    impl::viewport_model<TextModel, selection_model_t, viewport_position_model_t> viewport_mdl_;

    /// Layouts for each visible line
    std::vector<std::unique_ptr<QTextLayout>> text_layout_;

    bool is_mouse_pressed_ = false;             ///< Is mouse button pressed
    QPointF last_mouse_move_pos_;               ///< Last mouse move position
    bool is_cursor_visible_ = true;             ///< True if cursor is visible now
    QTimer * cursor_timer_;                     ///< Timer for cursor blinking

    // connections to model signals
    scoped_signal_connection pos_changed_con_;
    scoped_signal_connection viewport_layout_updated_con_;
    scoped_signal_connection viewport_pos_changed_con_;
};


}
