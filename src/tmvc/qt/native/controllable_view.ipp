// Copyright (c) 2025, Alexandr Esilevich
//
// Distributed under the Boost Software License.
// See accompanying file LICENSE for license information.
//

/// \file controllable_view.ipp
/// Contains implementation of the basic_controllable_view template and related classes.

#include "../impl/menu.hpp"
#include "../impl/key_controller.hpp"


namespace tmvc::qt::native {


template <text_model TextModel, selection_controller_with_select_text_for<TextModel> Controller>
void selection_controller_strategy<TextModel, Controller>::initialize(plain_text_view_base * view) {
    // connecting to model signals in base strategy
    single_selection_model_strategy<TextModel>::initialize(view);

    // listening for selection and anchor changes
    QObject::connect(view, &QPlainTextEdit::cursorPositionChanged, [this, view] {
        on_view_selection_changed(view);
    });
    QObject::connect(view, &QPlainTextEdit::selectionChanged, [this, view] {
        on_view_selection_changed(view);
    });
}

template <text_model TextModel, selection_controller_with_select_text_for<TextModel> Controller>
void selection_controller_strategy<TextModel, Controller>::on_view_selection_changed(plain_text_view_base * view) {
    // don't update model if we are updating view now
    if (view->is_updating_view()) {
        return;
    }

    auto cursor = view->textCursor();

    auto [anchor_pos, pos] = impl::get_selection_from_text_edit(view);

    // changing position via controller
    controller_.select_text(anchor_pos, pos);
}


////////////////////////////////////////////////////////////////////////////////


template <selection_controller Controller>
bool selection_controller_key_event_filter<Controller>::eventFilter(QObject * obj, QEvent * event) {
    if (event->type() != QEvent::KeyPress) {
        return false;
    }

    auto key_event = static_cast<QKeyEvent*>(event);
    return impl::process_selection_key_event(controller_, key_event);
}


////////////////////////////////////////////////////////////////////////////////


template <edit_controller Controller>
bool edit_controller_key_event_filter<Controller>::eventFilter(QObject * obj, QEvent * event) {
    using char_t = typename Controller::char_t;

    if (event->type() == QEvent::FocusIn) {
        view_->setOverwriteMode(controller_.is_overwrite_mode());
    }

    if (event->type() != QEvent::KeyPress) {
        return false;
    }

    // processing key event with controller
    auto key_event = static_cast<QKeyEvent*>(event);
    bool res = impl::process_edit_key_event(controller_, key_event);

    // setting (possibly changed) overwrite mode in view after processing key event
    view_->setOverwriteMode(controller_.is_overwrite_mode());

    return res;
}


template <edit_controller Controller>
void edit_controller_key_event_filter<Controller>::initialize(plain_text_view_base * view) {
    view_ = view;
    view_->setOverwriteMode(controller_.is_overwrite_mode());
}


////////////////////////////////////////////////////////////////////////////////



template <selection_controller Controller>
void selection_controller_menu_strategy<Controller>::initialize(plain_text_view_base * view) {
    view->setContextMenuPolicy(Qt::CustomContextMenu);
    QObject::connect(view, &QWidget::customContextMenuRequested, [this, view](const QPoint & pos) {
        on_menu_event(view->mapToGlobal(pos));
    });
}


template <selection_controller Controller>
void selection_controller_menu_strategy<Controller>::on_menu_event(const QPoint & pos) {
    auto menu = impl::create_qt_menu_from_context_menu(controller_.create_context_menu(), controller_);
    add_custom_menu_actions(menu.get());
    menu->exec(pos);
}


}
