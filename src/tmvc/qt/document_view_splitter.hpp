// Copyright (c) 2025, Alexandr Esilevich
//
// Distributed under the Boost Software License.
// See accompanying file LICENSE for license information.
//

/// \file document_view_splitter.hpp
/// Contains definition of the document_view_splitter class.

#pragma once

#include <QSplitter>


namespace tmvc::qt {


/// Improved version of QSplitter. Calculates proper preferred width taking into account
/// width of handle
class document_view_splitter: public QSplitter {
public:
    /// Constructs splitter with specified pointer to parent widget
    document_view_splitter(QWidget * parent = nullptr):
        QSplitter{parent} {}

    /// Calculates size hint for widget
    QSize sizeHint() const override {
        if (count() == 0) {
            return QSplitter::sizeHint();
        }

        // calculating sum of widths of all widgets in splitter
        auto new_width = 0;
        for (int i = 0, cnt = count(); i < cnt; ++i) {
            auto w = widget(i);
            new_width += w->sizeHint().width();
        }

        // adding width of all handles
        // NOTE: this is the main difference from standard QSplitter
        new_width += handleWidth() * (count() - 1);

        auto res = QSplitter::sizeHint();
        res.setWidth(new_width);
        return res;
    }
};


}
