// Copyright (c) 2025, Alexandr Esilevich
//
// Distributed under the Boost Software License.
// See accompanying file LICENSE for license information.
//

/// \file string.hpp
/// Contains definition of utility functions for converting strings.

#pragma once

#include "../../text_model.hpp"
#include <ranges>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>
#include <QString>


namespace tmvc::qt::impl {


/// Converts std string to QString
template <typename Char>
QString std_string_to_qstring(const std::basic_string<Char> & str) {
    if constexpr (std::same_as<Char, char>) {
        return QString::fromStdString(str);
    } else if constexpr (std::same_as<Char, wchar_t>) {
        return QString::fromStdWString(str);
    } else if constexpr (std::same_as<Char, char16_t>) {
        return QString::fromStdU16String(str);
    } else if constexpr (std::same_as<Char, char32_t>) {
        return QString::fromStdU32String(str);
    } else {
        static_assert(!std::is_same<Char, Char>::value, "unknown character type for conversion");
    }
}


/// Converts QString to std string
template <typename Char>
std::basic_string<Char> qstring_to_std_string(const QString & str) {
    if constexpr (std::same_as<Char, char>) {
        return str.toStdString();
    } else if constexpr (std::same_as<Char, wchar_t>) {
        return str.toStdWString();
    } else if constexpr (std::same_as<Char, char16_t>) {
        return str.toStdU16String();
    } else if constexpr (std::same_as<Char, char32_t>) {
        return str.toStdU32String();
    } else {
        static_assert(!std::is_same<Char, Char>::value, "unknown character type for conversion");
    }
}


/// Converts characters range to QString
template <typename CharsRange>
QString chars_to_qstring(CharsRange && chars) {
    using value_t = std::ranges::range_value_t<CharsRange>;

    if constexpr (text_model_character<value_t> && !std_character<value_t>) {
        using char_t = decltype(std::declval<value_t>().character());
        std::basic_string<char_t> str;
        if constexpr (requires { std::ranges::size(chars); }) {
            str.reserve(std::ranges::size(chars));
        }
        for (const auto & ch : chars) {
            str.push_back(ch.character());
        }
        return std_string_to_qstring(str);
    } else if constexpr (std::same_as<value_t, wchar_t>) {
        std::wstring str{std::ranges::begin(chars), std::ranges::end(chars)};
        return std_string_to_qstring(str);
    } else {
        std::string str{std::ranges::begin(chars), std::ranges::end(chars)};
        return std_string_to_qstring(str);
    }
}

/// Converts QString to character range (std::basic_string or vector)
template <typename Char>
auto qstring_to_chars(const QString & str) {
    if constexpr (std::same_as<Char, char>) {
        return qstring_to_std_string<char>(str);
    } else if constexpr (std::same_as<Char, wchar_t>) {
        return qstring_to_std_string<wchar_t>(str);
    } else if constexpr (std::same_as<Char, char16_t>) {
        return qstring_to_std_string<char16_t>(str);
    } else if constexpr (std::same_as<Char, char32_t>) {
        return qstring_to_std_string<char32_t>(str);
    } else if constexpr (text_model_character<Char> && !std_character<Char>) {
        using value_t = decltype(std::declval<Char>().character());
        auto plain = qstring_to_std_string<value_t>(str);
        std::vector<Char> out;
        out.reserve(plain.size());
        for (auto ch : plain) {
            out.emplace_back(ch);
        }
        return out;
    } else {
        static_assert(!std::is_same_v<Char, Char>, "unknown character type for conversion");
    }
}


}
