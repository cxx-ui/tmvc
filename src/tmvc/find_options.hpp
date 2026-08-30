// Copyright (c) 2026, Alexandr Esilevich
//
// Distributed under the Boost Software License.
// See accompanying file LICENSE for license information.
//

/// \file find_options.hpp
/// Contains definition of the find_options struct.

#pragma once


namespace tmvc {


/// Mode of comparing characters when searching text
enum class char_match_mode {
    whole_char,                 ///< Compares characters with all their attributes
    char_value,                 ///< Compares character values, taking case into account
    char_value_ignore_case      ///< Compares character values, ignoring case
};


/// Options for text search
struct find_options {
    /// Mode of comparing characters
    char_match_mode match_mode = char_match_mode::char_value_ignore_case;

    bool match_word = false;        ///< Whole words search

    /// Returns true if options are equal to another options
    constexpr bool operator ==(const find_options & o) const = default;
};


}
