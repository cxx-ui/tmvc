// Copyright (c) 2025, Alexandr Esilevich
//
// Distributed under the Boost Software License.
// See accompanying file LICENSE for license information.
//

/// \file context_menu.hpp
/// Contains definition of classes related to context menu information.

#pragma once

#include <memory>
#include <ranges>
#include <string>
#include <vector>


namespace tmvc {


/// Represents abstract single context menu action
class context_menu_action {
public:
    /// Default virtual destructor
    virtual ~context_menu_action() = default;
};


/// Type of standard context menu action
enum standard_context_menu_action_type {
    copy,
    select_all,
    cut,
    paste,
    undo,
    redo
};


/// Standard context menu action
class standard_context_menu_action: public context_menu_action {
public:
    /// Standard action type
    using action_type = standard_context_menu_action_type;

    /// Constructs context menu action with specified type
    standard_context_menu_action(action_type t):
        type_{t} {}

    /// Returns action type
    action_type type() const { return type_; }

private:
    action_type type_;
};


/// Base class for all instantiations of context menu actions
template <typename Char>
class custom_context_menu_action_base: public context_menu_action {
public:
    /// Constructs custom action with specified name and enabled state
    custom_context_menu_action_base(std::basic_string<Char> name, bool enabled):
        name_{std::move(name)}, enabled_{enabled} {}

    /// Returns action name
    auto & name() const { return name_; }

    /// Returns true if action is enabled
    bool enabled() const { return enabled_; }

    /// Executes action functor
    virtual void execute() const = 0;

private:
    std::basic_string<Char> name_;      ///< Action name
    bool enabled_;                      ///< Enabled state
};


/// Custom context menu action item
template <typename Char, typename Functor>
class custom_context_menu_action: public custom_context_menu_action_base<Char> {
public:
    /// Constructs custom action with specified name, enabled state, and functor
    custom_context_menu_action(std::basic_string<Char> name, bool enabled, Functor func):
        custom_context_menu_action_base<Char>{std::move(name), enabled}, functor_{func} {}

    /// Executes action functor
    void execute() const override { functor_(); }

private:
    Functor functor_;                   ///< Action functor
};


/// Separator context menu action
class separator_context_menu_action: public context_menu_action {};


/// Context menu
template <typename Char>
class basic_context_menu {
public:
    /// Creates empty context menu
    basic_context_menu() = default;

    /// Adds standard action with specified type and enabled state
    void add_action(standard_context_menu_action_type std_type) {
        actions_.push_back(std::make_unique<standard_context_menu_action>(std_type));
    }

    /// Adds custom context menu action
    template <typename Functor>
    void add_action(std::basic_string<Char> name, bool enabled, Functor && functor) {
        actions_.push_back(std::make_unique<custom_context_menu_action>(std::move(name),
                                                                        enabled,
                                                                        std::forward<Functor>(functor)));
    }

    /// Adds separator
    void add_separator() {
        actions_.push_back(std::make_unique<separator_context_menu_action>());
    }

    /// Returns range of actions
    auto actions() const {
        auto fn = [](const std::unique_ptr<context_menu_action> & action) {
            return static_cast<const context_menu_action*>(action.get());
        };

        return actions_ | std::ranges::views::transform(fn);
    }

private:
    std::vector<std::unique_ptr<context_menu_action>> actions_;     ///< Vector of menu actions
};


using context_menu = basic_context_menu<char>;
using wcontext_menu = basic_context_menu<wchar_t>;


}
