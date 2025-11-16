// Copyright (c) 2025, Alexandr Esilevich
//
// Distributed under the Boost Software License.
// See accompanying file LICENSE for license information.
//

/// \file single_edit_controller.hpp
/// Contains definition of the single_edit_controller class.

#pragma once

#include "single_selection_model.hpp"
#include "single_selection_controller.hpp"
#include "std_edit_controller.hpp"


namespace tmvc {


/// Standard edit controller for text view that works with single selection model.
template <editable_text_model TextModel>
class single_edit_controller:
    public single_selection_controller_base<TextModel>,
    public std_edit_controller <
        single_edit_controller<TextModel>,
        TextModel
    >
{
    using base_t = single_selection_controller_base<TextModel>;
    using edit_base_t = std_edit_controller <
        single_edit_controller<TextModel>,
        TextModel
    >;

public:
    /// Type of text model
    using text_model_t = TextModel;

    /// Type of character
    using char_t = typename TextModel::char_t;

    /// Type of selection model
    using selection_model_t = single_selection_model<TextModel>;

    /// Type of modification history
    using modification_history_t = modification_history<typename text_model_t::char_t>;

    /// Constructs edit controller with specified references to text and selection models
    /// and modification history
    single_edit_controller(text_model_t & text_mdl,
                           selection_model_t & sel_mdl,
                           modification_history_t & hist):
        base_t{sel_mdl}, edit_base_t{text_mdl, hist} {}
};


}
