
/// \file characters_range.hpp
/// Contains definition of the characters_range concept.

#pragma once

#include <ranges>


namespace tmvc {


/// Generic range of characters of specified type
template <typename Range, typename Char>
concept characters_range = std::ranges::range<Range> &&
                           std::convertible_to<std::ranges::range_value_t<Range>, Char>;


}
