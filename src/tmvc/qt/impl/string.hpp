// Copyright (c) 2025, Alexandr Esilevich
//
// Distributed under the Boost Software License.
// See accompanying file LICENSE for license information.
//

/// \file string.hpp
/// Contains definition of utility functions for converting strings.

#pragma once

#include <string>
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


}
