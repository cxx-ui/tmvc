// Copyright (c) 2025, Alexandr Esilevich
//
// Distributed under the Boost Software License.
// See accompanying file LICENSE for license information.
//

/// \file text_view.hpp
/// Contains definition of the text_view type.

#pragma once

#include "text_view_impl.hpp"
#include <QTextEdit>


namespace tmvc::qt::native {


/// Native Qt Rich text view based on QTextEdit widget
template <
    text_model Model,
    qt_selection_model<Model> SelectionModel,
    qt_selection_controller_for<Model> Controller
>
using text_view = text_view_impl<QTextEdit, Model, SelectionModel, Controller>;


}
