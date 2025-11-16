// Copyright (c) 2025, Alexandr Esilevich
//
// Distributed under the Boost Software License.
// See accompanying file LICENSE for license information.
//

/// \file std_edit_controller.ipp
/// Contains implementation of the std_edit_controller class.


namespace tmvc {


// Helper for defining space/tab search string depending on character type
template <typename char_t>
struct tab_space_search_str {
    static inline constexpr const char_t * value = "\t ";
};

template <>
struct tab_space_search_str<wchar_t> {
    static inline constexpr const wchar_t * value = L"\t ";
};

template <typename char_t>
inline auto tab_space_search_str_v = tab_space_search_str<char_t>::value;



template <typename Derived, editable_text_model TextModel>
std_edit_controller<Derived, TextModel>::std_edit_controller(text_model_t & text_mdl,
                                                             modification_history_t & hist):
base_t{text_mdl},
text_mdl_{text_mdl},
history_{hist} {
}


template <typename Derived, editable_text_model TextModel>
range std_edit_controller<Derived, TextModel>::insert_chars(const position & p, const string_t & chars) {
    if (chars.empty()) {
        return {p, p};
    }

    auto ins_range = text_mdl_.insert(p, chars);

    if (enable_history_) {
        history_.add(std::make_unique<insert_modification<char_t>>(ins_range, chars));
    }

    return ins_range;
}


template <typename Derived, editable_text_model TextModel>
void std_edit_controller<Derived, TextModel>::insert_chars(const string_t & chars) {
    // removing current selection if not empty
    delete_chars(selected_range());

    // inserting characters at current position
    insert_chars(derived_pos(), chars);
}


template <typename Derived, editable_text_model TextModel>
void std_edit_controller<Derived, TextModel>::delete_chars(const range & r) {
    if (r.start == r.end) {
        return;
    }

    // saving deleted chars for undo action
    auto chars = characters_str(text_mdl_, r);

    text_mdl_.erase(r);

    if (enable_history_) {
        history_.add(std::make_unique<erase_modification<char_t>>(r, chars));
    }
}


template <typename Derived, editable_text_model TextModel>
void std_edit_controller<Derived, TextModel>::delete_chars() {
    delete_chars(selected_range());
}


template <typename Derived, editable_text_model TextModel>
void std_edit_controller<Derived, TextModel>::perform_undo(const modification<char_t> & mod) {
    set_modification_history_enabled(false);

    if (auto insert_mod = dynamic_cast<const insert_modification<char_t>*>(&mod)) {
        derived_set_pos_move_anchor(insert_mod->modification_range().start);
        derived_set_pos_keep_anchor(insert_mod->modification_range().end);
        delete_();
    } else if (auto erase_mod = dynamic_cast<const erase_modification<char_t>*>(&mod)) {
        derived_set_pos_move_anchor(erase_mod->modification_range().start);
        paste(erase_mod->chars());
    } else if (auto replace_mod = dynamic_cast<const replace_modification<char_t>*>(&mod)) {
        derived_set_pos_move_anchor(replace_mod->modification_range().start);
        derived_set_pos_keep_anchor(replace_mod->modification_range().end);
        paste(replace_mod->new_chars());
    } else if (auto group_mod = dynamic_cast<const modification_group<char_t>*>(&mod)) {
        for (auto && mod : group_mod->modifications()) {
            perform_undo(*mod);
        }
    } else {
        assert(false && "unknown modification type");
    }

    set_modification_history_enabled(true);
}


template <typename Derived, editable_text_model TextModel>
void std_edit_controller<Derived, TextModel>::perform_redo(const modification<char_t> & mod) {
    set_modification_history_enabled(false);

    if (auto insert_mod = dynamic_cast<const insert_modification<char_t>*>(&mod)) {
        derived_set_pos_move_anchor(insert_mod->modification_range().start);
        paste(insert_mod->chars());
    } else if (auto erase_mod = dynamic_cast<const erase_modification<char_t>*>(&mod)) {\
        derived_set_pos_move_anchor(erase_mod->modification_range().start);
        derived_set_pos_keep_anchor(erase_mod->modification_range().end);
        delete_();
    } else if (auto replace_mod = dynamic_cast<const replace_modification<char_t>*>(&mod)) {
        derived_set_pos_move_anchor(replace_mod->modification_range().start);
        derived_set_pos_keep_anchor(replace_mod->modification_range().end);
        paste(replace_mod->old_chars());
    } else if (auto group_mod = dynamic_cast<const modification_group<char_t>*>(&mod)) {
        for (auto && mod : group_mod->modifications()) {
            perform_undo(*mod);
        }
    } else {
        assert(false && "unknown modification type");
    }

    set_modification_history_enabled(true);
}


template <typename Derived, editable_text_model TextModel>
void std_edit_controller<Derived, TextModel>::set_text(const string_t & t) {
    // removing existing content
    clear();

    // inserting new content
    insert_chars(derived_pos(), t);

    // moving position to beginning of document
    derived_set_pos_move_anchor({0, 0});

    // clearing modification history
    history_.clear();
}


template <typename Derived, editable_text_model TextModel>
void std_edit_controller<Derived, TextModel>::clear() {
    tmvc::clear(text_mdl_);
}


template <typename Derived, editable_text_model TextModel>
void std_edit_controller<Derived, TextModel>::do_delete(bool ctrl, bool shift) {
    auto sel = selected_range();
    if (sel.start == sel.end) {
        // deleting single character
        auto p_end = this->get_pos_forward(sel.start);
        if (p_end != sel.start) {
            delete_chars({sel.start, p_end});
        }
    } else {
        // deleting selected range
        delete_chars(sel);
    }
}


template <typename Derived, editable_text_model TextModel>
void std_edit_controller<Derived, TextModel>::do_backspace(bool ctrl, bool shift) {
    auto sel = selected_range();
    if (sel.start == sel.end) {
        // backspacing single character
        auto p_start = this->get_pos_backward(sel.start);
        if (p_start != sel.start) {
            delete_chars({p_start, sel.start});
        }
    } else {
        // deleting selected range
        delete_chars(sel);
    }
}


template <typename Derived, editable_text_model TextModel>
void std_edit_controller<Derived, TextModel>::do_enter(bool ctrl, bool shift) {
    // removing all characters from current line if it contains only spaces
    remove_all_spaces_current_line();

    string_t chars;
    chars.push_back(static_cast<char_t>('\n'));

    // adding auto indent characters
    {
        auto sel_begin = selected_range().start;
        auto start_search_line = sel_begin.line;

        // if current selection starts at the beginning of line
        // (not taking into account space characters) when start searching
        // from previous line

        auto sel_line_start = line_str(text_mdl_, start_search_line).substr(0, sel_begin.column);
        if (sel_line_start.find_first_not_of(tab_space_search_str_v<char_t>) == string_t::npos) {
            if (start_search_line != 0) {
                --start_search_line;
            }
        }

        auto search_res = find_indent_chars(start_search_line);
        chars.append(std::get<0>(search_res));

        auto lnum = std::get<1>(search_res);

        bool add_tab = false;

        if (lnum == sel_begin.line) {
            // adding one tab only if last character before current pos is {
            auto l_str = line_str(text_mdl_, lnum);
            auto lbegin = l_str.substr(0, sel_begin.column);
            auto last_char_pos = lbegin.find_last_not_of(tab_space_search_str_v<char_t>);
            if (last_char_pos != string_t::npos) {
                if (l_str.at(last_char_pos) == static_cast<char_t>('{')) {
                    add_tab = true;
                }
            }
        } else if (lnum != SIZE_MAX) {
            // adding one tab if line ends with {
            auto l_str = line_str(text_mdl_, lnum);
            auto last_char_pos = l_str.find_last_not_of(tab_space_search_str_v<char_t>);
            assert(last_char_pos != string_t::npos && "invalid position of last char");

            if (l_str.at(last_char_pos) == static_cast<char_t>('{')) {
                add_tab = true;
            }
        }

        if (add_tab) {
            if (expand_tabs()) {
                string_t spaces(tab_size(), static_cast<char_t>(' '));
                chars.append(spaces);
            } else {
                chars.push_back(static_cast<char_t>('\t'));
            }
        }
    }

    insert_chars(chars);
}


template <typename Derived, editable_text_model TextModel>
void std_edit_controller<Derived, TextModel>::do_tab(bool ctrl, bool shift) {
    transaction trans{text_mdl_, history()};

    // checking if we need add or remove indent
    bool add_or_remove_indent = false;
    if (selected_range().start.line != selected_range().end.line) {
        // multiple lines selected -> need add/remove indent
        add_or_remove_indent = true;
    } else {
        const auto & cline = line_str(text_mdl_, derived_pos().line);
        if (!cline.empty() &&
            selected_range().start.column == 0 &&
            selected_range().end.column == cline.size()) {

            // full line selected -> need add/remove indent
            add_or_remove_indent = true;
        }
    }

    if (add_or_remove_indent) {
        // saving original position and anchor position
        auto orig_pos = derived_pos();
        auto orig_anchor_pos = derived_anchor_pos();

        if (!shift) {
            string_t chars;
            if (expand_tabs()) {
                chars = string_t(tab_size(), static_cast<char_t>(' '));
            } else {
                chars = static_cast<char_t>('\t');
            }

            // adding indent at beginning of all selected lines
            for (auto i = selected_range().start.line, e = selected_range().end.line; i <= e; ++i) {
                trans.insert_characters({i, 0}, chars);
            }

            // updating selection position
            if (orig_pos.line == orig_anchor_pos.line) {
                // single line was selected, selecting entire line again
                if (orig_pos.column == 0) {
                    derived_set_pos_move_anchor({orig_anchor_pos.line, orig_anchor_pos.column + chars.size()});
                    derived_set_pos_keep_anchor({orig_pos.line, 0});
                } else {
                    derived_set_pos_move_anchor({orig_anchor_pos.line, 0});
                    derived_set_pos_keep_anchor({orig_pos.line, orig_pos.column + chars.size()});
                }
            } else {
                derived_set_pos_move_anchor({orig_anchor_pos.line, orig_anchor_pos.column + chars.size()});
                derived_set_pos_keep_anchor({orig_pos.line, orig_pos.column + chars.size()});
            }

        } else {
            auto first_line = selected_range().start.line;
            auto last_line = selected_range().end.line;
            auto anchor_line = derived_anchor_pos().line;
            auto pos_line = derived_pos().line;

            size_t n_anchor_line_removed = 0;
            size_t n_pos_line_removed = 0;

            // removing indent for all selected lines
            for (auto i = first_line; i <= last_line; ++i) {
                size_t n_chars_removed = 0;     // number of characters removed from current line

                const auto & cline = line_str(text_mdl_, i);
                if (cline.size() > 0 && cline[0] == static_cast<char_t>('\t')) {
                    // if first character is tab then remove it
                    trans.erase_characters({{i, 0}, {i, 1}});
                    n_chars_removed = 1;
                } else {
                    // removing up to tab_size_ spaces
                    for (auto c : cline) {
                        if (c != static_cast<char_t>(' ')) {
                            break;
                        }

                        ++n_chars_removed;
                        if (n_chars_removed == tab_size()) {
                            break;
                        }
                    }

                    trans.erase_characters({{i, 0}, {i, n_chars_removed}});
                }

                // saving number of characters removed for anchor/pos lines

                if (i == anchor_line) {
                    n_anchor_line_removed = n_chars_removed;
                }

                if (i == pos_line) {
                    n_pos_line_removed = n_chars_removed;
                }
            }

            // updating selection position

            size_t anchor_col = orig_anchor_pos.column;
            if (anchor_col < n_anchor_line_removed) {
                anchor_col = 0;
            } else {
                anchor_col -= n_anchor_line_removed;
            }

            size_t pos_col = orig_pos.column;
            if (pos_col < n_pos_line_removed) {
                pos_col = 0;
            } else {
                pos_col -= n_pos_line_removed;
            }

            derived_set_pos_move_anchor({orig_anchor_pos.line, anchor_col});
            derived_set_pos_keep_anchor({orig_pos.line, pos_col});
        }

        return;
    }

    if (shift) {
        // removing indent characters before cursor

        // if selected range is not empty then just removing it
        if (!selected_range().empty()) {
            trans.erase_characters(selected_range());
        } else {
            auto pos = selected_range().start;
            const auto & cline = line_str(text_mdl_, pos.line);

            if (pos.column != 0) {
                // if previous character is tab then just removing it
                if (cline[pos.column - 1] == '\t') {
                    trans.erase_characters({{pos.line, pos.column - 1}, {pos.line, pos.column}});
                } else {
                    // calculating number of spaces to remove

                    auto min_col = pos.column;
                    auto tsz = size_t(tab_size());
                    
                    if (pos.column % tsz == 0) {
                        assert(min_col >= tsz && "original column should be > tab size here");
                        min_col -= tsz;
                    } else {
                        min_col -= pos.column % tsz;
                    }

                    auto col = pos.column;
                    while (col > min_col) {
                        if (cline[col - 1] != static_cast<char_t>(' ')) {
                            break;
                        }

                        --col;
                    }

                    trans.erase_characters({{pos.line, col}, pos});
                }
            }
        }
    } else {
        // removing selected characters
        trans.erase_characters(selected_range());

        // adding indent characters after cursor
        if (expand_tabs()) {
            // calculating number of spaces to insert
            auto insert_pos = selected_range().start;
            size_t num_spaces = tab_size() - (insert_pos.column % tab_size());
    
            // inserting spaces
            trans.insert_characters(derived_pos(), string_t(num_spaces, static_cast<char_t>(' ')));
        } else {
            // inserting single tab character
            trans.insert_characters(derived_pos(), string_t{static_cast<char_t>('\t')});
        }
    }
}


template <typename Derived, editable_text_model TextModel>
void std_edit_controller<Derived, TextModel>::do_insert(bool ctrl, bool shift) {
    if (ctrl || shift) {
        return;
    }

    // changing overwrite mode when user presses insert button
    is_overwrite_mode_ = !is_overwrite_mode_;
}


template <typename Derived, editable_text_model TextModel>
void std_edit_controller<Derived, TextModel>::do_char(char_t c) {

    const auto & cline = line_str(text_mdl_, derived_pos().line);

    // handling overwrite mode
    if (is_overwrite_mode() &&
        selected_range().empty() &&
        derived_pos().column != cline.size()) {

        text_mdl_.replace(derived_pos(), string_t{c});
        auto new_pos = derived_pos();
        ++new_pos.column;
        derived_set_pos_move_anchor(new_pos);
        return;
    }

    // removing indent at } insertion
    if (c == static_cast<char_t>('}') &&
        selected_range().empty() &&
        derived_pos().line != 0 &&
        derived_pos().column == cline.size() &&
        !cline.empty() &&
        cline.find_first_not_of(tab_space_search_str_v<char_t>) == string_t::npos) {

        auto res = find_indent_chars(derived_pos().line - 1);
        const auto & res_chars = std::get<0>(res);
        if (cline.substr(0, res_chars.size()) == res_chars) {
            if (cline.back() == static_cast<char_t>('\t')) {
                delete_chars({{derived_pos().line, derived_pos().column - 1}, derived_pos()});
            } else if (cline.size() >= tab_size() &&
                       cline.substr(cline.size() - 4, 4) == string_t(tab_size(), static_cast<char_t>(' '))) {
                delete_chars({{derived_pos().line, derived_pos().column - tab_size()}, derived_pos()});
            }
        }
    }

    insert_chars(string_t{c});
}


template <typename Derived, editable_text_model TextModel>
bool std_edit_controller<Derived, TextModel>::can_cut() const {
    return !selected_range().empty();
}


template <typename Derived, editable_text_model TextModel>
auto std_edit_controller<Derived, TextModel>::cut() -> std::basic_string<char_t> {
    auto res = this->copy();
    do_delete(false, false);
    return res;
}


template <typename Derived, editable_text_model TextModel>
void std_edit_controller<Derived, TextModel>::paste(const std::basic_string<char_t> & text) {
    insert_chars(text);
}


template <typename Derived, editable_text_model TextModel>
bool std_edit_controller<Derived, TextModel>::can_delete() const {
    return !selected_range().empty();
}


template <typename Derived, editable_text_model TextModel>
void std_edit_controller<Derived, TextModel>::delete_() {
    do_delete(false, false);
}


template <typename Derived, editable_text_model TextModel>
bool std_edit_controller<Derived, TextModel>::can_undo() const {
    return history_.can_undo();
}


template <typename Derived, editable_text_model TextModel>
void std_edit_controller<Derived, TextModel>::undo() {
    // checking that we can do undo action
    if (!history_.can_undo()) {
        return;
    }

    // performing undo action for current undo modification
    perform_undo(history_.current_undo());

    // moving undo modification to redo list
    history_.undo();
}


template <typename Derived, editable_text_model TextModel>
bool std_edit_controller<Derived, TextModel>::can_redo() const {
    return history_.can_redo();
}


template <typename Derived, editable_text_model TextModel>
void std_edit_controller<Derived, TextModel>::redo() {
    // checking that we can do redo action
    if (!history_.can_redo()) {
        return;
    }

    // performing redo action for current redo modification
    perform_redo(history_.current_redo());

    // moving redo modification to undo list
    history_.redo();
}


template <typename Derived, editable_text_model TextModel>
void std_edit_controller<Derived, TextModel>::do_before_save() {
    // removing all characters from current line if it contains only spaces
    remove_all_spaces_current_line();
}


template <typename Derived, editable_text_model TextModel>
auto std_edit_controller<Derived, TextModel>::find_indent_chars(size_t lnum) const ->
std::tuple<std::basic_string<char_t>, size_t> {
    // searching for first line before current position that has
    // non space characters
    while (true) {
        const auto & l_str = line_str(text_mdl_, lnum);
        auto pos = l_str.find_first_not_of(tab_space_search_str_v<char_t>);
        if (pos != string_t::npos) {
            // found line with non space characters
            return std::make_tuple(l_str.substr(0, pos), lnum);
        }

        if (lnum == 0) {
            break;
        }

        --lnum;
    }

    return std::make_tuple(string_t{}, SIZE_MAX);
}


template <typename Derived, editable_text_model TextModel>
void std_edit_controller<Derived, TextModel>::remove_all_spaces_current_line() {
    const auto & cline = line_str(text_mdl_, derived_pos().line);
    
    // checking that line is not empty
    if (cline.empty()) {
        return;
    }

    // checking that line contains only spaces
    for (auto c : cline) {
        if (c != static_cast<char_t>(' ') && c != static_cast<char_t>('\t')) {
            return;
        }
    }
    
    // removing all characters from line
    delete_chars({{derived_pos().line, 0}, {derived_pos().line, cline.size()}});
}


}
