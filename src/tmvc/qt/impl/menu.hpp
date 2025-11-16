// Copyright (c) 2025, Alexandr Esilevich
//
// Distributed under the Boost Software License.
// See accompanying file LICENSE for license information.
//

/// \file menu.hpp
/// Contains utility functions for creating menus for text edit widgets.

#pragma once

#include "string.hpp"
#include "../../selection_controller.hpp"
#include <memory>
#include <QClipboard>
#include <QGuiApplication>
#include <QMenu>


namespace tmvc::qt::impl {


/// Adds copy action into menu which delegates call to specified selection controller
template <typename Char, selection_controller Controller>
QAction * add_menu_copy_action(QMenu * menu, Controller & controller) {
    auto copy = menu->addAction("Copy");
    copy->setIcon(QIcon::fromTheme("edit-copy"));
    copy->setShortcut(QKeySequence{QKeySequence::Copy});
    copy->setEnabled(controller.can_copy());
    QObject::connect(copy, &QAction::triggered, [&controller] {
        auto str = controller.copy();
        QGuiApplication::clipboard()->setText(std_string_to_qstring(str));
    });
    return copy;
}


/// Adds select all action into menu which delegates call to specified selection controller
template <typename Char, selection_controller Controller>
QAction * add_menu_select_all_action(QMenu * menu, Controller & controller) {
    auto action = menu->addAction("Select All");
    action->setIcon(QIcon::fromTheme("edit-select-all"));
    action->setShortcut(QKeySequence{QKeySequence::SelectAll});
    QObject::connect(action, &QAction::triggered, [&controller] {
        controller.select_all();
    });
    return action;
}


/// Adds undo action into menu which delegates call to specified edit controller
template <typename Char, edit_controller Controller>
QAction * add_menu_undo_action(QMenu * menu, Controller & controller) {
    auto undo = menu->addAction("Undo");
    undo->setIcon(QIcon::fromTheme("edit-undo"));
    undo->setShortcut(QKeySequence{QKeySequence::Undo});
    undo->setEnabled(controller.can_undo());
    QObject::connect(undo, &QAction::triggered, [&controller] { controller.undo(); });
    return undo;
}


/// Adds redo action into menu which delegates call to specified edit controller
template <typename Char, edit_controller Controller>
QAction * add_menu_redo_action(QMenu * menu, Controller & controller) {
    auto redo = menu->addAction("Redo");
    redo->setIcon(QIcon::fromTheme("edit-redo"));
    redo->setShortcut(QKeySequence{QKeySequence::Redo});
    redo->setEnabled(controller.can_redo());
    QObject::connect(redo, &QAction::triggered, [&controller] { controller.redo(); });
    return redo;
}


/// Adds cut action into menu which delegates call to specified edit controller
template <typename Char, edit_controller Controller>
QAction * add_menu_cut_action(QMenu * menu, Controller & controller) {
    auto cut = menu->addAction("Cut");
    cut->setIcon(QIcon::fromTheme("edit-cut"));
    cut->setShortcut(QKeySequence{QKeySequence::Cut});
    cut->setEnabled(controller.can_cut());
    QObject::connect(cut, &QAction::triggered, [&controller] {
        auto str = controller.cut();
        QGuiApplication::clipboard()->setText(std_string_to_qstring(str));
    });
    return cut;
}


/// Adds paste action into menu which delegates call to specified edit controller
template <typename Char, edit_controller Controller>
QAction * add_menu_paste_action(QMenu * menu, Controller & controller) {
    auto paste = menu->addAction("Paste");
    paste->setIcon(QIcon::fromTheme("edit-paste"));
    paste->setShortcut(QKeySequence{QKeySequence::Paste});
    auto clipboard = QGuiApplication::clipboard();
    paste->setEnabled(!clipboard->text().isEmpty());
    QObject::connect(paste, &QAction::triggered, [&controller] {
        auto qstr = QGuiApplication::clipboard()->text();
        controller.paste(qstring_to_std_string<typename Controller::char_t>(qstr));
    });
    return paste;
}


/// Creates standard context menu which delegates all actions to selection controller
template <typename Char, selection_controller Controller>
std::unique_ptr<QMenu> create_std_selection_context_menu(Controller & controller) {
    auto menu = std::make_unique<QMenu>();
    add_menu_copy_action<Char>(menu.get(), controller);
    menu->addSeparator();
    add_menu_select_all_action<Char>(menu.get(), controller);
    return menu;
}


/// Creates standard context menu which delegates all actions to edit controller
template <typename Char, edit_controller Controller>
std::unique_ptr<QMenu> create_std_edit_context_menu(Controller & controller) {
    auto menu = std::make_unique<QMenu>();

    add_menu_undo_action<Char>(menu.get(), controller);
    add_menu_redo_action<Char>(menu.get(), controller);

    menu->addSeparator();

    add_menu_cut_action<Char>(menu.get(), controller);
    add_menu_copy_action<Char>(menu.get(), controller);
    add_menu_paste_action<Char>(menu.get(), controller);

    menu->addSeparator();

    add_menu_select_all_action<Char>(menu.get(), controller);

    return menu;
}


/// Adds standard menu action for selection controller
template <typename Char, selection_controller Controller>
QAction * add_std_selection_qt_menu_action(QMenu * menu,
                                           const standard_context_menu_action & action,
                                           Controller & controller) {
    switch (action.type()) {
    case standard_context_menu_action_type::copy:
        return add_menu_copy_action<Char>(menu, controller);
    case standard_context_menu_action_type::select_all:
        return add_menu_select_all_action<Char>(menu, controller);
    default:
        assert(false && "unknown action for selection controller");
        return nullptr;
    }
}


/// Adds standard menu action for edit controller
template <typename Char, edit_controller Controller>
QAction * add_std_edit_qt_menu_action(QMenu * menu,
                                      const standard_context_menu_action & action,
                                      Controller & controller) {
    switch (action.type()) {
    case standard_context_menu_action_type::undo:
        return add_menu_undo_action<Char>(menu, controller);
    case standard_context_menu_action_type::redo:
        return add_menu_redo_action<Char>(menu, controller);
    case standard_context_menu_action_type::cut:
        return add_menu_cut_action<Char>(menu, controller);
    case standard_context_menu_action_type::paste:
        return add_menu_paste_action<Char>(menu, controller);
    default:
        return add_std_selection_qt_menu_action<Char>(menu, action, controller);
    }
}


/// Adds separator or custom action into Qt menu. Returns pointer to added QAction
/// or nullptr if action is a standard action
template <typename Char>
QAction * add_qt_menu_separator_or_custom_action(QMenu * menu, const context_menu_action & action) {
    // checking for separator action
    if (auto separator_action = dynamic_cast<const separator_context_menu_action*>(&action)) {
        return menu->addSeparator();
    }

    // checking for custom action
    if (auto custom_action = dynamic_cast<const custom_context_menu_action_base<Char>*>(&action)) {
        auto qt_action = menu->addAction(std_string_to_qstring(custom_action->name()));
        qt_action->setEnabled(custom_action->enabled());

        QObject::connect(qt_action, &QAction::triggered, [custom_action] {
            custom_action->execute();
        });

        return qt_action;
    }

    return nullptr;
}


/// Adds Qt action into Qt menu for context menu action and selection controller
template <typename Char, selection_controller Controller>
QAction * add_qt_menu_action(QMenu * menu, const context_menu_action & action, Controller & controller) {
    // trying add separator or custom action
    auto res = add_qt_menu_separator_or_custom_action<Char>(menu, action);
    if (res) {
        return res;
    }

    // adding standard action
    auto std_action = dynamic_cast<const standard_context_menu_action*>(&action);
    assert(std_action != nullptr && "unknown action type");
    return add_std_selection_qt_menu_action<Char>(menu, *std_action, controller);
}


/// Adds Qt action into Qt menu for context menu action and edit controller
template <typename Char, edit_controller Controller>
QAction * add_qt_menu_action(QMenu * menu, const context_menu_action & action, Controller & controller) {
    // trying add separator or custom action
    auto res = add_qt_menu_separator_or_custom_action<Char>(menu, action);
    if (res) {
        return res;
    }

    // adding standard action
    auto std_action = dynamic_cast<const standard_context_menu_action*>(&action);
    assert(std_action != nullptr && "unknown action type");
    return add_std_edit_qt_menu_action<Char>(menu, *std_action, controller);
}


/// Creates Qt menu from context_menu object
template <typename Char, selection_controller Controller>
std::unique_ptr<QMenu> create_qt_menu_from_context_menu(const basic_context_menu<Char> & menu,
                                                        Controller & controller) {
    auto qt_menu = std::make_unique<QMenu>();

    for (auto && action : menu.actions()) {
        add_qt_menu_action<Char>(qt_menu.get(), *action, controller);
    }

    return qt_menu;
}


}
