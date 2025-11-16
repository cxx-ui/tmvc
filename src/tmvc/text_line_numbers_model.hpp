// Copyright (c) 2025, Alexandr Esilevich
//
// Distributed under the Boost Software License.
// See accompanying file LICENSE for license information.
//

/// \file text_line_numbers_model.hpp
/// Contains definition of the text_line_numbers_model class.

#pragma once

#include "line_numbers_model.hpp"
#include "text_model.hpp"
#include <iomanip>


namespace tmvc {


/// Line numbers model that returns simple line numbers for a text model
template <text_model TextModel>
class text_line_numbers_model {
public:
    /// Type of character
    using char_t = typename TextModel::char_t;

    /// Type of string
    using string_t = typename std::basic_string<char_t>;

    /// Constructs line numbers model for specified text model
    text_line_numbers_model(const TextModel & txt):
    text_{txt} {
    }

    /// Returns maximum number of characters in line numbers string representation.
    /// Calculates size from max line number in a text model
    uint64_t max_size() const {
        auto lines_count = text_.lines_size();
        auto res = 0;
        while (lines_count > 0) {
            lines_count /= 10;
            ++res;
        }

        return res;
    }

    /// Returns string representation of line number for specified line
    string_t line_number(uint64_t idx) const {
        // TODO: rewrite in more optimal way
        std::basic_ostringstream<char_t> str;
        str << (idx + 1);
        return str.str();
    }

private:
    const TextModel & text_;            ///< Reference to text model
};


}
