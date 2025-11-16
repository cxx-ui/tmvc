// Copyright (c) 2025, Alexandr Esilevich
//
// Distributed under the Boost Software License.
// See accompanying file LICENSE for license information.
//

/// \file selection_controller.hpp
/// Contains definition of the selection_controller concept and selection controller utilities.

#pragma once

#include "context_menu.hpp"
#include "position.hpp"
#include "text_model.hpp"


namespace tmvc {


/// \concept selection_controller
/// \brief Represents controller for text selection in text views
template <typename Controller>
concept selection_controller = requires(Controller & cntrl, const position & p) {

    /// Type of character
    typename Controller::char_t;


    ////////////////////////////////////////////////////////////
    // Text navigation

    // Sets cursor position and selection anchor position
    cntrl.select_text(p, p);


    ////////////////////////////////////////////////////////////
    // Key button actions

    // Performs actions when user presses left button
    cntrl.do_left(true, false);

    // Performs actions when user presses right button
    cntrl.do_right(true, false);

    // Performs actions when user presses up button
    cntrl.do_up(true, false);

    // Performs actions when user presses down button
    cntrl.do_down(true, false);

    // Performs actions when user presses home button
    cntrl.do_home(true, false);

    // Performs actions when user presses end button
    cntrl.do_end(true, false);


    ////////////////////////////////////////////////////////////
    // Predefined user actions

    // Returns true if copy action can be performed now
    { cntrl.can_copy() } -> std::same_as<bool>;

    // Copies selected text
    { cntrl.copy() } -> std::same_as<std::basic_string<typename Controller::char_t>>;

    // Selects all text
    cntrl.select_all();


    ////////////////////////////////////////////////////////////
    // Context menu

    // Constructs context menu information
    // TODO: move to separate model
    { cntrl.create_context_menu() } -> std::same_as<basic_context_menu<typename Controller::char_t>>;
};


/// Represents selection controller with mouse navigation
template <typename Controller>
concept selection_controller_with_mouse = selection_controller<Controller> && requires(Controller & cntrl) {
    // Processes mouse press event at specified text position
    cntrl.do_mouse_press(std::declval<position>(), true, false);

    // Processes mouse release event at specified text position
    cntrl.do_mouse_release(std::declval<position>(), true, false);

    // Processes mouse move event at specified text position
    cntrl.do_mouse_move(std::declval<position>(), true, false);
};


/// Represents selection controller with paging navigation
/// Represents selection controller with mouse navigation
template <typename Controller>
concept selection_controller_with_paging = selection_controller<Controller> && requires(Controller & cntrl) {
    // Sets viewport height in lines
    cntrl.set_viewport_height(std::declval<uint64_t>());

    // Performs actions when user presses page up button
    cntrl.do_page_up(true, false);

    // Performs actions when user presses page down button
    cntrl.do_page_down(true, false);
};


/// Represents selection controller with text selection method
template <typename Controller>
concept selection_controller_with_select_text = selection_controller<Controller> && requires(Controller & cntrl) {
    // Selects text
    cntrl.select_text(std::declval<position>(), std::declval<position>());
};


/// Represents controller for text selection in specified text model
template <typename Controller, typename TextModel>
concept selection_controller_for =
    selection_controller<Controller> &&
    text_model<TextModel> &&
    std::same_as<typename Controller::char_t, typename TextModel::char_t>;


/// Represents controller with select_text method for specified text model
template <typename Controller, typename TextModel>
concept selection_controller_with_select_text_for =
    selection_controller_with_select_text<Controller> &&
    selection_controller_for<Controller, TextModel>;


}
