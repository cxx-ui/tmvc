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


    ////////////////////////////////////////////////////////////
    // Keyboard input processing

    // Performs actions when user presses delete button
    cntrl.do_delete(true, false, false);

    // Performs actions when user presses backspace button
    cntrl.do_backspace(true, false, false);

    /// Performs actions when user presses enter button
    cntrl.do_enter(true, false, false);

    /// Performs actions when user presses tab button
    cntrl.do_tab(true, false, false);

    /// Performs actions when user presses insert button
    cntrl.do_insert(true, false, false);

    /// Performs actions when user presses character button
    cntrl.do_char(std::declval<typename Controller::char_t>());


    ////////////////////////////////////////////////////////////
    // Text modification

    // Inserts characters at specified position. Returns range of inserted characters
    cntrl.insert(std::declval<position>(),
                 std::declval<std::vector<typename Controller::char_t>>());

    // Erases characters in specified range
    cntrl.erase(std::declval<range>());

    // Replaces characters at specified position. All replaced characters must be
    // in the same line
    cntrl.replace(std::declval<position>(),
                  std::declval<std::vector<typename Controller::char_t>>());


    ////////////////////////////////////////////////////////////
    // Predefined user actions

    // Returns true if cut action can be performed now
    { cntrl.can_cut() } -> std::same_as<bool>;

    // Cuts selected text
    { cntrl.cut() } -> std::ranges::range;
    requires std::convertible_to<std::ranges::range_value_t<decltype(cntrl.cut())>,
                                 typename Controller::char_t>;

    // Pastes text in current selection
    cntrl.paste(std::declval<std::vector<typename Controller::char_t>>());

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


/// Represents edit controller with replacement of found text
template <typename Controller>
concept edit_controller_with_replace = edit_controller<Controller> &&
                                       selection_controller_with_find<Controller> &&
                                       requires(Controller & cntrl) {
    // Replaces current find match with specified characters
    cntrl.replace(std::declval<std::vector<typename Controller::char_t>>());

    // Replaces all find matches with specified characters
    cntrl.replace_all(std::declval<std::vector<typename Controller::char_t>>());
};


/// Replaces current find match in edit controller with standard characters
template <edit_controller_with_replace Controller>
void replace(Controller & controller,
             const text_character_value_string<typename Controller::char_t> & str) {
    using char_t = typename Controller::char_t;

    // creating characters of controller character type from character values
    std::vector<char_t> chars;
    chars.reserve(str.size());
    for (auto ch : str) {
        chars.push_back(impl::make_char<char_t>(ch));
    }

    controller.replace(chars);
}


/// Replaces all find matches in edit controller with standard characters
template <edit_controller_with_replace Controller>
void replace_all(Controller & controller,
                 const text_character_value_string<typename Controller::char_t> & str) {
    using char_t = typename Controller::char_t;

    // creating characters of controller character type from character values
    std::vector<char_t> chars;
    chars.reserve(str.size());
    for (auto ch : str) {
        chars.push_back(impl::make_char<char_t>(ch));
    }

    controller.replace_all(chars);
}


/// Deletes text from start of previous word to current position in edit controller
template <edit_controller Controller>
requires selection_controller_with_word_navigation<Controller>
void delete_start_of_word(Controller & controller) {
    // selecting text up to start of previous word, selected text is deleted as is
    if (!controller.can_delete()) {
        controller.move_prev_word(true);
    }

    // there is nothing to delete when cursor is at the beginning of text
    if (controller.can_delete()) {
        controller.delete_();
    }
}


/// Deletes text from current position to start of next word in edit controller
template <edit_controller Controller>
requires selection_controller_with_word_navigation<Controller>
void delete_end_of_word(Controller & controller) {
    // selecting text up to start of next word, selected text is deleted as is
    if (!controller.can_delete()) {
        controller.move_next_word(true);
    }

    // there is nothing to delete when cursor is at the end of text
    if (controller.can_delete()) {
        controller.delete_();
    }
}


}
