// Copyright (c) 2025, Alexandr Esilevich
//
// Distributed under the Boost Software License.
// See accompanying file LICENSE for license information.
//

/// \file native_text_view.hpp
/// Aggregate header for all Qt native text views

#pragma once

#include "native/controllable_view.hpp"
#include "native/selectable_view.hpp"
#include "native/plain_text_view.hpp"


namespace tmvc::qt {
    /// Read only text view that uses QTextEdit to represent rich text model
    template <text_model TextModel>
    using ro_native_text_view = native::ro_text_view<TextModel>;

    template <typename TextModel>
    using ro_native_selectable_text_view = native::ro_selectable_view<TextModel>;

    template <text_model TextModel, selection_controller_for<TextModel> Controller>
    using native_controllable_text_view = native::controllable_view<TextModel, Controller>;
}
