// Copyright (c) 2025, Alexandr Esilevich
//
// Distributed under the Boost Software License.
// See accompanying file LICENSE for license information.
//

/// \file formatted_char.hpp
/// Contains definition of formatted character types.

#pragma once

#include "std_character.hpp"
#include <cstddef>
#include <cstdint>
#include <string>


namespace tmvc {


/// Represents RGB color.
class color {
public:
    /// Constructs black color (0, 0, 0)
    constexpr color() = default;
    /// Constructs color with specified RGB components
    constexpr color(std::uint8_t r, std::uint8_t g, std::uint8_t b):
        r_{r}, g_{g}, b_{b} {}

    /// Returns red component
    constexpr std::uint8_t r() const { return r_; }

    /// Returns green component
    constexpr std::uint8_t g() const { return g_; }

    /// Returns blue component
    constexpr std::uint8_t b() const { return b_; }

    /// Sets red component
    constexpr void set_r(std::uint8_t r) { r_ = r; }

    /// Sets green component
    constexpr void set_g(std::uint8_t g) { g_ = g; }

    /// Sets blue component
    constexpr void set_b(std::uint8_t b) { b_ = b; }

    /// Compares colors
    friend constexpr bool operator==(const color &, const color &) = default;

private:
    std::uint8_t r_ = 0;        ///< Red component
    std::uint8_t g_ = 0;        ///< Green component
    std::uint8_t b_ = 0;        ///< Blue component
};


/// Simple text formatting attributes.
/// Interpretation of fields is up to the view implementation.
class text_format {
public:
    /// Constructs empty text format
    constexpr text_format() = default;

    /// Constructs text format with specified colors and style flags
    constexpr text_format(const color & fg,
                          const color & bg,
                          bool bold,
                          bool italic,
                          bool underline,
                          bool strikethrough):
        foreground_{fg},
        background_{bg},
        bold_{bold},
        italic_{italic},
        underline_{underline},
        strikethrough_{strikethrough} {}

    /// Returns foreground color
    constexpr const color & foreground() const { return foreground_; }

    /// Returns foreground color
    constexpr color & foreground() { return foreground_; }

    /// Returns background color
    constexpr const color & background() const { return background_; }

    /// Returns background color
    constexpr color & background() { return background_; }

    /// Returns bold flag
    constexpr bool bold() const { return bold_; }

    /// Sets bold flag
    constexpr void set_bold(bool bold) { bold_ = bold; }

    /// Returns italic flag
    constexpr bool italic() const { return italic_; }

    /// Sets italic flag
    constexpr void set_italic(bool italic) { italic_ = italic; }

    /// Returns underline flag
    constexpr bool underline() const { return underline_; }

    /// Sets underline flag
    constexpr void set_underline(bool underline) { underline_ = underline; }

    /// Returns strikethrough flag
    constexpr bool strikethrough() const { return strikethrough_; }

    /// Sets strikethrough flag
    constexpr void set_strikethrough(bool strikethrough) { strikethrough_ = strikethrough; }

    /// Compares text formats
    friend constexpr bool operator==(const text_format &, const text_format &) = default;

private:
    color foreground_;           ///< Foreground color
    color background_;           ///< Background color
    bool bold_ = false;          ///< Bold flag
    bool italic_ = false;        ///< Italic flag
    bool underline_ = false;     ///< Underline flag
    bool strikethrough_ = false; ///< Strikethrough flag
};


/// Represents a character with associated formatting.
template <std_character Char, typename Format = text_format>
class basic_formatted_char {
public:
    using char_t = Char;
    using format_t = Format;

    /// Constructs formatted character with default values
    constexpr basic_formatted_char() = default;

    /// Constructs formatted character with specified character
    constexpr basic_formatted_char(char_t ch): ch_{ch} {}

    /// Constructs formatted character with specified character and format
    constexpr basic_formatted_char(char_t ch, const format_t & fmt):
        ch_{ch}, fmt_{fmt} {}

    /// Returns character code
    constexpr char_t character() const { return ch_; }

    /// Sets character code
    constexpr void set_character(char_t ch_val) { ch_ = ch_val; }

    /// Returns character format
    constexpr const format_t & format() const { return fmt_; }

    /// Returns character format
    constexpr format_t & format() { return fmt_; }

    /// Compares formatted characters
    friend constexpr bool operator==(const basic_formatted_char &,
                                      const basic_formatted_char &) = default;

    /// Compares formatted character with plain character
    friend constexpr bool operator==(const basic_formatted_char & c, char_t ch) {
        return c.ch_ == ch;
    }

    /// Compares plain character with formatted character
    friend constexpr bool operator==(char_t ch, const basic_formatted_char & c) {
        return c == ch;
    }

private:
    char_t ch_{};       ///< Character code
    format_t fmt_{};    ///< Character formatting
};


/// Type alias for formatted character with narrow char
using formatted_char = basic_formatted_char<char>;

/// Type alias for formatted character with wide char
using wformatted_char = basic_formatted_char<wchar_t>;


}
