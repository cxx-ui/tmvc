// Copyright (c) 2025, Alexandr Esilevich
//
// Distributed under the Boost Software License.
// See accompanying file LICENSE for license information.
//

/// \file plain_text_view.hpp
/// Contains definition of the basic_plain_text_view template and related type aliases.

#pragma once

#include "core_view.hpp"
#include "../../text_model.hpp"
#include <QPlainTextEdit>


namespace tmvc::qt::native {


/// Text strategy for core plain text view which updates text in view when
/// text changes in a model
template <text_model TextModel>
class view_text_update_strategy {
public:
    /// Non-copyable
    view_text_update_strategy(const view_text_update_strategy &) = delete;

    /// Movable
    view_text_update_strategy(view_text_update_strategy &&) = default;

    /// Constructs strategy with specified reference to text model
    view_text_update_strategy(const TextModel & txt):
        text_{txt} {}

    /// Initializes strategy for QPlainTextEdit.
    /// Connects to model signals with handlers which update QPlainTextEdit.
    void initialize(plain_text_view_base * view);


protected:
    /// Sets whether changes in model should be ignored
    void set_ignore_model_changes(bool val) { ignore_model_changes_ = val; }


private:
    /// Called after characters are inserted in model
    void on_model_inserted(plain_text_view_base * view, const range & r);

    /// Called after characters are erased from model
    void on_model_erased(plain_text_view_base * view, const range & r);

    /// Called after characters are replaced in model
    void on_model_replaced(plain_text_view_base * view, const range & r);


    const TextModel & text_;                ///< Reference to text model
    bool ignore_model_changes_ = false;     ///< Should changes in model be ignored?

    // connections to model signals
    scoped_signal_connection after_inserted_con_;
    scoped_signal_connection after_erased_con_;
    scoped_signal_connection after_replaced_con_;
};


/// Text strategy for core plain text view which updates text in a view when
/// text in changes in a model, and updates text in model when text changes in a view
template <editable_text_model TextModel>
class model_text_update_strategy: public view_text_update_strategy<TextModel> {
    /// Type of base class
    using base_t = view_text_update_strategy<TextModel>;

public:
    /// Constructs strategy with specified reference to text model
    model_text_update_strategy(TextModel & text):
        base_t{text}, text_{text} {}

    /// Initializes strategy for QPlainTextEdit. Connects to model signals and view signals
    void initialize(plain_text_view_base * view);

private:
    TextModel & text_;      ///< Reference to text model
};


template <text_model TextModel>
using ro_plain_text_view_base = core_view <
    true,
    view_text_update_strategy<TextModel>
>;


/// Read only text view that uses QPlainTextEdit to represent text model
template <text_model TextModel>
class ro_plain_text_view: public ro_plain_text_view_base<TextModel> {
    /// Type of base class
    using base_t = ro_plain_text_view_base<TextModel>;

public:
    /// Constructs text view with specified reference to text model, controller, and pointer to parent
    ro_plain_text_view(const TextModel & mdl, QWidget * parent = nullptr):
        ro_plain_text_view_base<TextModel>{parent, mdl} {}
};


template <editable_text_model TextModel>
using plain_text_view_x_base = core_view <
    false,
    model_text_update_strategy<TextModel>
>;


/// Text view that uses QPlainTextEdit to represent and edit text model
template <editable_text_model TextModel>
class plain_text_view: public plain_text_view_x_base<TextModel> {
public:
    /// Constructs text view with specified reference to text model, controller and pointer to parent
    plain_text_view(TextModel & mdl, QWidget * parent = nullptr):
        plain_text_view_x_base<TextModel>{parent, mdl} {}
};


}


namespace tmvc::qt {
    template <text_model TextModel>
    using native_ro_plain_text_view = native::ro_plain_text_view<TextModel>;

    template <text_model TextModel>
    using native_plain_text_view = native::plain_text_view<TextModel>;
}


#include "plain_text_view.ipp"
