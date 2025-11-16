// Copyright (c) 2025, Alexandr Esilevich
//
// Distributed under the Boost Software License.
// See accompanying file LICENSE for license information.
//

/// \file std_mouse_controller.hpp
/// Contains definition of the std_mouse_controller class.

#pragma once

#include "position.hpp"
#include <utility>


namespace tmvc {


/// Concept that all classes derived from std_mouse_controller should confirm.
template <typename Derived>
concept std_mouse_controller_derived = requires(Derived & derived) {
    /// Sets position with moving selection anchor
    derived.set_pos_move_anchor(std::declval<position>());

    /// Sets position with keeping selection anchor
    derived.set_pos_keep_anchor(std::declval<position>());
};


/// Standard mouse controller. Implements functions for handling mouse events in
/// selection controllers. Uses CTPR pattern for dispatching selection processing
/// to derived class.
template <typename Derived>
class std_mouse_controller {
public:
    /// Processes mouse press event at specified text position
    void do_mouse_press(const position & pos, bool ctrl, bool shift) {
        static_assert(std_mouse_controller_derived<Derived>);
        is_mouse_pressed_ = true;
        static_cast<Derived*>(this)->set_pos_move_anchor(pos);
    }


    /// Processes mouse release event at specified text position
    void do_mouse_release(const position & pos, bool ctrl, bool shift) {
        is_mouse_pressed_ = false;
    }


    // Processes mouse move event at specified text position
    void do_mouse_move(const position & pos, bool ctrl, bool shift) {
        static_assert(std_mouse_controller_derived<Derived>);
        if (is_mouse_pressed_) {
            static_cast<Derived*>(this)->set_pos_keep_anchor(pos);
        }
    }

private:
    bool is_mouse_pressed_ = false;     ///< True if mouse button is pressed now
};


}