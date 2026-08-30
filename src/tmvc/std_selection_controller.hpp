// Copyright (c) 2025, Alexandr Esilevich
//
// Distributed under the Boost Software License.
// See accompanying file LICENSE for license information.
//

/// \file std_selection_controller.hpp
/// Contains definition of the std_selection_controller template class.

#pragma once

#include "context_menu.hpp"
#include "selection_controller.hpp"
#include "selection_model.hpp"
#include "std_mouse_controller.hpp"
#include "std_paging_controller.hpp"
#include "text_model.hpp"
#include "impl/utils.hpp"
#include <algorithm>
#include <optional>


namespace tmvc {


/// Concept that all classes derived from the std_selection_controller class should confirm.
template <typename Derived>
concept std_selection_controller_derived = requires(Derived & derived, const Derived & cderived) {
    // Returns anchor position from selection model
    { cderived.anchor_pos() } -> std::convertible_to<position>;

    // Returns current position from selection model
    { cderived.pos() } -> std::convertible_to<position>;

    // Implementation of selecting text
    derived.select_text_impl(std::declval<position>(), std::declval<position>());
};


/// Base helper class for implementing controllers. Implements standard basic text navigation
/// logic. Uses CRTP pattern to implement non-virtual dynamic dispatch. The derived class
/// must confirm to the std_selection_controller_derived concept.
template <typename Derived, text_model TextModel>
class std_selection_controller: public std_mouse_controller<Derived>,
                                public std_paging_controller<Derived> {
public:
    /// Type of text model
    using text_model_t = TextModel;

    /// Type of character
    using char_t = typename TextModel::char_t;

    /// Type of underlying character code
    using char_value_t = impl::char_value_t<char_t>;

    /// Constructs controller with specified references to text model and selection model
    std_selection_controller(const TextModel & text_mdl):
    text_mdl_{text_mdl} {
        // listening for text model changes

        after_inserted_con_ = text_mdl.after_inserted.connect([this](auto && ...) {
            on_text_changed();
        });

        after_erased_con_ = text_mdl.after_erased.connect([this](auto && ...) {
            on_text_changed();
        });

        after_replaced_con_ = text_mdl.after_replaced.connect([this](auto && ...) {
            on_text_changed();
        });
    }

    ////////////////////////////////////////////////////////////
    // Models

    /// Returns const reference to text model
    const auto & text() const { return text_mdl_; }


    ////////////////////////////////////////////////////////////
    // Text navigation

    /// Sets position of cursor with moving anchor
    void set_pos_move_anchor(const position & p) {
        select_text(p, p);
    }



    /// Sets position of cursor in controller without moving selection anchor
    void set_pos_keep_anchor(const position & p) {
        select_text(derived_anchor_pos(), p);
    }


    /// Selects text
    void select_text(const position & anchor_p, const position & p, bool reset_up_down = true) {
        derived_select_text_impl(anchor_p, p);

        if (reset_up_down) {
            reset_up_down_saved_column();
        }
    }


    /// Moves position to next word
    void move_next_word(bool keep_anchor) {
        auto pos = derived_pos();

        // checking for end of line
        if (pos.column == text_mdl_.line_size(pos.line)) {
            // moving to next line
            if (pos.line != text_mdl_.lines_size() - 1) {
                position new_pos = {pos.line + 1, 0};
                if (keep_anchor) {
                    set_pos_keep_anchor(new_pos);
                } else {
                    set_pos_move_anchor(new_pos);
                }
            }
        } else {
            // calculating new column
            auto pos_range = positions(text_mdl_, {pos, {pos.line, text_mdl_.line_size(pos.line)}});
            auto col = calc_next_word_pos_in_line(text_mdl_, pos_range, true);
            auto new_col = col + pos.column;
            assert(new_col <= text_mdl_.line_size(pos.line) && "invalid new position");
            position new_pos = {pos.line, new_col};
            if (keep_anchor) {
                set_pos_keep_anchor(new_pos);
            } else {
                set_pos_move_anchor(new_pos);
            }
        }
    }


    /// Moves position to previous word
    void move_prev_word(bool keep_anchor) {
        auto pos = derived_pos();

        // checking for end of line
        if (pos.column == 0) {
            // moving to previous line
            if (derived_pos().line != 0) {
                auto new_line = pos.line - 1;
                position new_pos = {new_line, text_mdl_.line_size(new_line)};
                if (keep_anchor) {
                    set_pos_keep_anchor(new_pos);
                } else {
                    set_pos_move_anchor(new_pos);
                }
            }
        } else {
            // calculating new column
            auto pos_range = positions(text_mdl_, {{pos.line, 0}, pos}) | std::ranges::views::reverse;
            auto offs = calc_next_word_pos_in_line(text_mdl_, pos_range, false);
            
            assert(offs <= pos.column && "invalid new position");
            auto new_col = pos.column - offs;
            
            position new_pos = {pos.line, new_col};
            if (keep_anchor) {
                set_pos_keep_anchor(new_pos);
            } else {
                set_pos_move_anchor(new_pos);
            }
        }
    }


    ////////////////////////////////////////////////////////////
    // Key user actions

    /// Performs actions when user presses left button
    void do_left(bool ctrl,
                 bool shift,
                 bool alt,
                 const std::optional<position> & suggested_pos = std::nullopt) {
        if (suggested_pos) {
            if (shift) {
                set_pos_keep_anchor(*suggested_pos);
            } else {
                set_pos_move_anchor(*suggested_pos);
            }
            return;
        }

        bool move_word = ctrl;
#if defined(__APPLE__)
        move_word = alt;
#endif
        if (move_word) {
            move_prev_word(shift);
            return;
        }

        auto new_pos = get_pos_backward(derived_pos());
        if (shift) {
            set_pos_keep_anchor(new_pos);
        } else {
            set_pos_move_anchor(new_pos);
        }
    }

    /// Performs actions when user presses right button
    void do_right(bool ctrl,
                  bool shift,
                  bool alt,
                  const std::optional<position> & suggested_pos = std::nullopt) {
        if (suggested_pos) {
            if (shift) {
                set_pos_keep_anchor(*suggested_pos);
            } else {
                set_pos_move_anchor(*suggested_pos);
            }
            return;
        }

        bool move_word = ctrl;
#if defined(__APPLE__)
        move_word = alt;
#endif
        if (move_word) {
            move_next_word(shift);
            return;
        }

        auto new_pos = get_pos_forward(derived_pos());
        if (shift) {
            set_pos_keep_anchor(new_pos);
        } else {
            set_pos_move_anchor(new_pos);
        }
    }

    /// Performs actions when user presses up button
    void do_up(bool,
               bool shift,
               bool,
               const std::optional<position> & suggested_pos = std::nullopt) {
        if (suggested_pos) {
            auto new_pos = *suggested_pos;
            if (new_pos.line >= text_mdl_.lines_size()) {
                new_pos.line = text_mdl_.lines_size() - 1;
            }
            auto line_sz = text_mdl_.line_size(new_pos.line);
            if (new_pos.column > line_sz) {
                new_pos.column = line_sz;
            }

            if (shift) {
                select_text(derived_anchor_pos(), new_pos, false);
            } else {
                select_text(new_pos, new_pos, false);
            }
            return;
        }

        if (derived_pos().line == 0) {
            // first line
            return;
        }

        // saving original column number before start up/down moves
        if (up_down_saved_column() == 0) {
            set_up_down_saved_column(derived_pos().column);
        }

        auto new_pos = derived_pos();
        --new_pos.line;

        // restoring original column that current position had before
        // starting up/down actions
        if (up_down_saved_column() != 0) {
            new_pos.column = up_down_saved_column();
        }

        if (text_mdl_.line_size(new_pos.line) < new_pos.column) {
            new_pos.column = text_mdl_.line_size(new_pos.line);
        }

        // setting new position without resetting up/down column
        if (shift) {
            select_text(derived_anchor_pos(), new_pos, false);
        } else {
            select_text(new_pos, new_pos, false);
        }
    }

    /// Performs actions when user presses down button
    void do_down(bool,
                 bool shift,
                 bool,
                 const std::optional<position> & suggested_pos = std::nullopt) {
        if (suggested_pos) {
            auto new_pos = *suggested_pos;
            if (new_pos.line >= text_mdl_.lines_size()) {
                new_pos.line = text_mdl_.lines_size() - 1;
            }
            auto line_sz = text_mdl_.line_size(new_pos.line);
            if (new_pos.column > line_sz) {
                new_pos.column = line_sz;
            }

            if (shift) {
                select_text(derived_anchor_pos(), new_pos, false);
            } else {
                select_text(new_pos, new_pos, false);
            }
            return;
        }

        if (derived_pos().line == text_mdl_.lines_size() - 1) {
            // last line
            return;
        }

        // saving original column number before start up/down moves
        if (up_down_saved_column() == 0) {
            set_up_down_saved_column(derived_pos().column);
        }

        auto new_pos = derived_pos();
        ++new_pos.line;

        // restoring original column that current position had before
        // starting up/down actions
        if (up_down_saved_column() != 0) {
            new_pos.column = up_down_saved_column();
        }

        if (text_mdl_.line_size(new_pos.line) < new_pos.column) {
            new_pos.column = text_mdl_.line_size(new_pos.line);
        }

        // setting new position without resetting up/down column
        if (shift) {
            select_text(derived_anchor_pos(), new_pos, false);
        } else {
            select_text(new_pos, new_pos, false);
        }
    }

    /// Performs actions when user presses home button
    void do_home(bool ctrl, bool shift, bool alt) {
        move_line_start(shift, std::nullopt);
    }

    /// Performs actions when user presses end button
    void do_end(bool ctrl, bool shift, bool alt) {
        move_line_end(shift, std::nullopt);
    }

    /// Moves cursor to start of line, using target_pos for visual wrap mode
    void move_line_start(bool keep_anchor, const std::optional<position> & target_pos) {
        if (target_pos) {
            if (keep_anchor) {
                set_pos_keep_anchor(*target_pos);
            } else {
                set_pos_move_anchor(*target_pos);
            }

            return;
        }

        position new_pos{derived_pos().line, 0};
        if (keep_anchor) {
            set_pos_keep_anchor(new_pos);
        } else {
            set_pos_move_anchor(new_pos);
        }
    }

    /// Moves cursor to end of line, using target_pos for visual wrap mode
    void move_line_end(bool keep_anchor, const std::optional<position> & target_pos) {
        if (target_pos) {
            if (keep_anchor) {
                set_pos_keep_anchor(*target_pos);
            } else {
                set_pos_move_anchor(*target_pos);
            }

            return;
        }

        position new_pos{derived_pos().line, text_mdl_.line_size(derived_pos().line)};
        if (keep_anchor) {
            set_pos_keep_anchor(new_pos);
        } else {
            set_pos_move_anchor(new_pos);
        }
    }

    /// Moves cursor to previous line
    void move_prev_line(bool keep_anchor, const std::optional<position> & target_pos) {
        do_up(false, keep_anchor, false, target_pos);
    }

    /// Moves cursor to next line
    void move_next_line(bool keep_anchor, const std::optional<position> & target_pos) {
        do_down(false, keep_anchor, false, target_pos);
    }

    /// Moves cursor to start of block
    void move_block_start(bool keep_anchor) {
        move_line_start(keep_anchor, std::nullopt);
    }

    /// Moves cursor to end of block
    void move_block_end(bool keep_anchor) {
        move_line_end(keep_anchor, std::nullopt);
    }

    ////////////////////////////////////////////////////////////
    // Paging

    // Moving position to specified number of lines up
    void move_lines_up(uint64_t n_lines, bool kee_anchor) {
        // calculating new selection position
        auto new_line_idx = derived_pos().line;
        if (new_line_idx < n_lines) {
            new_line_idx = 0;
        } else {
            new_line_idx -= n_lines;
        }

        auto new_column_idx = derived_pos().column;
        auto line_sz = text_mdl_.line_size(new_line_idx);
        if (new_column_idx > line_sz) {
            new_column_idx = line_sz;
        }

        // setting new selection position
        position new_pos{new_line_idx, new_column_idx};
        if (kee_anchor) {
            set_pos_keep_anchor(new_pos);
        } else {
            set_pos_move_anchor(new_pos);
        }
    }


    // Moving position to specified number of lines down
    void move_lines_down(uint64_t n_lines, bool keep_anchor) {
        // calculating new text position
        auto new_line_idx = derived_pos().line;
        if (text_mdl_.lines_size() < n_lines ||
            text_mdl_.lines_size() - n_lines <= new_line_idx) {

            new_line_idx = text_mdl_.lines_size() - 1;
        } else {
            new_line_idx += n_lines;
        }

        auto new_column_idx = derived_pos().column;
        auto line_sz = text_mdl_.line_size(new_line_idx);
        if (new_column_idx > line_sz) {
            new_column_idx = line_sz;
        }

        // setting new text position
        position new_pos{new_line_idx, new_column_idx};
        if (keep_anchor) {
            set_pos_keep_anchor(new_pos);
        } else {
            set_pos_move_anchor(new_pos);
        }
    }


    ////////////////////////////////////////////////////////////
    // Predefined user actions

    /// Selects all text
    void select_all() {
        set_pos_move_anchor(begin_pos(text_mdl_));
        set_pos_keep_anchor(end_pos(text_mdl_));
    }


    ////////////////////////////////////////////////////////////
    // Predefined user actions

    /// Returns true if copy action can be performed now
    bool can_copy() const {
        return !impl::selected_range(derived_anchor_pos(), derived_pos()).empty();
    }

    /// Performs copy action. Returns vector of characters that should be placed in clipboard
    std::vector<char_t> copy() const {
        auto chars = characters(text_mdl_,
                                impl::selected_range(derived_anchor_pos(), derived_pos()));
        auto chars_common = chars | std::ranges::views::common;
        return std::vector<char_t>{chars_common.begin(), chars_common.end()};
    }


    ////////////////////////////////////////////////////////////
    // Context menu

    /// Creates context menu
    basic_context_menu<char_t> create_context_menu() {
        basic_context_menu<char_t> menu;
        menu.add_action(standard_context_menu_action_type::copy);
        menu.add_separator();
        menu.add_action(standard_context_menu_action_type::select_all);
        return menu;
    }

    /// Selects token at specified text position
    void select_word(const position & pos,
                     bool,
                     bool) {
        auto line_sz = text_mdl_.line_size(pos.line);
        if (pos.column >= line_sz) {
            set_pos_move_anchor(pos);
            return;
        }

        auto is_word = is_word_char_at(pos);
        auto is_space = char_is_space_at(text_mdl_, pos);

        auto same_kind = [this, is_word, is_space](const position & p) {
            if (is_word) {
                return is_word_char_at(p);
            }

            if (is_space) {
                return char_is_space_at(text_mdl_, p);
            }

            return !is_word_char_at(p) && !char_is_space_at(text_mdl_, p);
        };

        auto start = pos;
        auto end = pos;

        while (start.column > 0) {
            position prev{start.line, start.column - 1};
            if (!same_kind(prev)) {
                break;
            }
            --start.column;
        }

        while (end.column < line_sz && same_kind(end)) {
            ++end.column;
        }

        set_pos_move_anchor(start);
        set_pos_keep_anchor(end);
    }


    ////////////////////////////////////////////////////////////
    // Text search

    /// Searches for specified text in text model and returns ranges of all found matches
    const std::vector<range> & find(const std::vector<char_t> & find_chars,
                                    const find_options & options) {
        find_chars_ = find_chars;
        find_options_ = options;
        update_find_results();
        return find_results_;
    }


    /// Returns ranges of matches found by the last search
    const std::vector<range> & find_results() const { return find_results_; }


    /// Selects next find match after current selection
    void find_next() {
        if (find_results_.empty()) {
            return;
        }

        // searching for next find match after current selection
        auto sel = impl::selected_range(derived_anchor_pos(), derived_pos());
        auto it = std::find_if(find_results_.begin(), find_results_.end(), [&sel](auto && r) {
            return r.start >= sel.end;
        });

        // continuing from the first find match when there are no matches after selection
        if (it == find_results_.end()) {
            it = find_results_.begin();
        }

        select_text(it->start, it->end);
    }


    /// Selects previous find match before current selection
    void find_prev() {
        if (find_results_.empty()) {
            return;
        }

        // searching for previous find match before current selection
        auto sel = impl::selected_range(derived_anchor_pos(), derived_pos());
        auto it = std::find_if(find_results_.rbegin(), find_results_.rend(), [&sel](auto && r) {
            return r.end <= sel.start;
        });

        // continuing from the last find match when there are no matches before selection
        if (it == find_results_.rend()) {
            it = find_results_.rbegin();
        }

        select_text(it->start, it->end);
    }


protected:
    /// Resets saved number of column when moving up/down
    void reset_up_down_saved_column() {
        up_down_saved_column_ = 0;
    }


    /// Searches for the text of the last search and updates find results
    void update_find_results() {
        find_results_.clear();

        if (find_chars_.empty()) {
            return;
        }

        // searching for matches starting from all positions in text
        auto text_end = end_pos(text_mdl_);
        auto pos = begin_pos(text_mdl_);
        while (pos != text_end) {
            auto match_end = match_at(pos, find_chars_, find_options_);
            if (match_end) {
                // continuing to search after found match
                find_results_.push_back({pos, *match_end});
                pos = *match_end;
            } else {
                pos = next_pos(text_mdl_, pos);
            }
        }
    }




private:
    /// Returns current position. Uses CTPR pattern to call method of derived class.
    position derived_pos() const {
        static_assert(std_selection_controller_derived<Derived>,
                      "Derived class must confirm to the std_selection_controller_derived concept");
        return static_cast<const Derived*>(this)->pos();
    }

    /// Returns selection anchor position. Uses CTPR pattern to call method of derived class.
    position derived_anchor_pos() const {
        static_assert(std_selection_controller_derived<Derived>,
                      "Derived class must confirm to the std_selection_controller_derived concept");
        return static_cast<const Derived*>(this)->anchor_pos();
    }

    /// Sets current position and anchor. Uses CTPR pattern to call method of derived class.
    void derived_select_text_impl(const position & anchor_p, const position & p) {
        static_assert(std_selection_controller_derived<Derived>,
                      "Derived class must confirm to the std_selection_controller_derived concept");
        static_cast<Derived*>(this)->select_text_impl(anchor_p, p);
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

    /// Returns saved number of column when moving up/down
    size_t up_down_saved_column() { return up_down_saved_column_; }

    /// Sets up/down saved column to specified value
    void set_up_down_saved_column(size_t c) { up_down_saved_column_ = c; }


    /// Calculates position (offset from start of range) of next word in single line.
    /// is_next is true if performing calculation for stepping to next word. Else
    /// performing calculation for stepping to prev word.
    template <typename PositionsRange>
    size_t calc_next_word_pos_in_line(const TextModel & mdl,
                                      const PositionsRange & range,
                                      bool is_next) {
        auto it = std::ranges::begin(range);
        auto end = std::ranges::end(range);

        // if current symbol is space then searching for next non-space character
        if (char_is_space_at(mdl, *it)) {
            while (it != end) {
                if (!char_is_space_at(mdl, *it)) {
                    break;
                }

                ++it;
            }

            if (is_next) {
                // we reached next word
                return std::ranges::distance(std::ranges::begin(range), it);
            } else {
                // if we stepping to previous word then we should skip spaces and then
                // skip word after spaces, so continue calculation
            }
        }

        // if current symbol is part of word then skipping all such symbols
        if (is_word_char_at(*it)) {
            while (it != end) {
                if (!is_word_char_at(*it)) {
                    break;
                }

                ++it;
            }
        } else {
            // skipping all characters that are not part of word and not spaces
            while (it != end) {
                if (is_word_char_at(*it) || char_is_space_at(mdl, *it)) {
                    break;
                }

                ++it;
            }
        }

        // skipping spaces after word if is_next is true
        if (is_next) {
            while (it != end) {
                if (!char_is_space_at(mdl, *it)) {
                    break;
                }
        
                ++it;
            }
        }

        return std::ranges::distance(std::ranges::begin(range), it);
    }


    /// Called after text in model changed
    void on_text_changed() {
        reset_up_down_saved_column();
        update_find_results();
    }


    /// Returns end position of match starting at specified position, or empty
    /// value if text at specified position doesn't match
    std::optional<position> match_at(const position & pos,
                                     const std::vector<char_t> & find_chars,
                                     const find_options & options) const {
        auto text_end = end_pos(text_mdl_);
        auto cur = pos;

        for (auto && ch : find_chars) {
            if (cur == text_end || !chars_equal(char_at_pos(cur), ch, options.match_mode)) {
                return std::nullopt;
            }

            cur = next_pos(text_mdl_, cur);
        }

        // checking that match is not surrounded by word characters
        if (options.match_word) {
            if (pos != begin_pos(text_mdl_) && is_word_char_at(prev_pos(text_mdl_, pos))) {
                return std::nullopt;
            }

            if (cur != text_end && is_word_char_at(cur)) {
                return std::nullopt;
            }
        }

        return cur;
    }


    /// Returns character at specified position. Returns newline character
    /// for position at the end of line.
    char_t char_at_pos(const position & pos) const {
        if (pos.column == text_mdl_.line_size(pos.line)) {
            return impl::make_char<char_t>('\n');
        }

        return text_mdl_.char_at(pos);
    }


    /// Returns true if character at specified position can be part of word
    bool is_word_char_at(const position & pos) const {
        // end of line is never part of word
        if (pos.column == text_mdl_.line_size(pos.line)) {
            return false;
        }

        return char_is_alnum_at(text_mdl_, pos) ||
               char_value(text_mdl_.char_at(pos)) == static_cast<char_value_t>('_');
    }


    /// Returns true if characters are equal in specified character match mode
    static bool chars_equal(const char_t & ch1, const char_t & ch2, char_match_mode mode) {
        switch (mode) {
        case char_match_mode::whole_char:
            return ch1 == ch2;

        case char_match_mode::char_value:
            return char_value(ch1) == char_value(ch2);

        case char_match_mode::char_value_ignore_case:
            return char_to_lower(char_value(ch1)) == char_to_lower(char_value(ch2));
        }

        assert(false && "invalid character match mode");
        return false;
    }


    /// Returns code of specified character
    static char_value_t char_value(const char_t & ch) {
        if constexpr (std_character<char_t>) {
            return ch;
        } else {
            return ch.character();
        }
    }


    /// Returns lower case for specified character code
    static char_value_t char_to_lower(char_value_t code) {
        if constexpr (std::is_same_v<char_value_t, wchar_t>) {
            return static_cast<char_value_t>(std::towlower(code));
        } else if constexpr (std::is_same_v<char_value_t, char>) {
            return static_cast<char_value_t>(std::tolower(static_cast<unsigned char>(code)));
        } else {
            // for now, cast all other character types to char. This should
            // work in most cases even for multibyte unicode string.
            return static_cast<char_value_t>(
                std::tolower(static_cast<unsigned char>(static_cast<char>(code))));
        }
    }


    const TextModel & text_mdl_;    ///< Reference to text model

    /// Saved original column number that current position had before starting
    /// move up or down in document
    size_t up_down_saved_column_ = 0;

    std::vector<char_t> find_chars_;     ///< Text of the last search
    find_options find_options_;          ///< Options of the last search

    /// Ranges of matches found by the last search
    std::vector<range> find_results_;

    // connections to model signals
    scoped_signal_connection after_inserted_con_;
    scoped_signal_connection after_erased_con_;
    scoped_signal_connection after_replaced_con_;
};


}
