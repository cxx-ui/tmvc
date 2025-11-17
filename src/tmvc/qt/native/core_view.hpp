// Copyright (c) 2025, Alexandr Esilevich
//
// Distributed under the Boost Software License.
// See accompanying file LICENSE for license information.
//

/// \file core_view.hpp
/// Contains definition of the core_view template and related types.

#pragma once

#include <type_traits>
#include <QPainter>
#include <QPlainTextEdit>
#include <QTextBlock>


namespace tmvc::qt::native {


template <typename QtTextEdit>
class core_view_base;


/// Concept representing piece of logic of interacting with text view widget.
/// Contains single initialize function that performs setup of text view
/// and associated signal and event handlers.
template <typename Strategy, typename QtTextEdit>
concept core_plain_text_view_strategy = std::is_move_constructible_v<Strategy> &&
requires(Strategy & strategy, core_view_base<QtTextEdit> * view) {
    // initializes handlers for specified pointer to text view
    strategy.initialize(view);
};


/// Base class for all core_view instantiations. Contains
/// common data and logic to synchronizing between strategies.
template <typename QtTextEdit>
class core_view_base: public QtTextEdit {
public:
    /// Constructs text view with specified pointer to parent widget
    core_view_base(QWidget * parent = nullptr):
        QtTextEdit{parent} {}

    /// Returns true if text view is being updated now
    bool is_updating_view() const { return is_updating_view_; }

    /// Sets whether text view is being updated now
    void set_is_updating_view(bool val) { is_updating_view_ = val; }

private:
    bool is_updating_view_ = false;     ///< True if text view is being updated now
};


/// Qt plain text view that uses QTextEdit or QPlainTextEdit to display text
/// and customizable strategies to handle selection, user input, and menus.
template <
    typename QtTextEdit,
    bool ReadOnly,
    core_plain_text_view_strategy<QtTextEdit> ... Strategies
>
class core_view: public core_view_base<QtTextEdit>, Strategies... {
    /// Type of this class
    using this_type = core_view <
        QtTextEdit,
        ReadOnly,
        Strategies...
    >;

    /// Tuple of strategies
    using strategies_tuple = std::tuple<Strategies...>;

public:
    /// Constructs view with specified reference to text model, key handle strategy,
    /// and pointer to parent widget
    core_view(QWidget * parent, Strategies && ... strategies):
    core_view_base<QtTextEdit>{parent},
    Strategies{std::move(strategies)}... {
        // setting read only flag
        this->setReadOnly(ReadOnly);

        // initializing strategies
        (Strategies::initialize(this), ...);

        // repainting widget when current selection changes. This is needed to
        // repaint rectangle for current line
        QObject::connect(this, &QtTextEdit::cursorPositionChanged, [this] {
            this->viewport()->update();
        });
        QObject::connect(this, &QtTextEdit::selectionChanged, [this] {
            this->viewport()->update();
        });
    }

protected:
    /// Processes paint event. Draws additional rectangle for current line
    void paintEvent(QPaintEvent * event) override {
        // painting text edit
        QtTextEdit::paintEvent(event);

        if constexpr (std::same_as<QtTextEdit, QPlainTextEdit>) {
            // highlighting current line with rect if selection is empty
            if (!this->textCursor().hasSelection()) {
                auto currentLineRect = this->blockBoundingGeometry(this->textCursor().block());
                auto offs = this->contentOffset();
                currentLineRect.translate(offs);
                currentLineRect.setRight(this->viewport()->width() - static_cast<int>(this->contentOffset().rx()));

                QPainter painter{this->viewport()};
                painter.setPen(QPen{QBrush{QColor{Qt::lightGray}}, 1});
                painter.drawRect(currentLineRect);
            }
        }
    }
};


}
