// Copyright (c) 2025, Alexandr Esilevich
//
// Distributed under the Boost Software License.
// See accompanying file LICENSE for license information.
//

/// \file std_paging_controller.hpp
/// Contains definition of the std_paging_controller class.

#pragma once

#include "position.hpp"
#include <cstdint>
#include <optional>
#include <utility>


namespace tmvc {


/// Concept that all classes derived from std_paging_controller should confirm.
template <typename Derived>
concept std_paging_controller_derived = requires(Derived & derived) {
    // Returns selection anchor position
    { derived.anchor_pos() } -> std::convertible_to<position>;

    // Selects text
    derived.select_text(std::declval<position>(), std::declval<position>(), std::declval<bool>());

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

    /// Moves cursor to previous page, using target_pos for visual wrap mode
    void move_prev_page(bool keep_anchor,
                        const std::optional<position> & target_pos = std::nullopt) {
        static_assert(std_paging_controller_derived<Derived>);
        auto * d = static_cast<Derived*>(this);
        if (target_pos) {
            if (keep_anchor) {
                d->select_text(d->anchor_pos(), *target_pos, false);
            } else {
                d->select_text(*target_pos, *target_pos, false);
            }
            return;
        }

        d->move_lines_up(viewport_height_, keep_anchor);
    }

    /// Moves cursor to next page, using target_pos for visual wrap mode
    void move_next_page(bool keep_anchor,
                        const std::optional<position> & target_pos = std::nullopt) {
        static_assert(std_paging_controller_derived<Derived>);
        auto * d = static_cast<Derived*>(this);
        if (target_pos) {
            if (keep_anchor) {
                d->select_text(d->anchor_pos(), *target_pos, false);
            } else {
                d->select_text(*target_pos, *target_pos, false);
            }
            return;
        }

        d->move_lines_down(viewport_height_, keep_anchor);
    }

    /// Performs actions when user presses page up button
    void do_page_up(bool,
                    bool shift,
                    bool,
                    const std::optional<position> & pos = std::nullopt) {
        move_prev_page(shift, pos);
    }

    /// Performs actions when user presses page down button
    void do_page_down(bool,
                      bool shift,
                      bool,
                      const std::optional<position> & pos = std::nullopt) {
        move_next_page(shift, pos);
    }

private:
    /// Height of viewport in lines
    uint64_t viewport_height_ = 1;
};


}
