// Copyright (c) 2025, Alexandr Esilevich
//
// Distributed under the Boost Software License.
// See accompanying file LICENSE for license information.
//

/// \file application.hpp
/// Contains definition of the application class.

#include <QApplication>


/// Rich text view example application main calss
class application: public QApplication {
public:
    /// Constructs application
    application(int & argc, char * argv[]):
        QApplication{argc, argv} {}
};
