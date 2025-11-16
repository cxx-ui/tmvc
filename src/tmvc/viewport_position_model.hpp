// Copyright (c) 2025, Alexandr Esilevich
//
// Distributed under the Boost Software License.
// See accompanying file LICENSE for license information.
//

/// \file viewport_position_model.hpp
/// Contains definition of the viewport_position_model concept.

#pragma once

#include <type_traits>


namespace tmvc {


/// Represents vertical position in text view viewport
template <typename Model>
concept viewport_position_model = requires(const Model & cmdl, Model & mdl) {
    /// Returns current vertical position in viewport
    { cmdl.pos() } -> std::convertible_to<uint64_t>;

    /// Sets current vertical position in viewport
    mdl.set_pos(std::declval<uint64_t>());

    /// Sets number of visible lines in viewport
    mdl.set_visible_lines_count(std::declval<uint64_t>());
};


}
