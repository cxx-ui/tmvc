// Copyright (c) 2025, Alexandr Esilevich
//
// Distributed under the Boost Software License.
// See accompanying file LICENSE for license information.
//

/// \file text_view_impl.hpp
/// Contains definition of the text_view_impl class.

#pragma once

#include "controller.hpp"
#include "selection_model.hpp"
#include "../impl/key_controller.hpp"
#include "../impl/formatted_char.hpp"
#include "../impl/string.hpp"
#include "../impl/utils.hpp"
#include "../../edit_controller.hpp"
#include "../../editable_text_model.hpp"
#include "../../text_model.hpp"
#include <QApplication>
#include <QPlainTextEdit>
#include <QTextEdit>


namespace tmvc::qt::native {


/// Helper base class for text_view_impl, used for selecting user defined controller storage.
template <qt_selection_controller Controller>
struct text_model_impl_controller_base {
    text_model_impl_controller_base(Controller & cntrl):
        cntrl_{cntrl} {}

    Controller & cntrl_;            ///< Reference to user defined controller
};

/// text_view_impl_controller_base specialization for base std controllers
template <qt_selection_controller Controller>
requires qt_std_controller<Controller>
struct text_model_impl_controller_base<Controller> {
    text_model_impl_controller_base() = default;
    text_model_impl_controller_base(Controller & cntrl) {}
};


/// Helper base class for text_view_impl, used for selecting user defined selection model storage.
template <typename SelectionModel>
struct text_model_impl_selection_model_base {
    text_model_impl_selection_model_base(SelectionModel & sel_mdl):
        selection_{sel_mdl} {}

    SelectionModel & selection_;        ///< Reference to user defined selection model
};

/// text_model_impl_selection_model_base specialization for base std selection model
template <typename SelectionModel>
requires std::same_as<SelectionModel, std_selection_model>
struct text_model_impl_selection_model_base<SelectionModel> {
    text_model_impl_selection_model_base() = default;
    text_model_impl_selection_model_base(SelectionModel & mdl) {}
};


/// Generic native Qt text view implementation for both plain and right text widgets.
template <
    typename QtTextEdit,
    text_model Model,
    qt_selection_model<Model> SelectionModel,
    qt_selection_controller_for<Model> Controller
>
requires (!qt_std_controller<Controller> ||
          editable_single_selection_model<SelectionModel> ||
          std::same_as<SelectionModel, std_selection_model>)
class text_view_impl:
        public QtTextEdit,
        private text_model_impl_controller_base<Controller>,
        private text_model_impl_selection_model_base<SelectionModel> {

    static constexpr bool is_const = !qt_edit_controller<Controller>;

    /// Standard Qt edit controller can be used only with editable model
    static_assert(!std::same_as<Controller, std_edit_controller> || editable_text_model<Model>);

    /// User defined controller must support either mouse events or text select event
    static_assert(qt_std_controller<Controller> ||
                  selection_controller_with_mouse<Controller> ||
                  selection_controller_with_select_text_for<Controller, Model>);

public:
    /// Type of reference to model (const or non-const) depending on controller
    using model_t = std::conditional_t<
        qt_edit_controller<Controller>,
        Model &,
        const Model &
    >;

    /// Constructs text view with specified reference to model
    text_view_impl(model_t & text)
    requires qt_std_controller<Controller> && std::same_as<SelectionModel, std_selection_model>:
    text_{text} {
        init();
    }

    /// Constructs text view with specified references to text model and selection model
    text_view_impl(model_t & text, SelectionModel & sel_mdl)
    requires qt_std_controller<Controller>:
    text_model_impl_selection_model_base<SelectionModel>{sel_mdl},
    text_{text} {
        init();
    }

    /// Constructs text view with specified references to text model, selection model,
    /// and controller
    text_view_impl(model_t & text, SelectionModel & sel_mdl, Controller & cntrl):
    text_model_impl_selection_model_base<SelectionModel>{sel_mdl},
    text_model_impl_controller_base<Controller>{cntrl},
    text_{text} {
        init();
    }

protected:
    /// Handles key press event in text view
    void keyPressEvent(QKeyEvent * event) override {
        if constexpr (!qt_std_controller<Controller>) {
            // passing key event to user defined controller
            impl::process_edit_key_event(this->cntrl_, event);
        } else {
            // passing key event to Qt control
            QtTextEdit::keyPressEvent(event);
        }
    }

    /// Handles mouse press event
    void mousePressEvent(QMouseEvent * event) override {
        if constexpr (selection_controller_with_mouse<Controller>) {
            if (event->button() != Qt::LeftButton) {
                QtTextEdit::mousePressEvent(event);
                return;
            }

            auto cursor = this->cursorForPosition(event->pos());
            auto pos = impl::get_position_from_cursor(cursor);
            auto modifiers = QApplication::keyboardModifiers();
            this->cntrl_.do_mouse_press(pos,
                                        modifiers & Qt::ControlModifier,
                                        modifiers & Qt::ShiftModifier);
            event->accept();
        } else {
            // passing mouse event to Qt control
            QtTextEdit::mousePressEvent(event);
        }
    }

    /// Handles mouse release event
    void mouseReleaseEvent(QMouseEvent * event) override {
        if constexpr (selection_controller_with_mouse<Controller>) {
            if (event->button() != Qt::LeftButton) {
                QtTextEdit::mouseReleaseEvent(event);
                return;
            }

            auto cursor = this->cursorForPosition(event->pos());
            auto pos = impl::get_position_from_cursor(cursor);
            auto modifiers = QApplication::keyboardModifiers();
            this->cntrl_.do_mouse_release(pos,
                                          modifiers & Qt::ControlModifier,
                                          modifiers & Qt::ShiftModifier);
            event->accept();
        } else {
            // passing mouse event to Qt control
            QtTextEdit::mouseReleaseEvent(event);
        }
    }

    /// Handles mouse move event
    void mouseMoveEvent(QMouseEvent * event) override {
        if constexpr (selection_controller_with_mouse<Controller>) {
            if (!(event->buttons() & Qt::LeftButton)) {
                QtTextEdit::mouseMoveEvent(event);
                return;
            }

            auto cursor = this->cursorForPosition(event->pos());
            auto pos = impl::get_position_from_cursor(cursor);
            auto modifiers = QApplication::keyboardModifiers();
            this->cntrl_.do_mouse_move(pos,
                                       modifiers & Qt::ControlModifier,
                                       modifiers & Qt::ShiftModifier);
            event->accept();
        } else {
            // passing mouse event to Qt control
            QtTextEdit::mouseMoveEvent(event);
        }
    }

private:
    /// Initializes text view
    void init() {
        // setting read only flag
        this->setReadOnly(is_const);

        // setting text in view
        if constexpr (text_model_character<typename Model::char_t> &&
                      !std_character<typename Model::char_t>) {
            this->setPlainText(QString{});
            auto cursor = this->textCursor();
            impl::insert_formatted_chars(cursor, characters_vector(text_));
            this->setTextCursor(cursor);
        } else {
            this->setPlainText(impl::chars_to_qstring(characters(text_)));
        }

        // listening for changes in text model
        after_inserted_con_ = text_.after_inserted.connect([this](const range & r) {
            on_model_inserted(r);
        });
        after_erased_con_ = text_.after_erased.connect([this](const range & r) {
            on_model_erased(r);
        });
        after_replaced_con_ = text_.after_replaced.connect([this](const range & r) {
            on_model_replaced(r);
        });

        // listening for changes in selection model if we are using user defined selection model
        if constexpr (!std::same_as<SelectionModel, std_selection_model>) {
            selection_changed_con_ = this->selection_.changed.connect([this]() {
                on_model_selection_changed();
            });
        }

        // listening for changes of text in view if controller is a standard qt edit controller
        if constexpr (std::same_as<Controller, std_edit_controller>) {
            QObject::connect(this->document(), &QTextDocument::contentsChange,
            [this](int pos, int removed, int inserted) {
                on_view_contents_changed(pos, removed, inserted);
            });
        }

        // listening for changes of selection in view if:
        // 1. we are using user defined selection model
        // 2. we are using standard controller or user defined controller with mouse support
        if constexpr (!std::same_as<SelectionModel, std_selection_model> &&
                      qt_std_controller<Controller>) {

            QObject::connect(this, &QtTextEdit::cursorPositionChanged, [this] {
                on_view_selection_changed();
            });
            QObject::connect(this, &QtTextEdit::selectionChanged, [this] {
                on_view_selection_changed();
            });
        }
    }

    /// Called after characters are inserted in model
    void on_model_inserted(const range & r) {
        if (ignore_model_changes_) {
            return;
        }

        is_updating_view_ = true;
        auto cursor = impl::get_qt_cursor_for_selection(this, r.start, r.start);
        if constexpr (text_model_character<typename Model::char_t> &&
                      !std_character<typename Model::char_t>) {
            auto chars = characters_vector(text_, r);
            impl::insert_formatted_chars(cursor, chars);
        } else {
            auto chars = impl::chars_to_qstring(characters(text_, r));
            cursor.insertText(chars);
        }
        is_updating_view_ = false;
    }

    /// Called after characters are erased from model
    void on_model_erased(const range & r) {
        if (ignore_model_changes_) {
            return;
        }
    
        is_updating_view_ = true;
        impl::get_qt_cursor_for_selection(this, r.start, r.end).removeSelectedText();
        is_updating_view_ = false;
    }

    /// Called after characters are replaced in model
    void on_model_replaced(const range & r) {
        if (ignore_model_changes_) {
            return;
        }

        // preserving cursor position when replacing text
        auto [anchor_pos, pos] = impl::get_selection_from_text_edit(this);

        is_updating_view_ = true;
        auto cursor = impl::get_qt_cursor_for_selection(this, r.start, r.end);
        if constexpr (text_model_character<typename Model::char_t> &&
                      !std_character<typename Model::char_t>) {
            auto chars = characters_vector(text_, r);
            impl::insert_formatted_chars(cursor, chars);
        } else {
            auto chars = impl::chars_to_qstring(characters(text_, r));
            cursor.insertText(chars);
        }
        this->setTextCursor(impl::get_qt_cursor_for_selection(this, anchor_pos, pos));
        is_updating_view_ = false;
    }

    /// Called when view contents changed
    void on_view_contents_changed(int pos, int removed, int inserted)
    requires std::same_as<Controller, std_edit_controller> {

        if (is_updating_view_) {
            return;
        }

        auto change_pos_it = std::ranges::next(std::ranges::begin(positions(text_)), pos);
        auto remove_end_pos_it = std::ranges::next(change_pos_it, removed);

        ignore_model_changes_ = true;
        
        // removing characters
        if (removed != 0) {
            text_.erase({*change_pos_it, *remove_end_pos_it});
        }

        // inserting characters
        if (inserted != 0) {
            if constexpr (text_model_character<typename Model::char_t> &&
                          !std_character<typename Model::char_t>) {
                QTextCursor cursor(this->document());
                cursor.setPosition(pos);
                cursor.setPosition(pos + inserted, QTextCursor::KeepAnchor);
                using base_char_t = ::tmvc::impl::char_value_t<typename Model::char_t>;
                auto inserted_chars = impl::read_formatted_chars<base_char_t>(cursor);
                text_.insert(*change_pos_it, inserted_chars);
            } else {
                auto inserted_qstr = this->toPlainText().mid(pos, inserted);
                auto inserted_chars = impl::qstring_to_chars<typename Model::char_t>(inserted_qstr);
                text_.insert(*change_pos_it, inserted_chars);
            }
        }

        ignore_model_changes_ = false;
    }

    /// Called when selection in view is changed
    void on_view_selection_changed()
    requires (!std::same_as<SelectionModel, std_selection_model> &&
              qt_std_controller<Controller>) {
        if (is_updating_view_) {
            return;
        }

        auto [anchor_pos, pos] = impl::get_selection_from_text_edit(this);

        // changing position in document if not same
        if (this->selection_.pos() != pos || this->selection_.anchor_pos() != anchor_pos) {
            this->selection_.set_pos_and_anchor(anchor_pos, pos);
        }
    }

    /// Called when selection changed in selection model
    void on_model_selection_changed()
    requires (!std::same_as<SelectionModel, std_selection_model>) {
        // setting update flag to prevent model updates in other strategies
        is_updating_view_ = true;

        // updating view
        auto cursor = impl::get_qt_cursor_for_selection(this,
                                                        this->selection_.anchor_pos(),
                                                        this->selection_.pos());
        this->setTextCursor(cursor);

        // clearing update flag
        is_updating_view_ = false;
    }

    model_t & text_;                        ///< Reference to text model
    bool ignore_model_changes_ = false;     ///< Should changes in model be ignored?
    bool is_updating_view_ = false;         ///< True if text view is being updated now

    // connections to text model signals
    scoped_signal_connection after_inserted_con_;
    scoped_signal_connection after_erased_con_;
    scoped_signal_connection after_replaced_con_;

    // connection to selection model signal
    scoped_signal_connection selection_changed_con_;
};


}
