// Copyright (c) 2025, Alexandr Esilevich
//
// Distributed under the Boost Software License.
// See accompanying file LICENSE for license information.
//

/// \file single_selection_controller.hpp
/// Contains definition of the single_selection_controller class.

#pragma once

#include "single_selection_model.hpp"
#include "std_selection_controller.hpp"


namespace tmvc {


template <text_model TextModel>
class single_selection_controller_base {
public:
    /// Type of selection model
    using selection_model_t = single_selection_model<TextModel>;

    /// Constructs controller with specified references to text model and selection model
    single_selection_controller_base(selection_model_t & sel_mdl):
        selection_{sel_mdl} {}

    /// Returns reference to selection model
    auto & selection() const { return selection_; }

    /// Returns anchor position from selection model
    position anchor_pos() const {
        return selection_.anchor_pos();
    }

    /// Returns current position from selection model
    position pos() const {
        return selection_.pos();
    }

    /// Sets current position and anchor in selection model
    void select_text_impl(const position & anchor_p, const position & p) {
        if (selection_.pos() != p || selection_.anchor_pos() != anchor_p) {
            selection_.set_pos_and_anchor(anchor_p, p);
        }
    }

private:
    selection_model_t & selection_;     ///< Reference to selection model
};


/// Selection controller for text view that works with single selection model.
template <text_model TextModel>
class single_selection_controller:
    public single_selection_controller_base<TextModel>,
    public std_selection_controller <
        single_selection_controller<TextModel>,
        TextModel
    >
{
    using base_t = single_selection_controller_base<TextModel>;
    using selection_base_t = std_selection_controller <
        single_selection_controller<TextModel>,
        TextModel
    >;

public:
    /// Type of selection model
    using selection_model_t = single_selection_model<TextModel>;

    /// Constructs controller with specified references to text model and selection model
    single_selection_controller(const TextModel & text_mdl, selection_model_t & sel_mdl):
        base_t{sel_mdl}, selection_base_t{text_mdl} {}
};


}
