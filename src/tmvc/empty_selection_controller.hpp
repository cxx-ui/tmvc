// Copyright (c) 2025, Alexandr Esilevich
//
// Distributed under the Boost Software License.
// See accompanying file LICENSE for license information.
//

/// \file empty_selection_controller.hpp
/// Contains definition of the emtpy_selection_controller class

#pragma once

#include "selection_controller.hpp"


namespace tmvc {


/// Selection controller that does nothing
template <typename Char>
class empty_selection_controller {
public:
    /// Type of character
    using char_t = Char;

    /// Constructs empty selection controller
    empty_selection_controller() = default;

    /// Constructs empty selection controller for any text and selection models
    template <typename TextModel, typename SelectionModel>
    empty_selection_controller(TextModel &&, SelectionModel &&) {}

    /// Sets cursor position and selection anchor position. Should not be called.
    /// TODO: refactor and remove from controller concept.
    static void select_text(const position & a_pos, const position & pos) {
        assert(false && "should never be called");
    }

    static void set_viewport_height(uint64_t) {}

    static void do_mouse_press(const position &, bool, bool) {}
    static void do_mouse_release(const position &, bool, bool) {}
    static void do_mouse_move(const position &, bool, bool) {}

    static void do_left(bool, bool) {}
    static void do_right(bool, bool) {}
    static void do_up(bool, bool) {}
    static void do_down(bool, bool) {}
    static void do_home(bool, bool) {}
    static void do_end(bool, bool) {}
    static void do_page_up(bool, bool) {}
    static void do_page_down(bool, bool) {}

    static bool can_copy() { return false; }
    static std::vector<char_t> copy() { assert(false && "should never be called"); return {}; }
    static void select_all() {}

    static basic_context_menu<char_t> create_context_menu() { return {}; }
};


static_assert(selection_controller<empty_selection_controller<char>>);


}
