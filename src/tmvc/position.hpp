// Copyright (c) 2025, Alexandr Esilevich
//
// Distributed under the Boost Software License.
// See accompanying file LICENSE for license information.
//

/// \file position.hpp
/// Contains definition of the position class.

#pragma once

#include <cassert>
#include <cstddef>
#include <cstdint>


namespace tmvc {


/// Represents position in text
struct position {
    uint64_t line;      ///< Line number
    uint64_t column;    ///< Column number

    /// Constructs zero text position
    constexpr position():
        position{0, 0} {}

    /// Constructs text position with specified line and column numbers
    constexpr position(uint64_t l, uint64_t c):
        line{l}, column{c} {}

    /// Returns true if two positions are equal
    constexpr bool operator==(const position & p) const {
        return line == p.line && column == p.column;
    }

    /// Returns true if this position is less that another position p
    constexpr bool operator<(const position & p) const {
        if (line == p.line) {
            return column < p.column;
        }

        return line < p.line;
    }

    /// Returns true if this position is not equal to another position p
    constexpr bool operator!=(const position & p) const {
        return !(*this == p);
    }

    /// Returns true if this position is less or equal to another position p
    constexpr bool operator<=(const position & p) const {
        return *this < p || *this == p;
    }

    /// Returns true if this position is greater that another position p
    constexpr bool operator>(const position & p) const {
        return !(*this <= p);
    }

    /// Returns true if this position is greater or equal to another position p
    constexpr bool operator>=(const position & p) const {
        return !(*this < p);
    }
};


}
