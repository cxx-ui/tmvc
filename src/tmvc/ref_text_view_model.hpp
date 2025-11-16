// Copyright (c) 2025, Alexandr Esilevich
//
// Distributed under the Boost Software License.
// See accompanying file LICENSE for license information.
//

/// \file ref_text_view_model.hpp
/// Contains definition of the ref_text_view_model class

#pragma once

#include "text_view_model.hpp"


namespace tmvc {


/// Text view model that stores all components by reference
template <
    text_model TextModel,
    selection_model SelectionModel,
    viewport_position_model ViewportPositionModel,
    selection_controller Controller
>
class ref_text_view_model {
public:
    /// Type of text model
    using text_model_t = TextModel;

    /// Type of selection model
    using selection_model_t = SelectionModel;

    /// Type of viewport position model
    using viewport_position_model_t = ViewportPositionModel;

    /// Type of controller
    using controller_t = Controller;


    /// Constructs text view model with specified references to all components
    ref_text_view_model(text_model_t & txt,
                        selection_model_t & sel,
                        viewport_position_model_t & v_pos,
                        controller_t & cntrl):
        text_{txt}, selection_{sel}, viewport_pos_{v_pos}, controller_{cntrl} {}

    /// Returns reference to text model
    text_model_t & text() { return text_; }

    /// Returns reference to selection model
    selection_model_t & selection() { return selection_; }

    /// Returns reference to viewport position model
    viewport_position_model_t & viewport_pos() { return viewport_pos_; }

    /// Returns reference to controller
    controller_t & controller() { return controller_; }

private:
    text_model_t & text_;                       ///< Reference to text model
    selection_model_t & selection_;             ///< Reference to selection model
    viewport_position_model_t & viewport_pos_;  ///< Reference to viewport position model
    controller_t & controller_;                 ///< Reference to controller
};


}
