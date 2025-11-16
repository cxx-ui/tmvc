// Copyright (c) 2025, Alexandr Esilevich
//
// Distributed under the Boost Software License.
// See accompanying file LICENSE for license information.
//

/// \file simple_viewport_position_model.hpp
/// Contains definition of the simple_viewport_position_model class.

#pragma once

#include "viewport_position_model.hpp"
#include <cstdint>


namespace tmvc {


/// Simple viewport position model that only stores position and does not contain
/// any logic.
class simple_viewport_position_model {
public:
    /// Constructs viewport position model with specified references
    /// to text model and selection model, and current position
    simple_viewport_position_model(uint64_t p = 0):
        pos_{p} {}

    /// Returns current viewport position
    uint64_t pos() const { return pos_; }

    /// Sets current viewport position
    void set_pos(uint64_t p) {
        if (pos_ == p) {
            return;
        }

        pos_ = p;
        changed();
    }

    /// Sets number of visible lines in viewport. Does nothing
    void set_visible_lines_count(uint64_t cnt) {}

    /// The changed signal is emitted when current position changed
    signal<void ()> changed;


private:
    uint64_t pos_;                  ///< Current viewport position
};


static_assert(viewport_position_model<simple_viewport_position_model>);


}
