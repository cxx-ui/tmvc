// Copyright (c) 2025, Alexandr Esilevich
//
// Distributed under the Boost Software License.
// See accompanying file LICENSE for license information.
//

/// \file empty_selection_model.hpp
/// Contains definition of the empty_selection_model class.

#pragma once

#include "selection_model.hpp"


namespace tmvc {


/// Selection model that does not store any value and always has no selection
class empty_selection_model {
public:
    /// Always returns invalid position
    static position pos() { return {UINT64_MAX, UINT64_MAX}; }

    /// Always returns invalid position
    static position anchor_pos() { return {UINT64_MAX, UINT64_MAX}; }

    /// Constructs emtpy selection model
    empty_selection_model() = default;

    /// Constructs empty selection model for any text model
    template <typename TextModel>
    empty_selection_model(TextModel &&) {}

    /// Returns range of cursors. Always returns empty range
    auto cursors() const {
        return std::ranges::views::empty<position>;
    }

    /// Returns range of selections. Always returns empty range
    auto selections() const {
        return std::ranges::views::empty<range>;
    }

    /// Changed signal. Never emitted
    mutable signal <void()> changed;
};


static_assert(selection_model<empty_selection_model>,
              "empty selection model must confirm to selection_model concept");


}
