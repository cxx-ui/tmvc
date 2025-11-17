// Copyright (c) 2025, Alexandr Esilevich
//
// Distributed under the Boost Software License.
// See accompanying file LICENSE for license information.
//

/// \file controllable_view.hpp
/// Contains definition of the basic_controllable_view template and related classes.

#pragma once

#include "event_filter_strategy.hpp"
#include "selectable_view.hpp"
#include "../../edit_controller.hpp"
#include "../../selection_controller.hpp"


namespace tmvc::qt::native {


/// Selection strategy for core plain text view that displays selection from selection
/// model, and uses selection controller to handle changes of selection in a view
template <
    typename QtTextEdit,
    text_model TextModel,
    selection_controller_with_select_text_for<TextModel> Controller
>
class selection_controller_strategy: public single_selection_model_strategy<QtTextEdit, TextModel> {
public:
    /// Constructs strategy with specified references to selection model and controller
    selection_controller_strategy(single_selection_model<TextModel> & sel, Controller & cntrl):
        single_selection_model_strategy<QtTextEdit, TextModel>{sel}, controller_{cntrl} {}

    /// Initializes strategy for edit control. Connects to model and view signals.
    void initialize(core_view_base<QtTextEdit> * view);

private:
    /// Called when selection changed in view
    void on_view_selection_changed(core_view_base<QtTextEdit> * view);

    Controller & controller_;       ///< Reference to selection controller
};


/// Event filter for handling key events and delegating them to selection controller
template <typename QtTextEdit, selection_controller Controller>
class selection_controller_key_event_filter: public QObject {
public:
    /// Constructs event filter with specified pointer to view and
    /// reference to selection controller
    selection_controller_key_event_filter(Controller & cntrl):
        controller_{cntrl} {}

    /// Filter initialization function. Does nothing
    void initialize(core_view_base<QtTextEdit> * view) {}

protected:
    /// Filters and handles key event for text view
    bool eventFilter(QObject * obj, QEvent * event) override;

private:
    Controller & controller_;           ///< Reference to selection controller
};


/// Text view strategy for handling key events and delegating them to selection controller
template <typename QtTextEdit, selection_controller Controller>
using selection_controller_key_strategy = event_filter_strategy <
    QtTextEdit,
    selection_controller_key_event_filter<QtTextEdit, Controller>
>;


/// Event filter for handling key events in text view and delegating them to edit controller.
/// Also handles focus in event and setting overwrite mode for view.
template <typename QtTextEdit, edit_controller Controller>
class edit_controller_key_event_filter: public QObject {
public:
    /// Constructs event filter with specified pointer to view
    /// and reference to edit controller
    edit_controller_key_event_filter(Controller & cntrl):
        controller_{cntrl} {}

    /// Filter initialization functions. Set view overwrite mode
    void initialize(core_view_base<QtTextEdit> * view);

protected:
    /// Intercepts and handles key event for text view
    bool eventFilter(QObject * obj, QEvent * event) override;

private:
    Controller & controller_;                           ///< Reference to edit controller
    core_view_base<QtTextEdit> * view_ = nullptr; ///< Pointer to text view
};


/// Text view strategy for handling key events and delegating them to edit controller
template <typename QtTextEdit, edit_controller Controller>
using edit_controller_key_strategy = event_filter_strategy <
    QtTextEdit,
    edit_controller_key_event_filter<QtTextEdit, Controller>
>;


/// Text view strategy that displays custom context menu created by selection controller
template <typename QtTextEdit, selection_controller Controller>
class selection_controller_menu_strategy {
public:
    /// Constructs strategy with specified reference to selection controller
    selection_controller_menu_strategy(Controller & cntrl):
        controller_{cntrl} {}

    /// Initializes strategy
    void initialize(core_view_base<QtTextEdit> * view);

protected:
    /// Adds custom menu actions into context menu
    virtual void add_custom_menu_actions(QMenu * menu) {}

private:
    /// Handles menu event
    void on_menu_event(const QPoint & pos);

    Controller & controller_;       ///< Reference to selection controller
};


/// Helper class for selecting controller key strategy based on controller type
template <typename QtTextEdit, selection_controller Controller>
struct controller_key_strategy {
    /// Default is key strategy for selection controller
    using type = selection_controller_key_strategy<QtTextEdit, Controller>;
};

/// Key strategy selection for edit controller
template <typename QtTextEdit, edit_controller Controller>
struct controller_key_strategy<QtTextEdit, Controller> {
    using type = edit_controller_key_strategy<QtTextEdit, Controller>;
};

template <typename QtTextEdit, selection_controller Controller>
using controller_key_strategy_t = typename controller_key_strategy<QtTextEdit, Controller>::type;


template <
    typename QtTextEdit,
    text_model TextModel,
    selection_controller_with_select_text_for<TextModel> Controller
>
using controllable_view_base = core_view <
    QPlainTextEdit,
    !edit_controller<Controller>,        // is read only?
    controller_key_strategy_t<QPlainTextEdit, Controller>,
    view_text_update_strategy<QPlainTextEdit, TextModel>,
    selection_controller_strategy<QPlainTextEdit, TextModel, Controller>,
    selection_controller_menu_strategy<QPlainTextEdit, Controller>
>;


/// Plain text view that also uses selection model to store current selection
/// and selection controller to handle user input
template <text_model TextModel, selection_controller_with_select_text_for<TextModel> Controller>
class plain_controllable_view: public controllable_view_base<QPlainTextEdit, TextModel, Controller> {
public:
    /// Constructs view with specified references to text and selection models,
    /// selection controller, and pointer to parent widget
    plain_controllable_view(TextModel & txt,
                            single_selection_model<TextModel> & sel,
                            Controller & cntrl,
                            QWidget * parent = nullptr):
        controllable_view_base<QPlainTextEdit, TextModel, Controller>{
            parent, {cntrl}, {txt}, {sel, cntrl}, {cntrl}} {}
};


/// Rich text view that also uses selection model to store current selection
/// and selection controller to handle user input
template <text_model TextModel, selection_controller_with_select_text_for<TextModel> Controller>
class controllable_view: public controllable_view_base<QTextEdit, TextModel, Controller> {
public:
    /// Constructs view with specified references to text and selection models,
    /// selection controller, and pointer to parent widget
    controllable_view(TextModel & txt,
                      single_selection_model<TextModel> & sel,
                      Controller & cntrl,
                      QWidget * parent = nullptr):
        controllable_view_base<QTextEdit, TextModel, Controller>{
            parent, {cntrl}, {txt}, {sel, cntrl}, {cntrl}} {}
};


}


#include "controllable_view.ipp"
