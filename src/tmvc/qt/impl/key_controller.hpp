// Copyright (c) 2025, Alexandr Esilevich
//
// Distributed under the Boost Software License.
// See accompanying file LICENSE for license information.
//

/// \file key_controller.hpp
/// Contains implementation of function for processing Qt key events for controllers.

#pragma once

#include "../../selection_controller.hpp"
#include "string.hpp"
#include <QClipboard>
#include <QGuiApplication>
#include <QKeyEvent>


namespace tmvc::qt::impl {


/// Processes key event for selection controller. Returns true if event was processed
/// and future processing is not required
template <selection_controller Controller>
bool process_selection_key_event(Controller & controller, QKeyEvent * event) {
    bool ctrl = (event->modifiers() & Qt::ControlModifier) != 0;
    bool shift = (event->modifiers() & Qt::ShiftModifier) != 0;

    // processing standard navigation key sequences
    if (event->matches(QKeySequence::Copy)) {
        auto str = controller.copy();
        auto qstr = std_string_to_qstring(str);
        QGuiApplication::clipboard()->setText(qstr);
        event->accept();
        return true;
    } else if (event->matches(QKeySequence::SelectAll)) {
        controller.select_all();
        event->accept();
        return true;
    }


    // processing navigation keys

    switch (event->key()) {
    case Qt::Key_Left:
        controller.do_left(ctrl, shift);
        event->accept();
        return true;
    case Qt::Key_Right:
        controller.do_right(ctrl, shift);
        event->accept();
        return true;
    case Qt::Key_Up:
        controller.do_up(ctrl, shift);
        event->accept();
        return true;
    case Qt::Key_Down:
        controller.do_down(ctrl, shift);
        event->accept();
        return true;
    case Qt::Key_Home:
        controller.do_home(ctrl, shift);
        event->accept();
        return true;
    case Qt::Key_End:
        controller.do_end(ctrl, shift);
        event->accept();
        return true;

    // delegating pageup/pagedown processing to QPlainTextEdit widget
    case Qt::Key_PageUp:
    case Qt::Key_PageDown:
        return false;

    default:
        // doing nothing for now
        break;
    }

    return false;
}


/// Processes key event for edit controller. Returns true if event was processed
/// and future processing is not required
template <edit_controller Controller>
bool process_edit_key_event(Controller & controller, QKeyEvent * event) {
    using char_t = typename Controller::char_t;

    bool ctrl = (event->modifiers() & Qt::ControlModifier) != 0;
    bool shift = (event->modifiers() & Qt::ShiftModifier) != 0;

    // processing standard editing key sequences
    if (event->matches(QKeySequence::Cut)) {
        auto str = controller.cut();
        QGuiApplication::clipboard()->setText(std_string_to_qstring(str));
        event->accept();
        return true;
    } else if (event->matches(QKeySequence::Paste)) {
        auto qstr = QGuiApplication::clipboard()->text();
        controller.paste(qstring_to_std_string<char_t>(qstr));
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
        controller.do_backspace(ctrl, shift);
        event->accept();
        return true;
    case Qt::Key_Delete:
        controller.do_delete(ctrl, shift);
        event->accept();
        return true;

    case Qt::Key_Enter:
    case Qt::Key_Return:
        controller.do_enter(ctrl, shift);
        event->accept();
        return true;

    case Qt::Key_Tab:
        controller.do_tab(ctrl, shift);
        event->accept();
        return true;

    case Qt::Key_Backtab:
        controller.do_tab(ctrl, true);
        event->accept();
        return true;

    case Qt::Key_Insert:
        controller.do_insert(ctrl, shift);
        event->accept();
        return true;

    default:
        // inserting single character if its printable
        if (!event->text().isEmpty()) {
            auto str = qstring_to_std_string<char_t>(event->text());
            if (str.size() == 1) {
                std::locale loc("");
                auto ch = str[0];
                if (std::isprint(ch, loc)) {
                    controller.do_char(ch);
                    event->accept();
                    return true;
                }
            }
        }
    }

    // delegating key handle to selection controller
    return process_selection_key_event(controller, event);
}


}
