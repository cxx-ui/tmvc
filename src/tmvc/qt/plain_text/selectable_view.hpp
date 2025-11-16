// Copyright (c) 2025, Alexandr Esilevich
//
// Distributed under the Boost Software License.
// See accompanying file LICENSE for license information.
//

/// \file selectable_view.hpp
/// Contains definition of the selectable_view and related types.

#pragma once

#include "plain_text_view.hpp"
#include "../../single_selection_model.hpp"


namespace tmvc::qt::plain_text {


/// Selection strategy for core plain text view which uses selection model
/// to store current selection, and synchronizes view and model in both ways.
template <text_model TextModel>
class single_selection_model_strategy {
public:
    /// Type of selection model
    using single_selection_model_t = single_selection_model<TextModel>;

    /// Constructs strategy with specified reference to selection model
    single_selection_model_strategy(single_selection_model_t & sel):
        selection_{sel} {}

    /// Initializes strategy for Qt text edit. Connects to model and edit signals.
    void initialize(plain_text_view_base * view);

private:
    /// Called when selection in model changes
    void on_model_changed(plain_text_view_base * view);

    /// Called when selection in text widget changes
    void on_view_changed(plain_text_view_base * view);


    single_selection_model_t & selection_;         ///< Reference to selection model

    scoped_signal_connection selection_changed_con_;
};


template <text_model TextModel>
using ro_selectable_view_base = core_view <
    true,
    view_text_update_strategy<TextModel>,
    single_selection_model_strategy<TextModel>
>;


/// Read only plain text view that uses selection model to store current selection state.
template <text_model TextModel>
class ro_selectable_view: public ro_selectable_view_base<TextModel> {
public:
    /// Constructs text view with specified references to text and selection models, and
    /// pointer to parent widget
    ro_selectable_view(const TextModel & txt,
                             single_selection_model<TextModel> & sel,
                             QWidget * parent = nullptr):
        ro_selectable_view_base<TextModel>{{}, parent, txt, sel} {}
};


template <editable_text_model TextModel>
using selectable_view_base = core_view <
    false,
    model_text_update_strategy<TextModel>,
    single_selection_model_strategy<TextModel>
>;


/// Plain text view that uses selection model to store current selection state.
template <editable_text_model TextModel>
class selectable_view: public selectable_view_base<TextModel> {
public:
    /// Constructs text view with specified references to text and selection models, and
    /// pointer to parent widget
    selectable_view(TextModel & txt,
                    single_selection_model<TextModel> & sel,
                    QWidget * parent = nullptr):
        selectable_view_base<TextModel>{parent, txt, sel} {}
};


}


namespace tmvc::qt {
    template <typename TextModel>
    using ro_plain_selectable_text_view = plain_text::ro_selectable_view<TextModel>;

    template <typename TextModel>
    using plain_selectable_text_view = plain_text::selectable_view<TextModel>;
}


#include "selectable_view.ipp"
