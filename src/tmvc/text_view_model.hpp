// Copyright (c) 2025, Alexandr Esilevich
//
// Distributed under the Boost Software License.
// See accompanying file LICENSE for license information.
//

/// \file text_view_model.hpp
/// Contains definition of the text_view_model concept.

#pragma once

#include "selection_controller.hpp"
#include "selection_model.hpp"
#include "text_model.hpp"
#include "viewport_position_model.hpp"


namespace tmvc{


/// Represents model for displaying text in text view widgets.
/// Contains text model, selection model, viewport position model, and controller.
template <typename Model>
concept text_view_model = requires(Model & model) {
    /// Type of text model
    requires text_model<typename Model::text_model_t>;

    /// Type of selection model
    requires selection_model<typename Model::selection_model_t>;

    /// Type of viewport position model
    requires viewport_position_model<typename Model::viewport_position_model_t>;

    /// Type of controller
    requires selection_controller<typename Model::controller_t>;

    /// Returns reference to text model
    requires std::is_lvalue_reference_v<decltype(model.text())>;
    requires std::same_as<std::decay_t<decltype(model.text())>, typename Model::text_model_t>;

    /// Returns reference to selection model
    requires std::is_lvalue_reference_v<decltype(model.selection())>;
    requires std::same_as<std::decay_t<decltype(model.selection())>, typename Model::selection_model_t>;

    /// Returns reference to viewport position model
    requires std::is_lvalue_reference_v<decltype(model.viewport_pos())>;
    requires std::same_as<std::decay_t<decltype(model.viewport_pos())>, typename Model::viewport_position_model_t>;

    /// Returns reference to controller
    requires std::is_lvalue_reference_v<decltype(model.controller())>;
    requires std::same_as<std::decay_t<decltype(model.controller())>, typename Model::controller_t>;
};


}
