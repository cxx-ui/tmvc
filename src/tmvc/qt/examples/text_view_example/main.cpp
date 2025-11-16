// Copyright (c) 2025, Alexandr Esilevich
//
// Distributed under the Boost Software License.
// See accompanying file LICENSE for license information.
//

/// \file main.cpp
/// Contains main function for the text_viewer example

#include "application.hpp"
#include "main_window.hpp"


int main(int argc, char * argv[]) {
    application app{argc, argv};
    main_window wnd;
    wnd.show();
    return app.exec();
}
