
/// \file std_character.hpp
/// Contains definition of the std_character concept.

#pragma once

#include <concepts>


namespace tmvc {


/// Represents one of standard characters type (char, wchar_t, etc)
template <typename Char>
concept std_character = std::same_as<Char, char> ||
                        std::same_as<Char, wchar_t> ||
                        std::same_as<Char, char8_t> ||
                        std::same_as<Char, char16_t> ||
                        std::same_as<Char, char32_t>;

}
