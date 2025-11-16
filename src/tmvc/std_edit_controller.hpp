// Copyright (c) 2025, Alexandr Esilevich
//
// Distributed under the Boost Software License.
// See accompanying file LICENSE for license information.
//

/// \file std_edit_controller.hpp
/// Contains definition of the std_edit_controller template class.

#pragma once

#include "modification.hpp"
#include "selection_model.hpp"
#include "std_selection_controller.hpp"
#include "text_model.hpp"
#include "impl/utils.hpp"


namespace tmvc {


/// Implementation of simple edit controller. Uses CRTP pattern to implement
/// non-virtual dynamic dispatch.
template <typename Derived, editable_text_model TextModel>
class std_edit_controller: public std_selection_controller<Derived, TextModel> {
    /// Type of base class
    using base_t = std_selection_controller<Derived, TextModel>;

public:
    /// Type of character
    using char_t = typename TextModel::char_t;

    /// Type of text model
    using text_model_t = TextModel;

    /// Type of modification history
    using modification_history_t = modification_history<char_t>;

    /// Type of string
    using string_t = std::basic_string<char_t>;

    /// Constructs edit controller with specified references to text and selection models
    /// and modification history
    std_edit_controller(text_model_t & text_mdl, modification_history_t & hist);

    /// Returns reference to text model
    auto & text() { return text_mdl_; }

    /// Returns const reference to text model
    const auto & text() const { return text_mdl_; }

    /// Returns reference modification history
    auto & history() { return history_; }

    /// Returns reference to const modification history
    const auto & history() const { return history_; }

    /// Returns true if tabs should be expanded to spaces
    bool expand_tabs() const { return expand_tabs_; }

    /// Sets whether tabs should be expanded to spaces
    void set_expand_tabs(bool val) { expand_tabs_ = val; }

    /// Returns expand tab size
    unsigned tab_size() const { return tab_size_; }

    /// Sets expand tab size
    void set_tab_size(unsigned val) { tab_size_ = val; }

    /// Returns true if controller is in overwrite mode
    bool is_overwrite_mode() const { return is_overwrite_mode_; }

    /// Sets current overwrite mode
    void set_overwrite_mode(bool val) { is_overwrite_mode_ = val; }


    ////////////////////////////////////////////////////////////
    // Text editing

    /// Replaces text of entire document
    void set_text(const string_t & t);

    /// Clears all document content
    void clear();


    ////////////////////////////////////////////////////////////
    // User actions

    /// Performs actions when user presses delete button
    void do_delete(bool ctrl, bool shift);

    /// Performs actions when user presses backspace button
    void do_backspace(bool ctrl, bool shift);

    /// Performs actions when user presses enter button
    void do_enter(bool ctrl, bool shift);

    /// Performs actions when user presses tab button
    void do_tab(bool ctrl, bool shift);

    /// Performs action when user presses insert button
    void do_insert(bool ctrl, bool shift);

    /// Performs actions when user presses character button
    void do_char(char_t c);


    ////////////////////////////////////////////////////////////
    // Predefined user actions

    /// Returns true if cut action can be performed now
    bool can_cut() const;

    /// Cuts currently selected text. Returns text what should be placed in clipboard
    std::basic_string<char_t> cut();

    /// Pastes text replacing current selection.
    void paste(const std::basic_string<char_t> & text);

    /// Returns true if delete action can be performed now
    bool can_delete() const;

    /// Performs delete action
    void delete_();

    /// Returns true if undo action can be performed now
    bool can_undo() const;

    /// Performs undo action if can be done
    void undo();

    /// Returns true if redo action can be performed now
    bool can_redo() const;

    /// Performs redo action if can be done
    void redo();


    ////////////////////////////////////////////////////////////
    // Context menu

    /// Creates context menu
    basic_context_menu<char_t> create_context_menu() {
        basic_context_menu<char_t> menu;

        menu.add_action(standard_context_menu_action_type::undo);
        menu.add_action(standard_context_menu_action_type::redo);

        menu.add_separator();

        menu.add_action(standard_context_menu_action_type::cut);
        menu.add_action(standard_context_menu_action_type::copy);
        menu.add_action(standard_context_menu_action_type::paste);

        menu.add_separator();

        menu.add_action(standard_context_menu_action_type::select_all);
        return menu;
    }


    ////////////////////////////////////////////////////////////
    // Document actions
    // TODO: move to separate controller

    /// Performs text modification required before saving document (removing lines with spaces, etc)
    void do_before_save();


protected:
    /// Insert characters at specified position.
    /// If current position in document is after or equal to the insert
    /// position then it moves forward with moved content.
    /// If current position in document is before the insert position
    /// then it does not change.
    /// Returns range of inserted characters.
    range insert_chars(const position & p, const string_t & chars);

    /// Inserts characters at current position and moves current position
    /// to end of inserted range
    void insert_chars(const string_t & chars);

    /// Deletes characters from specified range. If current document position
    /// is within specified range then it changes to the start of the range.
    /// If current document position is after specified range when it
    /// moves backward to the number characters deleted. If current document
    /// position is before specified range when it does not change.
    void delete_chars(const range & r);

    /// Deletes characters from current selection and moves current position
    /// in document to start of removed range.
    void delete_chars();

    /// Performs undo action for specified modification
    void perform_undo(const modification<char_t> & mod);

    /// Performs redo action for specified modification
    void perform_redo(const modification<char_t> & mod);

    /// Enables or disables modification history
    void set_modification_history_enabled(bool val) { enable_history_  = val; }


private:
    /// Returns current position. Uses CTPR pattern to call method of derived class.
    position derived_pos() const {
        return static_cast<const Derived*>(this)->pos();
    }

    /// Returns selection anchor position. Uses CTPR pattern to call method of derived class.
    position derived_anchor_pos() const {
        return static_cast<const Derived*>(this)->anchor_pos();
    }

    /// Sets current position and anchor. Uses CTPR pattern to call method of derived class.
    void derived_set_pos_and_anchor(const position & anchor_p, const position & p) {
        static_cast<Derived*>(this)->select_text(anchor_p, p);
    }

    /// Sets position of cursor with moving anchor. Uses CTPR pattern to call method of derived class.
    void derived_set_pos_move_anchor(const position & p) {
        derived_set_pos_and_anchor(p, p);
    }

    /// Sets position of cursor in controller without moving selection anchor
    void derived_set_pos_keep_anchor(const position & p) {
        derived_set_pos_and_anchor(derived_anchor_pos(), p);
    }

    /// Returns position one character forward from specified position.
    /// Returns the original position if specified position is at the end of text.
    position get_pos_forward(const position & p) const {
        if (p == end_pos(text_mdl_)) {
            return p;
        }

        return next_pos(text_mdl_, p);
    }

    /// Returns position one character backward from specified position.
    /// Returns the original position if specified position is at the beginning of text.
    position get_pos_backward(const position & p) const {
        if (p == begin_pos(text_mdl_)) {
            return p;
        }

        return prev_pos(text_mdl_, p);
    }

    /// Returns range of selected text
    range selected_range() const {
        return impl::selected_range(derived_anchor_pos(), derived_pos());
    }

    /// Searches for indent characters located at the beginning of first non empty
    /// line looking back starting from specified line number
    std::tuple<string_t, size_t> find_indent_chars(size_t lnum) const;

    /// Removes spaces from current line if line contains only spaces
    void remove_all_spaces_current_line();


    text_model_t & text_mdl_;                   ///< Reference to text model
    modification_history_t & history_;          ///< Reference to modification history

    bool expand_tabs_ = false;                  ///< Should tabs be expanded into spaces
    unsigned tab_size_ = 4;                     ///< Tab size
    bool is_overwrite_mode_ = false;            ///< True if current mode is overwrite
    bool enable_history_ = true;                ///< Should modifications be recorded into history
};


}


#include "std_edit_controller.ipp"
