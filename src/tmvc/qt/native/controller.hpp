// Copyright (c) 2025, Alexandr Esilevich
//
// Distributed under the Boost Software License.
// See accompanying file LICENSE for license information.
//

/// \file controller.hpp
/// Contains definitions of controller related types for native Qt text views.

#pragma once

#include "../../edit_controller.hpp"


namespace tmvc::qt::native {


/// Standard Qt selection controller
struct std_selection_controller {};


/// Standard Qt edit constroller
struct std_edit_controller {};


/// Standard Qt controller concept
template <typename Controller>
concept qt_std_controller = std::same_as<Controller, std_selection_controller> ||
                            std::same_as<Controller, std_edit_controller>;


/// Selection controller concept
template <typename Controller>
concept qt_selection_controller = selection_controller<Controller> ||
                                  qt_std_controller<Controller>;

/// Selection controller for model concept
template <typename Controller, typename Model>
concept qt_selection_controller_for = selection_controller_for<Controller, Model> ||
                                      qt_std_controller<Controller>;


/// Edit controller concept
template <typename Controller>
concept qt_edit_controller = edit_controller<Controller> ||
                             std::same_as<Controller, std_edit_controller>;


}
