// Copyright (c) 2025, Alexandr Esilevich
//
// Distributed under the Boost Software License.
// See accompanying file LICENSE for license information.
//

/// \file std_paging_controller.hpp
/// Contains definition of the std_paging_controller class.

#pragma once

#include <cstdint>
#include <utility>


namespace tmvc {


/// Concept that all classes derived from std_paging_controller should confirm.
template <typename Derived>
concept std_paging_controller_derived = requires(Derived & derived) {
    // Moves position up by specified number of lines
    derived.move_lines_up(std::declval<uint64_t>(), std::declval<bool>());

    // Moves position down by specified number of lines
    derived.move_lines_down(std::declval<uint64_t>(), std::declval<bool>());
};


/// Standard controlling for handling page up/down actions.
/// Uses CTPR pattern to dispatch event processing to derived class.
template <typename Derived>
class std_paging_controller {
public:
    /// Sets viewport height in lines
    void set_viewport_height(uint64_t height) {
        viewport_height_ = height;
    }

    // Performs actions when user presses page up button
    void do_page_up(bool ctrl, bool shift) {
        static_assert(std_paging_controller_derived<Derived>);
        static_cast<Derived*>(this)->move_lines_up(viewport_height_, shift);
    }

    // Performs actions when user presses page down button
    void do_page_down(bool ctrl, bool shift) {
        static_assert(std_paging_controller_derived<Derived>);
        static_cast<Derived*>(this)->move_lines_down(viewport_height_, shift);
    }

private:
    /// Height of viewport in lines
    uint64_t viewport_height_ = 1;
};


}
