// Copyright (c) 2025, Alexandr Esilevich
//
// Distributed under the Boost Software License.
// See accompanying file LICENSE for license information.
//

/// \file utils.hpp
/// Contains text utility functions.

#pragma once

#include <list>
#include <memory>
#include <string>
#include "../range.hpp"


namespace tmvc::impl {


/// Converts string to list of lines
template <typename Char>
std::list<std::shared_ptr<std::basic_string<Char>>> split_string_to_lines(const std::basic_string<Char> & chars) {
    if (chars.empty()) {
        return {std::make_shared<std::basic_string<Char>>()};
    }

    // creating list of new lines
    size_t new_lines_count = 0;
    std::list<std::shared_ptr<std::basic_string<Char>>> lines;
    auto new_line = std::make_shared<std::basic_string<Char>>();

    for (auto ch : chars) {
        if (ch == static_cast<Char>('\r')) {
            // skipping \r characters
            continue;
        }

        if (ch == static_cast<Char>('\n')) {
            lines.push_back(new_line);
            new_line = std::make_shared<std::basic_string<Char>>();
        } else {
            new_line->push_back(ch);
        }
    }

    lines.push_back(new_line);

    return lines;
}


/// Returns selected range of text taking into account correct order
/// of current position and selection anchor
inline range selected_range(const position & a_pos, const position & pos) {
    if (a_pos <= pos) {
        return {a_pos, pos};
    } else {
        return {pos, a_pos};
    }
}


}
