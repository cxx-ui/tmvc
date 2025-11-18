// Copyright (c) 2025, Alexandr Esilevich
//
// Distributed under the Boost Software License.
// See accompanying file LICENSE for license information.
//

/// \file native_plain_text_view.hpp
/// Aggregate header for all Qt native plain text views

#pragma once

#include "native/plain_text_view.hpp"


namespace tmvc::qt {
    /// Qt native text view that uses QPlainTextEdit for displaying text
    template <
        text_model Model,
        native::qt_selection_model<Model> SelectionModel,
        native::qt_selection_controller_for<Model> Controller
    >
    using native_plain_text_view = native::plain_text_view<Model, SelectionModel, Controller>;

}
