// Copyright (c) 2025, Alexandr Esilevich
//
// Distributed under the Boost Software License.
// See accompanying file LICENSE for license information.
//

/// \file editor_model.hpp
/// Contains definition of the basic_document_model class and associated type aliases.

#pragma once

#include "basic_text_view_model.hpp"
#include "document_model.hpp"
#include "modification.hpp"
#include "selection_model.hpp"
#include "viewport_position_model.hpp"
#include "preserving_viewport_position_model.hpp"
#include "simple_text_model.hpp"
#include "single_edit_controller.hpp"
#include "text_line_numbers_model.hpp"
//#include <ranges.hpp>
#include <string>


namespace tmvc {


/// Text document model. Used with document_view to display text editor with line numbers.
template <
    text_model TextModel,
    selection_model SelectionModel = simple_single_selection_model<TextModel>,
    viewport_position_model ViewportPositionModel = preserving_viewport_position_model<TextModel, SelectionModel>,
    selection_controller_for<TextModel> Controller = single_edit_controller<TextModel>
>
class text_document_model {
    /// Type of modification history
    using modification_history_t = std::conditional_t <
        edit_controller<Controller>,
        modification_history<typename TextModel::char_t>,
        std::tuple<>
    >;

public:
    /// Type of text model
    using text_model_t = TextModel;

    /// Type of selection model
    using selection_model_t = SelectionModel;

    /// Type of viewport position model
    using viewport_position_model_t = ViewportPositionModel;

    /// Type of controller
    using controller_t = Controller;

    /// True if document is editable
    static constexpr auto is_editable = edit_controller<controller_t>;

    /// Constructs document model with specified reference to text model
    text_document_model(text_model_t & txt) requires (!is_editable):
        text_{txt}, controller_{text_, selection_} {}

    /// Constructs document model with specified reference to text model
    text_document_model(text_model_t & txt) requires (is_editable):
        text_{txt}, controller_{text_, selection_, history_} {}

    /// Returns reference to text model
    auto & text() { return text_; }

    /// Returns const reference to text model
    const auto & text() const { return text_; }

    /// Returns reference to selection model
    auto & selection() { return selection_; }

    /// Returns const reference to selection model
    const auto & selection() const { return selection_; }

    /// Returns reference to controller
    auto & controller() { return controller_; }

    /// Returns const reference to controller
    const auto & controller() const { return controller_; }

    /// Returns reference to modification history
    auto & history() requires (is_editable) { return history_; }

    /// Returns const reference to modification history
    const auto & history() const requires (is_editable) { return history_; }

    /// Returns reference to viewport position model
    auto & viewport_pos() { return viewport_pos_; }

    /// Returns reference to line numbers model
    auto & line_numbers() { return line_numbers_; }

    /// Return tuple of document sections
    auto sections() {
        return std::tuple {
            document_section {
                text_,
                selection_,
                controller_
            }
        };
    }

private:
    text_model_t & text_;                       ///< Reference to text model
    selection_model_t selection_{text_};        ///< Text selection model

    /// Viewport position model
    viewport_position_model_t viewport_pos_{text_, selection_};

    modification_history_t history_;            ///< Modification history
    Controller controller_;                     ///< Controller

    /// Line numbers text model
    text_line_numbers_model<text_model_t> line_numbers_{text_};
};


}
