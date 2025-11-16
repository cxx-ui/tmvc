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


}
