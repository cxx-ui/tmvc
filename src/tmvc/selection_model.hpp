// Copyright (c) 2025, Alexandr Esilevich
//
// Distributed under the Boost Software License.
// See accompanying file LICENSE for license information.
//

/// \file selection_model.hpp
/// Contains definition of the selection model concept.

#pragma once

#include "position.hpp"
#include "range.hpp"
#include "signals.hpp"
#include <concepts>


namespace tmvc {


/// Represents model containing current position and anchor position in text model
template <typename SelectionModel>
concept selection_model = requires (const SelectionModel & csel) {
    /// Returns range of cursor positions
    { csel.cursors() } -> std::ranges::range;
    requires std::convertible_to<std::ranges::range_value_t<decltype(csel.cursors())>, position>;

    /// Returns range of selections
    { csel.selections() } -> std::ranges::range;
    requires std::convertible_to<std::ranges::range_value_t<decltype(csel.selections())>, range>;

    /// The changed signal is emitted after current or anchor position changes
    { csel.changed } -> std::convertible_to<signal<void ()> &>;
};


/// Returns selected range of text taking into account correct order
/// of current position and selection anchor
template <selection_model SelectionModel>
range selected_range(const SelectionModel & sel) {
    if (sel.anchor_pos() <= sel.pos()) {
        return {sel.anchor_pos(), sel.pos()};
    } else {
        return {sel.pos(), sel.anchor_pos()};
    }
}


}
