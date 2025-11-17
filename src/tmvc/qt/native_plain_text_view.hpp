// Copyright (c) 2025, Alexandr Esilevich
//
// Distributed under the Boost Software License.
// See accompanying file LICENSE for license information.
//

/// \file native_plain_text_view.hpp
/// Aggregate header for all Qt native plain text views

#pragma once

#include "native/controllable_view.hpp"
#include "native/selectable_view.hpp"
#include "native/plain_text_view.hpp"


namespace tmvc::qt {
    template <text_model TextModel>
    using ro_native_plain_text_view = native::ro_plain_text_view<TextModel>;

    template <text_model TextModel>
    using native_plain_text_view = native::plain_text_view<TextModel>;

    template <typename TextModel>
    using ro_native_plain_selectable_text_view = native::ro_plain_selectable_view<TextModel>;

    template <typename TextModel>
    using native_plain_selectable_text_view = native::plain_selectable_view<TextModel>;

    template <text_model TextModel, selection_controller_for<TextModel> Controller>
    using native_plain_controllable_text_view = native::plain_controllable_view<TextModel, Controller>;
}
