// Copyright (c) 2025, Alexandr Esilevich
//
// Distributed under the Boost Software License.
// See accompanying file LICENSE for license information.
//

/// \file single_selection_model.hpp
/// Contains definition of the single_selection_model concept.

#pragma once

#include "selection_model.hpp"


namespace tmvc {


/// Represents single-selection model with current position and anchor position.
/// This concept requires only read access to selection state.
template <typename SelectionModel>
concept single_selection_model = selection_model<SelectionModel> &&
requires (const SelectionModel & csel) {
    /// Returns current position
    { csel.pos() } -> std::convertible_to<position>;

    /// Returns selection anchor position
    { csel.anchor_pos() } -> std::convertible_to<position>;
};

/// Represents single-selection model with mutable selection state.
template <typename SelectionModel>
concept editable_single_selection_model = single_selection_model<SelectionModel> &&
requires (SelectionModel & sel,
          const position & anchor_pos,
          const position & pos) {
    /// Sets current position and anchor position
    sel.set_pos_and_anchor(anchor_pos, pos);
};


/// Returns selected range of text taking into account correct order
/// of current position and selection anchor.
template <single_selection_model SelectionModel>
range selected_range(const SelectionModel & sel) {
    if (sel.anchor_pos() <= sel.pos()) {
        return {sel.anchor_pos(), sel.pos()};
    } else {
        return {sel.pos(), sel.anchor_pos()};
    }
}

/// Sets anchor position.
template <editable_single_selection_model SelectionModel>
void set_anchor_pos(SelectionModel & sel, const position & anchor_pos) {
    sel.set_pos_and_anchor(anchor_pos, sel.pos());
}

/// Sets current position and keeps selection anchor position.
template <editable_single_selection_model SelectionModel>
void set_pos_keep_anchor(SelectionModel & sel, const position & pos) {
    sel.set_pos_and_anchor(sel.anchor_pos(), pos);
}

/// Sets current position and selection anchor position to same values.
template <editable_single_selection_model SelectionModel>
void set_pos_move_anchor(SelectionModel & sel, const position & pos) {
    sel.set_pos_and_anchor(pos, pos);
}


}
