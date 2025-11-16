// Copyright (c) 2025, Alexandr Esilevich
//
// Distributed under the Boost Software License.
// See accompanying file LICENSE for license information.
//

/// \file line_numbers_model.hpp
/// Contains definition of the line_numbers_model concept.

#pragma once

#include <cstdint>
#include <string>
#include <utility>


namespace tmvc {


/// Represents generic line numbers model that provide string representation for
/// line numbers of a text model
template <typename LineNumbersModel>
concept line_numbers_model = requires (const LineNumbersModel & model) {
    /// Type of character
    typename LineNumbersModel::char_t;

    /// Returns maximum number of characters in line numbers string representation
    { model.max_size() } -> std::convertible_to<uint64_t>;

    /// Returns line number string representation for specified line
    { model.line_number(std::declval<uint64_t>()) } ->
        std::convertible_to<std::basic_string<typename LineNumbersModel::char_t>>;
};


}
