// Copyright (c) 2025, Alexandr Esilevich
//
// Distributed under the Boost Software License.
// See accompanying file LICENSE for license information.
//

/// \file selection_model.hpp
/// Contains definitions of classes and concepts realted to Qt selection models.

#pragma once

#include "../../single_selection_model.hpp"


namespace tmvc::qt::native {


/// Standard Qt selection model
struct std_selection_model {};


/// Qt selection model concept for text model
template <typename SelectionModel, typename TextModel>
concept qt_selection_model = single_selection_model<SelectionModel> ||
                             std::same_as<SelectionModel, std_selection_model>;


}
