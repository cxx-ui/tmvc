// Copyright (c) 2025, Alexandr Esilevich
//
// Distributed under the Boost Software License.
// See accompanying file LICENSE for license information.
//

/// \file simple_text_model.hpp
/// Contains definition of the basic_simple_text_model class and associated type aliases.

#pragma once

#include "characters_range.hpp"
#include "editable_text_model.hpp"
#include "signals.hpp"
#include "simple_text_data.hpp"
#include "std_character.hpp"
#include "text_model_signals.hpp"
#include "impl/utils.hpp"
#include <memory>
#include <string>
#include <vector>


namespace tmvc {


/// Represents text model that holds all text in memory as array of lines.
template <typename Char>
class basic_simple_text_model: public text_model_signals {
public:
    /// Type of character
    using char_t = Char;

    /// Constructs empty text model that contains empty text line
    explicit basic_simple_text_model();

    /// Constructs text model with specified range of characters
    explicit basic_simple_text_model(characters_range<Char> auto && chars);

    /// Constructs text model with specified pointer to null terminated string
    explicit basic_simple_text_model(const char_t * chars)
    requires std_character<char_t>:
        basic_simple_text_model{std::basic_string_view{chars}} {}

    /// Returns number of lines in model
    uint64_t lines_size() const {
        return data_.lines_size();
    }

    /// Returns length of line at specified index
    uint64_t line_size(uint64_t line_idx) const {
        return data_.line_size(line_idx);
    }

    /// Returns character at specified position
    Char char_at(const position & pos) const {
        return data_.char_at(pos);
    }


    ////////////////////////////////////////////////////////////
    // Text modification

    /// Insert characters at specified position. Returns position range for inserted characters.
    range insert(const position & p, characters_range<Char> auto && chars);

    /// Insert characters at specified position. Returns position range for inserted characters.
    range insert(const position & p, const char_t * chars) requires std_character<Char> {
        return insert(p, std::basic_string_view{chars});
    }

    /// Deletes characters from specified position range.
    void erase(const range & r);

    /// Replaces characters at specified position
    void replace(const position & p, characters_range<Char> auto && chars);

    void replace(const position & p, const char_t * chars) requires std_character<Char> {
        replace(p, std::basic_string_view{chars});
    }

    /// Resets text in model without sending any signals and updating current
    /// position. Should be used only for testing.
    void reset(characters_range<Char> auto && chars);

    /// Resets text in model without sending any signals and updating current
    /// position. Should be used only for testing.
    void reset(const char_t * chars) requires std_character<Char> {
        reset(std::basic_string_view{chars});
    }

private:
    basic_simple_text_data<Char> data_;
};


/// Type alias for simple text model with simple character
using simple_text_model = basic_simple_text_model<char>;

/// Type alias for simple text model with wide character
using wsimple_text_model = basic_simple_text_model<wchar_t>;


}


#include "simple_text_model.ipp"
