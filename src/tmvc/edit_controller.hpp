// Copyright (c) 2025, Alexandr Esilevich
//
// Distributed under the Boost Software License.
// See accompanying file LICENSE for license information.
//

/// \file edit_controller.hpp
/// Contains definition of the edit_controller concept.

#pragma once

#include "selection_controller.hpp"


namespace tmvc {


/// Represents controller for text edit actions in text views
template <typename Controller>
concept edit_controller = selection_controller<Controller> && requires(Controller & cntrl) {
    // Returns true if controller is in overwrite mode
    { cntrl.is_overwrite_mode() } -> std::convertible_to<bool>;

    // Performs actions when user presses delete button
    cntrl.do_delete(true, false);

    // Performs actions when user presses backspace button
    cntrl.do_backspace(true, false);

    /// Performs actions when user presses enter button
    cntrl.do_enter(true, false);

    /// Performs actions when user presses tab button
    cntrl.do_tab(true, false);

    /// Performs actions when user presses insert button
    cntrl.do_insert(true, false);

    /// Performs actions when user presses character button
    cntrl.do_char(std::declval<typename Controller::char_t>());


    ////////////////////////////////////////////////////////////
    // Predefined user actions

    // Returns true if cut action can be performed now
    { cntrl.can_cut() } -> std::same_as<bool>;

    // Cuts selected text
    { cntrl.cut() } -> std::same_as<std::basic_string<typename Controller::char_t>>;

    // Pastes text in current selection
    cntrl.paste(std::declval<std::basic_string<typename Controller::char_t>>());

    /// Returns true if delete action can be performed now
    { cntrl.can_delete() } -> std::same_as<bool>;

    /// Performs delete action in document
    cntrl.delete_();

    // Returns true if undo action can be performed now
    { cntrl.can_undo() } -> std::same_as<bool>;

    // Performs undo action if can be done
    cntrl.undo();

    // Returns true if redo action can be performed now
    { cntrl.can_redo() } -> std::same_as<bool>;

    // Performs redo action if can be done
    cntrl.redo();
};


/// Represents controller for text editing in specified text model
template <typename Controller, typename TextModel>
concept edit_controller_for =
    selection_controller_for<Controller, TextModel> &&
    edit_controller<Controller>;


}
