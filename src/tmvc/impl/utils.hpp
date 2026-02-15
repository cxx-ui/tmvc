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
#include <utility>
#include <vector>
#include "../range.hpp"
#include "../text_model.hpp"


namespace tmvc::impl {

/// Returns type of underlying character code for basic or formatted characters
template <typename Char>
struct char_value_type {
    using type = Char;
};

template <text_model_character Char>
requires (!std_character<Char>)
struct char_value_type<Char> {
    using type = decltype(std::declval<Char>().character());
};

template <typename Char>
using char_value_t = typename char_value_type<Char>::type;

/// Creates a character of specified type from a character literal/code point
template <typename Char, typename Value>
constexpr Char make_char(Value ch) {
    using base_t = char_value_t<Char>;
    if constexpr (requires { Char{static_cast<base_t>(ch)}; }) {
        return Char{static_cast<base_t>(ch)};
    } else {
        return static_cast<Char>(ch);
    }
}


/// Converts range of characters to list of lines
template <typename Char, typename CharsRange>
std::list<std::shared_ptr<std::vector<Char>>>
split_chars_to_lines(CharsRange && chars) {

    if (std::ranges::empty(chars)) {
        return {std::make_shared<std::vector<Char>>()};
    }

    // creating list of new lines
    size_t new_lines_count = 0;
    std::list<std::shared_ptr<std::vector<Char>>> lines;
    auto new_line = std::make_shared<std::vector<Char>>();

    for (auto ch : chars) {
        if (ch == static_cast<char_value_t<decltype(ch)>>('\r')) {
            // skipping \r characters
            continue;
        }

        if (ch == static_cast<char_value_t<decltype(ch)>>('\n') ||

            // paragraph end character
            ch == static_cast<char_value_t<decltype(ch)>>(0x2029)) {

            lines.push_back(new_line);
            new_line = std::make_shared<std::vector<Char>>();
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
