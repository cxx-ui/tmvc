// Copyright (c) 2025, Alexandr Esilevich
//
// Distributed under the Boost Software License.
// See accompanying file LICENSE for license information.
//

/// \file label_text_view.hpp
/// Contains definition of the label_text_view class.

#pragma once

#include "../text_model.hpp"
#include <QLabel>


namespace tmvc::qt {


/// View for text model that uses QLabel for representing text
template <text_model TextModel>
class label_text_view: public QLabel {
public:
    /// Constructs text view with specified reference to text model
    label_text_view(const TextModel & mdl, QWidget * parent = nullptr);

private:
    /// Updates text in label
    void update_text();

    const TextModel & mdl_;            ///< Reference to model

    scoped_signal_connection after_inserted_con_;
    scoped_signal_connection after_erased_con_;
    scoped_signal_connection after_replaced_con_;
};


}


#include "label_text_view.ipp"

