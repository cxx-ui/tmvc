// Copyright (c) 2025, Alexandr Esilevich
//
// Distributed under the Boost Software License.
// See accompanying file LICENSE for license information.
//

/// \file plain_text_view.ipp
/// Contains implementation of the basic_plain_text_view template.

#include "../impl/string.hpp"
#include "../impl/utils.hpp"
#include <QTextBlock>
#include <iostream>


namespace tmvc::qt::native {



template <typename QtTextEdit, text_model TextModel>
void view_text_update_strategy<QtTextEdit, TextModel>::initialize(
        core_view_base<QtTextEdit> * view) {

    // setting text in view
    view->setPlainText(impl::std_string_to_qstring(characters_str(text_)));

    // listening for changes in text model
    after_inserted_con_ = text_.after_inserted.connect([this, view](const range & r) {
        on_model_inserted(view, r);
    });
    after_erased_con_ = text_.after_erased.connect([this, view](const range & r) {
        on_model_erased(view, r);
    });
    after_replaced_con_ = text_.after_replaced.connect([this, view](const range & r) {
        on_model_replaced(view, r);
    });
}


template <typename QtTextEdit, text_model TextModel>
void view_text_update_strategy<QtTextEdit, TextModel>::on_model_inserted(
        core_view_base<QtTextEdit> * view, const range & r) {

    if (ignore_model_changes_) {
        return;
    }

    auto chars = impl::std_string_to_qstring(characters_str(text_, r));

    view->set_is_updating_view(true);
    impl::get_qt_cursor_for_selection(view, r.start, r.start).insertText(chars);
    view->set_is_updating_view(false);
}


template <typename QtTextEdit, text_model TextModel>
void view_text_update_strategy<QtTextEdit, TextModel>::on_model_erased(
        core_view_base<QtTextEdit> * view, const range & r) {

    if (ignore_model_changes_) {
        return;
    }

    view->set_is_updating_view(true);
    impl::get_qt_cursor_for_selection(view, r.start, r.end).removeSelectedText();
    view->set_is_updating_view(false);
}


template <typename QtTextEdit, text_model TextModel>
void view_text_update_strategy<QtTextEdit, TextModel>::on_model_replaced(
        core_view_base<QtTextEdit> * view, const range & r) {

    if (ignore_model_changes_) {
        return;
    }

    auto chars = impl::std_string_to_qstring(characters_str(text_, r));

    // preserving cursor position when replacing text
    auto [anchor_pos, pos] = impl::get_selection_from_text_edit(view);

    view->set_is_updating_view(true);
    impl::get_qt_cursor_for_selection(view, r.start, r.end).insertText(chars);
    view->setTextCursor(impl::get_qt_cursor_for_selection(view, anchor_pos, pos));
    view->set_is_updating_view(false);
}


////////////////////////////////////////////////////////////////////////////////


template <typename QtTextEdit, editable_text_model TextModel>
void model_text_update_strategy<QtTextEdit, TextModel>::initialize(
        core_view_base<QtTextEdit> * view) {

    // connecting to model signals in base class
    base_t::initialize(view);

    // listening for changes of text in view
    QObject::connect(view->document(), &QTextDocument::contentsChange,
    [this, view](int pos, int removed, int inserted) {
        if (view->is_updating_view()) {
            return;
        }

        auto change_pos_it = std::ranges::next(std::ranges::begin(positions(text_)), pos);
        auto remove_end_pos_it = std::ranges::next(change_pos_it, removed);

        this->set_ignore_model_changes(true);
        
        // removing characters
        if (removed != 0) {
            text_.erase({*change_pos_it, *remove_end_pos_it});
        }

        // inserting characters
        if (inserted != 0) {
            auto inserted_qstr = view->toPlainText().mid(pos, inserted);
            auto inserted_str = impl::qstring_to_std_string<typename TextModel::char_t>(inserted_qstr);
            text_.insert(*change_pos_it, inserted_str);
        }

        this->set_ignore_model_changes(false);
    });
}


}
