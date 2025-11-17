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


namespace tmvc::qt::native {


/// Selection strategy for core plain text view which uses selection model
/// to store current selection, and synchronizes view and model in both ways.
template <typename QtTextEdit, text_model TextModel>
class single_selection_model_strategy {
public:
    /// Type of selection model
    using single_selection_model_t = single_selection_model<TextModel>;

    /// Constructs strategy with specified reference to selection model
    single_selection_model_strategy(single_selection_model_t & sel):
        selection_{sel} {}

    /// Initializes strategy for Qt text edit. Connects to model and edit signals.
    void initialize(core_view_base<QtTextEdit> * view);

private:
    /// Called when selection in model changes
    void on_model_changed(core_view_base<QtTextEdit> * view);

    /// Called when selection in text widget changes
    void on_view_changed(core_view_base<QtTextEdit> * view);


    single_selection_model_t & selection_;         ///< Reference to selection model

    scoped_signal_connection selection_changed_con_;
};


template <typename QtTextEdit, text_model TextModel>
using ro_selectable_view_base = core_view <
    QtTextEdit,
    true,
    view_text_update_strategy<QtTextEdit, TextModel>,
    single_selection_model_strategy<QtTextEdit, TextModel>
>;


/// Read only text view that uses single selection model to store current selection state.
template <typename QtTextEdit, text_model TextModel>
class ro_selectable_view_t: public ro_selectable_view_base<QtTextEdit, TextModel> {
public:
    /// Constructs text view with specified references to text and selection models, and
    /// pointer to parent widget
    ro_selectable_view_t(const TextModel & txt,
                         single_selection_model<TextModel> & sel,
                         QWidget * parent = nullptr):
        ro_selectable_view_base<QtTextEdit, TextModel>{parent, txt, sel} {}
};


/// Read only Qt plain text view (QPlainTextEdit) that uses selection model to
/// store current selection state.
template <text_model TextModel>
using ro_plain_selectable_view = ro_selectable_view_t<QPlainTextEdit, TextModel>;


/// Read only Qt rich text view (QTextEdit) that uses selection model to
/// store current selection state.
template <text_model TextModel>
using ro_selectable_view = ro_selectable_view_t<QTextEdit, TextModel>;


template <editable_text_model TextModel>
using selectable_view_base = core_view <
    QPlainTextEdit,
    false,
    model_text_update_strategy<QPlainTextEdit, TextModel>,
    single_selection_model_strategy<QPlainTextEdit, TextModel>
>;


/// Plain text view that uses selection model to store current selection state.
template <editable_text_model TextModel>
class plain_selectable_view: public selectable_view_base<TextModel> {
public:
    /// Constructs text view with specified references to text and selection models, and
    /// pointer to parent widget
    plain_selectable_view(TextModel & txt,
                          single_selection_model<TextModel> & sel,
                          QWidget * parent = nullptr):
        selectable_view_base<TextModel>{parent, txt, sel} {}
};


}


#include "selectable_view.ipp"
