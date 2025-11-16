// Copyright (c) 2025, Alexandr Esilevich
//
// Distributed under the Boost Software License.
// See accompanying file LICENSE for license information.
//

/// \file label_text_view.ipp
/// Contains implementation of the basic_label_text_view template.

#include "impl/string.hpp"
#include <type_traits>


namespace tmvc::qt {


template <text_model TextModel>
label_text_view<TextModel>::label_text_view(const TextModel & mdl, QWidget * parent):
QLabel{parent}, mdl_{mdl} {
    // connecting to model signals
    after_inserted_con_ = mdl_.after_inserted.connect([this](auto && range) {
        update_text();
    });
    after_erased_con_ = mdl_.after_erased.connect([this](auto && range) {
        update_text();
    });
    after_replaced_con_ = mdl_.after_replaced.connect([this](auto && range) {
        update_text();
    });

    // updating text
    update_text();
}


template <text_model TextModel>
void label_text_view<TextModel>::update_text() {
    setText(impl::std_string_to_qstring(string(mdl_)));
}


}
