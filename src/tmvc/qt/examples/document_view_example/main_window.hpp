// Copyright (c) 2025, Alexandr Esilevich
//
// Distributed under the Boost Software License.
// See accompanying file LICENSE for license information.
//

/// \file main_window.hpp
/// Contains definition of the main_window class.

#include "../../../text_document_model.hpp"
#include "../../../preserving_viewport_position_model.hpp"
#include "../../../simple_text_model.hpp"
#include "../../../text_line_numbers_model.hpp"
#include <QMainWindow>


/// Main window for the text viewer example application
class main_window: public QMainWindow {
public:
    /// Constructs main window
    explicit main_window();

private:
    /// Called when user selects open menu item
    void open_file();

    using text_model_t = tmvc::wsimple_text_model;
    using document_model_t = tmvc::text_document_model<text_model_t>;

    text_model_t text_;
    document_model_t doc_{text_};
};
