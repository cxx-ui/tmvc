// Copyright (c) 2025, Alexandr Esilevich
//
// Distributed under the Boost Software License.
// See accompanying file LICENSE for license information.
//

/// \file selection_controller.hpp
/// Contains definition of the selection_controller concept and selection controller utilities.

#pragma once

#include "context_menu.hpp"
#include "find_options.hpp"
#include "position.hpp"
#include "range.hpp"
#include "text_model.hpp"
#include "impl/utils.hpp"
#include <optional>
#include <vector>


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
    cntrl.do_left(true, false, false, std::declval<std::optional<position>>());

    // Performs actions when user presses right button
    cntrl.do_right(true, false, false, std::declval<std::optional<position>>());

    // Performs actions when user presses up button
    cntrl.do_up(true, false, false, std::declval<std::optional<position>>());

    // Performs actions when user presses down button
    cntrl.do_down(true, false, false, std::declval<std::optional<position>>());

    // Performs actions when user presses home button
    cntrl.do_home(true, false, false);

    // Performs actions when user presses end button
    cntrl.do_end(true, false, false);

    // Performs actions when user presses page up button
    cntrl.do_page_up(true, false, false, std::declval<std::optional<position>>());

    // Performs actions when user presses page down button
    cntrl.do_page_down(true, false, false, std::declval<std::optional<position>>());


    ////////////////////////////////////////////////////////////
    // Predefined user actions

    // Returns true if copy action can be performed now
    { cntrl.can_copy() } -> std::same_as<bool>;

    // Copies selected text
    { cntrl.copy() } -> std::ranges::range;
    requires std::convertible_to<std::ranges::range_value_t<decltype(cntrl.copy())>,
                                 typename Controller::char_t>;

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

    // Processes mouse double click event at specified text position
    cntrl.do_mouse_double_click(std::declval<position>(), true, false);

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

    // Moves cursor to previous page, keeping selection anchor if keep_anchor is true
    cntrl.move_prev_page(true, std::declval<std::optional<position>>());

    // Moves cursor to next page, keeping selection anchor if keep_anchor is true
    cntrl.move_next_page(true, std::declval<std::optional<position>>());
};


/// Represents selection controller with text selection method
template <typename Controller>
concept selection_controller_with_select_text = selection_controller<Controller> &&
                                                requires(Controller & cntrl) {
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


/// Represents selection controller with word navigation
template <typename Controller>
concept selection_controller_with_word_navigation = selection_controller<Controller> &&
                                                    requires(Controller & cntrl) {
    // Moves cursor to next word, keeping selection anchor if keep_anchor is true
    cntrl.move_next_word(true);

    // Moves cursor to previous word, keeping selection anchor if keep_anchor is true
    cntrl.move_prev_word(true);
};


/// Represents selection controller with line navigation
template <typename Controller>
concept selection_controller_with_line_navigation = selection_controller<Controller> &&
                                                    requires(Controller & cntrl) {
    // Moves cursor to start of line, keeping selection anchor if keep_anchor is true
    cntrl.move_line_start(true, std::declval<std::optional<position>>());

    // Moves cursor to end of line, keeping selection anchor if keep_anchor is true
    cntrl.move_line_end(true, std::declval<std::optional<position>>());

    // Moves cursor to previous line, keeping selection anchor if keep_anchor is true
    cntrl.move_prev_line(true, std::declval<std::optional<position>>());

    // Moves cursor to next line, keeping selection anchor if keep_anchor is true
    cntrl.move_next_line(true, std::declval<std::optional<position>>());
};


/// Represents selection controller with block navigation
template <typename Controller>
concept selection_controller_with_block_navigation = selection_controller<Controller> &&
                                                     requires(Controller & cntrl) {
    // Moves cursor to start of block, keeping selection anchor if keep_anchor is true
    cntrl.move_block_start(true);

    // Moves cursor to end of block, keeping selection anchor if keep_anchor is true
    cntrl.move_block_end(true);
};


/// Represents selection controller with text search
template <typename Controller>
concept selection_controller_with_find = selection_controller<Controller> &&
                                         requires(Controller & cntrl) {
    // Searches for specified text and returns ranges of all found matches
    { cntrl.find(std::declval<std::vector<typename Controller::char_t>>(),
                 std::declval<find_options>()) } -> std::convertible_to<const std::vector<range> &>;

    // Returns ranges of matches found by the last search
    { cntrl.find_results() } -> std::convertible_to<const std::vector<range> &>;

    // Selects next match after current selection
    cntrl.find_next();

    // Selects previous match before current selection
    cntrl.find_prev();
};


/// Finds text in selection controller with standard characters
template <selection_controller_with_find Controller>
decltype(auto) find(Controller & controller,
                    const text_character_value_string<typename Controller::char_t> & str,
                    const find_options & options) {
    using char_t = typename Controller::char_t;

    // creating characters of controller character type from character values
    std::vector<char_t> chars;
    chars.reserve(str.size());
    for (auto ch : str) {
        chars.push_back(impl::make_char<char_t>(ch));
    }

    return controller.find(chars, options);
}


}
