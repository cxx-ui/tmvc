// Copyright (c) 2025, Alexandr Esilevich
//
// Distributed under the Boost Software License.
// See accompanying file LICENSE for license information.
//

/// \file selectable_view.ipp
/// Contains implementation of the basic_selectable_view template and related types.


namespace tmvc::qt::native {


template <typename QtTextEdit, text_model TextModel>
void single_selection_model_strategy<QtTextEdit, TextModel>::initialize(
        core_view_base<QtTextEdit> * view) {

    // setting initial position in view
    on_model_changed(view);
    {
        auto cursor = view->textCursor();
        cursor.movePosition(QTextCursor::Start, QTextCursor::MoveAnchor, 1);
        view->setTextCursor(cursor);
    }

    // listening for selection change in model and changing position in text widget
    selection_changed_con_ = selection_.changed.connect([this, view]() {
        on_model_changed(view);
    });

    // listening for cursor position change and anchor change in text widget and updating model
    QObject::connect(view, &QtTextEdit::cursorPositionChanged, [this, view] {
        on_view_changed(view);
    });
    QObject::connect(view, &QtTextEdit::selectionChanged, [this, view] {
        on_view_changed(view);
    });
}


template <typename QtTextEdit, text_model TextModel>
void single_selection_model_strategy<QtTextEdit, TextModel>::on_model_changed(
        core_view_base<QtTextEdit> * view) {

    // setting update flag to prevent model updates in other strategies
    view->set_is_updating_view(true);

    // updating view
    view->setTextCursor(impl::get_qt_cursor_for_selection(view, selection_.anchor_pos(), selection_.pos()));

    // clearing update flag
    view->set_is_updating_view(false);
}


template <typename QtTextEdit, text_model TextModel>
void single_selection_model_strategy<QtTextEdit, TextModel>::on_view_changed(
        core_view_base<QtTextEdit> * view) {

    // don't change model if we are updating view now
    if (view->is_updating_view()) {
        return;
    }

    auto [anchor_pos, pos] = impl::get_selection_from_text_edit(view);

    // changing position in document if not same
    if (selection_.pos() != pos || selection_.anchor_pos() != anchor_pos) {
        selection_.set_pos_and_anchor(anchor_pos, pos);
    }
}


}
