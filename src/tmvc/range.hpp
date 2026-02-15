// Copyright (c) 2025, Alexandr Esilevich
//
// Distributed under the Boost Software License.
// See accompanying file LICENSE for license information.
//

/// \file range.hpp
/// Contains implementation of the range struct.

#pragma once

#include "position.hpp"


namespace tmvc {


/// Represents range in text
struct range {
    position start;
    position end;

    /// Constructs range with specified start and end position
    constexpr range(const position & s, const position & e):
    start{s}, end{e} {
        assert(start <= end && "range start should be <= end");
    }

    /// Returns true if range is equal to another range
    constexpr bool operator ==(const range & r) const {
        return start == r.start && end == r.end;
    }

    /// Returns true if range is empty, i. e. start == end
    constexpr bool empty() const { return start == end; }
};

/// Returns position adjusted after insertion in specified range.
/// If move_eq_pos is true and pos == r.start, position is moved.
inline position adjust_pos_after_insert(const position & pos,
                                        const range & r,
                                        bool move_eq_pos = true) {
    auto new_pos = pos;
    if (r.start < new_pos || (move_eq_pos && r.start == new_pos)) {
        if (new_pos.line == r.start.line) {
            new_pos.column = r.end.column + (new_pos.column - r.start.column);
        }

        new_pos.line += (r.end.line - r.start.line);
    }

    return new_pos;
}

/// Returns position adjusted after erasing characters in specified range.
inline position adjust_pos_after_erase(const position & pos, const range & r) {
    auto new_pos = pos;
    if (new_pos > r.end) {
        if (r.end.line == new_pos.line) {
            assert(new_pos.column > r.end.column && "invalid column for current pos");
            new_pos.line = r.start.line;
            new_pos.column = r.start.column + new_pos.column - r.end.column;
        } else {
            new_pos.line = new_pos.line - (r.end.line - r.start.line);
        }
    } else if (r.start < new_pos) {
        new_pos = r.start;
    }

    return new_pos;
}


}
