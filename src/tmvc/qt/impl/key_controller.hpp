// Copyright (c) 2025, Alexandr Esilevich
//
// Distributed under the Boost Software License.
// See accompanying file LICENSE for license information.
//

/// \file key_controller.hpp
/// Contains implementation of function for processing Qt key events for controllers.

#pragma once

#include "../../edit_controller.hpp"
#include "../../selection_controller.hpp"
#include "string.hpp"
#include <optional>
#include <ranges>
#include <QClipboard>
#include <QGuiApplication>
#include <QKeyEvent>


namespace tmvc::qt::impl {


/// Processes key event for selection controller. Returns true if event was processed
/// and future processing is not required
template <selection_controller Controller>
bool process_selection_key_event(Controller & controller,
                                 QKeyEvent * event,
                                 const std::optional<position> & pos = std::nullopt) {
    bool ctrl = (event->modifiers() & Qt::ControlModifier) != 0;
    bool shift = (event->modifiers() & Qt::ShiftModifier) != 0;
    bool alt = (event->modifiers() & Qt::AltModifier) != 0;

    // processing standard navigation key sequences
    if (event->matches(QKeySequence::Copy)) {
        auto chars = controller.copy();
        auto qstr = chars_to_qstring(chars);
        QGuiApplication::clipboard()->setText(qstr);
        event->accept();
        return true;
    } else if (event->matches(QKeySequence::SelectAll)) {
        controller.select_all();
        event->accept();
        return true;
    }

    // dispatching word navigation sequences if controller supports them
    if constexpr (selection_controller_with_word_navigation<Controller>) {
        if (event->matches(QKeySequence::MoveToNextWord) ||
            event->matches(QKeySequence::SelectNextWord)) {
            controller.move_next_word(ctrl, alt, shift);
            event->accept();
            return true;
        } else if (event->matches(QKeySequence::MoveToPreviousWord) ||
                   event->matches(QKeySequence::SelectPreviousWord)) {
            controller.move_prev_word(ctrl, alt, shift);
            event->accept();
            return true;
        }
    }

    // dispatching line navigation sequences if controller supports them
    if constexpr (selection_controller_with_line_navigation<Controller>) {
        if (event->matches(QKeySequence::MoveToStartOfLine) ||
            event->matches(QKeySequence::SelectStartOfLine)) {
            controller.move_line_start(ctrl, alt, shift, pos);
            event->accept();
            return true;
        } else if (event->matches(QKeySequence::MoveToEndOfLine) ||
                   event->matches(QKeySequence::SelectEndOfLine)) {
            controller.move_line_end(ctrl, alt, shift, pos);
            event->accept();
            return true;
        } else if (event->matches(QKeySequence::MoveToPreviousLine) ||
                   event->matches(QKeySequence::SelectPreviousLine)) {
            controller.move_prev_line(ctrl, alt, shift, pos);
            event->accept();
            return true;
        } else if (event->matches(QKeySequence::MoveToNextLine) ||
                   event->matches(QKeySequence::SelectNextLine)) {
            controller.move_next_line(ctrl, alt, shift, pos);
            event->accept();
            return true;
        }
    }

    // dispatching block navigation sequences if controller supports them
    if constexpr (selection_controller_with_block_navigation<Controller>) {
        // Alt+Up/Down are the macOS bindings for MoveToStartOfBlock/MoveToEndOfBlock.
        // On non-macOS platforms the standard sequence is unbound, so we match them
        // explicitly here so that the behaviour is consistent across all platforms.
        bool alt_up   = alt && !ctrl && event->key() == Qt::Key_Up;
        bool alt_down = alt && !ctrl && event->key() == Qt::Key_Down;
        if (event->matches(QKeySequence::MoveToStartOfBlock) ||
            event->matches(QKeySequence::SelectStartOfBlock) || alt_up) {
            controller.move_block_start(ctrl, alt, shift);
            event->accept();
            return true;
        } else if (event->matches(QKeySequence::MoveToEndOfBlock) ||
                   event->matches(QKeySequence::SelectEndOfBlock) || alt_down) {
            controller.move_block_end(ctrl, alt, shift);
            event->accept();
            return true;
        }
    }

    // dispatching page navigation sequences
    if (event->matches(QKeySequence::MoveToPreviousPage) ||
        event->matches(QKeySequence::SelectPreviousPage)) {
        controller.do_page_up(ctrl, shift, alt, pos);
        event->accept();
        return true;
    } else if (event->matches(QKeySequence::MoveToNextPage) ||
               event->matches(QKeySequence::SelectNextPage)) {
        controller.do_page_down(ctrl, shift, alt, pos);
        event->accept();
        return true;
    }

    // processing navigation keys

    switch (event->key()) {
    case Qt::Key_Left:
        controller.do_left(ctrl, shift, alt, pos);
        event->accept();
        return true;
    case Qt::Key_Right:
        controller.do_right(ctrl, shift, alt, pos);
        event->accept();
        return true;
    case Qt::Key_Up:
        controller.do_up(ctrl, shift, alt, pos);
        event->accept();
        return true;
    case Qt::Key_Down:
        controller.do_down(ctrl, shift, alt, pos);
        event->accept();
        return true;
    case Qt::Key_Home:
        controller.do_home(ctrl, shift, alt);
        event->accept();
        return true;
    case Qt::Key_End:
        controller.do_end(ctrl, shift, alt);
        event->accept();
        return true;

    default:
        // doing nothing for now
        break;
    }

    return false;
}


/// Processes key event for edit controller. Returns true if event was processed
/// and future processing is not required
template <edit_controller Controller>
bool process_edit_key_event(Controller & controller,
                            QKeyEvent * event,
                            const std::optional<position> & pos = std::nullopt) {
    using char_t = typename Controller::char_t;

    bool ctrl = (event->modifiers() & Qt::ControlModifier) != 0;
    bool shift = (event->modifiers() & Qt::ShiftModifier) != 0;
    bool alt = (event->modifiers() & Qt::AltModifier) != 0;
    bool meta = (event->modifiers() & Qt::MetaModifier) != 0;

    // processing standard editing key sequences
    if (event->matches(QKeySequence::Cut)) {
        auto str = controller.cut();
        QGuiApplication::clipboard()->setText(chars_to_qstring(str));
        event->accept();
        return true;
    } else if (event->matches(QKeySequence::Paste)) {
        auto qstr = QGuiApplication::clipboard()->text();
        controller.paste(qstring_to_chars<char_t>(qstr));
        event->accept();
        return true;
    } else if (event->matches(QKeySequence::Undo)) {
        controller.undo();
        event->accept();
        return true;
    } else if (event->matches(QKeySequence::Redo)) {
        controller.redo();
        event->accept();
        return true;
    }

    // processing modification keys
    switch (event->key()) {
    case Qt::Key_Backspace:
        controller.do_backspace(ctrl, shift, alt);
        event->accept();
        return true;
    case Qt::Key_Delete:
        controller.do_delete(ctrl, shift, alt);
        event->accept();
        return true;

    case Qt::Key_Enter:
    case Qt::Key_Return:
        controller.do_enter(ctrl, shift, alt);
        event->accept();
        return true;

    case Qt::Key_Tab:
        controller.do_tab(ctrl, shift, alt);
        event->accept();
        return true;

    case Qt::Key_Backtab:
        controller.do_tab(ctrl, true, alt);
        event->accept();
        return true;

    case Qt::Key_Insert:
        controller.do_insert(ctrl, shift, alt);
        event->accept();
        return true;

    default:
        // inserting single character if its printable
        if (!ctrl && !meta && !event->text().isEmpty()) {
            auto chars = qstring_to_chars<char_t>(event->text());
            if (std::ranges::size(chars) == 1) {
                std::locale loc("");
                auto ch = *std::ranges::begin(chars);
                auto ch_value = [&] {
                    if constexpr (requires { ch.character(); }) {
                        return ch.character();
                    } else {
                        return ch;
                    }
                }();
                if (std::isprint(ch_value, loc)) {
                    controller.do_char(ch);
                    event->accept();
                    return true;
                }
            }
        }
    }

    // delegating key handle to selection controller
    return process_selection_key_event(controller, event, pos);
}


}
