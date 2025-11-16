// Copyright (c) 2025, Alexandr Esilevich
//
// Distributed under the Boost Software License.
// See accompanying file LICENSE for license information.
//

/// \file basic_text_view_model.hpp
/// Contains definition of the basic_text_view_model class.

#pragma once

#include "edit_controller.hpp"
#include "empty_selection_controller.hpp"
#include "empty_selection_model.hpp"
#include "modification.hpp"
#include "preserving_viewport_position_model.hpp"
#include "text_view_model.hpp"
#include <tuple>


namespace tmvc {


/// Text view model that stores all components inside and constructs them in constructor
template <
    text_model TextModel,
    selection_model SelectionModel = empty_selection_model,
    viewport_position_model ViewportPositionModel = preserving_viewport_position_model<TextModel, SelectionModel>,
    selection_controller Controller = empty_selection_controller<typename TextModel::char_t>
>
class basic_text_view_model {
    static constexpr auto has_modification_history = edit_controller<Controller>;

public:
    /// Type of text model
    using text_model_t = TextModel;

    /// Type of selection model
    using selection_model_t = SelectionModel;

    /// Type of viewport position model
    using viewport_position_model_t = ViewportPositionModel;

    /// Type of controller
    using controller_t = Controller;

    /// Type of modification history
    using modification_history_t = std::conditional_t <
        edit_controller<Controller>,
        modification_history<typename text_model_t::char_t>,
        std::tuple<>
    >;


    /// Constructs text view model creating text model from specified parameters
    /// in the case when controller is an edit controller and has modification history
    template <typename ... Args>
    requires (has_modification_history &&
              std::constructible_from<text_model_t, Args...>)
    basic_text_view_model(Args && ... args):
        text_{std::forward<Args>(args)...},
        selection_{text_},
        controller_{text_, selection_, history_} {}

    /// Constructs text view model creating text model from specified parameters
    /// in the case when controller is not an edit controller and does not have
    /// modification history
    template <typename ... Args>
    requires (!has_modification_history &&
              std::constructible_from<text_model_t, Args...>)
    basic_text_view_model(Args && ... args):
        text_{std::forward<Args>(args)...},
        selection_{text_},
        controller_{text_, selection_} {}

    /// Returns reference to text model
    auto & text() { return text_; }

    /// Returns reference to const text model
    const auto & text() const { return text_; }

    /// Returns reference to selection model
    auto & selection() { return selection_; }

    /// Returns reference to const selection model
    const auto & selection() const { return selection_; }

    /// Returns reference to viewport position model
    auto & viewport_pos() { return viewport_pos_; }

    /// Returns reference to const viewport position model
    const auto & viewport_pos() const { return viewport_pos_; }

    /// Returns reference to controller
    auto & controller() { return controller_; }

    /// Returns reference to const controller
    const auto & controller() const { return controller_; }

private:
    text_model_t text_;                 ///< Text model
    selection_model_t selection_;       ///< Selection model

    /// Viewport position model
    viewport_position_model_t viewport_pos_{text_, selection_};

    modification_history_t history_;    ///< Modification history
    controller_t controller_;           ///< Controller
};


}
