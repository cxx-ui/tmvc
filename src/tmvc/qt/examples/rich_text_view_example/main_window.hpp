// Copyright (c) 2025, Alexandr Esilevich
//
// Distributed under the Boost Software License.
// See accompanying file LICENSE for license information.
//

/// \file main_window.hpp
/// Contains definition of the main_window class.

#include "../../../preserving_viewport_position_model.hpp"
#include "../../../basic_text_view_model.hpp"
#include "../../../simple_text_model.hpp"
#include "../../../single_edit_controller.hpp"
#include "../../../single_selection_model.hpp"
#include "../../../modification.hpp"
#include <QMainWindow>


/// Main window for the rich text viewer example application
class main_window: public QMainWindow {
public:
    /// Constructs main window
    explicit main_window();

private:
    /// Called when user selects open menu item
    void open_file();

    using text_model_t = tmvc::wsimple_text_model;
    using selection_model_t = tmvc::single_selection_model<text_model_t>;
    using controller_t = tmvc::single_edit_controller<text_model_t>;

    text_model_t text_;
    selection_model_t selection_{text_};
    tmvc::modification_history<wchar_t> history_;
    controller_t controller_{text_, selection_, history_};
};
