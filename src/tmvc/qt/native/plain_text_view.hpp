// Copyright (c) 2025, Alexandr Esilevich
//
// Distributed under the Boost Software License.
// See accompanying file LICENSE for license information.
//

/// \file plain_text_view.hpp
/// Contains definition of the basic_plain_text_view template and related type aliases.

#pragma once

#include "text_view_impl.hpp"
#include <QPlainTextEdit>


namespace tmvc::qt::native {


/// Native Qt Plain text view based on QPlainTextEdit widget
template <
    text_model Model,
    qt_selection_model<Model> SelectionModel,
    qt_selection_controller_for<Model> Controller
>
using plain_text_view = text_view_impl<QPlainTextEdit, Model, SelectionModel, Controller>;


}
