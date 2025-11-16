// Copyright (c) 2025, Alexandr Esilevich
//
// Distributed under the Boost Software License.
// See accompanying file LICENSE for license information.
//

/// \file line_numbers_text_model.hpp
/// Contains definition of the line_numbers_text_model class.

#pragma once

#include "line_numbers_model.hpp"
#include "text_model.hpp"


namespace tmvc {


/// Text model that contains line numbers for specified text model
/// and line numbers model
template <text_model TextModel, line_numbers_model LineNumbersModel>
class line_numbers_text_model: public text_model_signals {
public:
    /// Type of character
    using char_t = TextModel::char_t;

    /// Type of string
    using string_t = std::basic_string<char_t>;


    /// Constructs model with specified references to base text model and
    /// line numbers model
    line_numbers_text_model(const TextModel & txt, const LineNumbersModel & lnums):
    text_{txt}, line_numbers_{lnums}, max_line_size_{line_numbers_.max_size()} {
        before_inserted_con_ = text_.before_inserted.connect([this](auto && r) {
            on_before_inserted(r);
        });

        after_inserted_con_ = text_.after_inserted.connect([this](auto && r) {
            on_after_inserted(r);
        });

        before_erased_con_ = text_.before_erased.connect([this](auto && r) {
            on_before_erased(r);
        });

        after_erased_con_ = text_.after_erased.connect([this](auto && r) {
            on_after_erased(r);
        });
    }

    /// Returns number of lines in the model
    uint64_t lines_size() const {
        return text_.lines_size();
    }

    /// Returns length of line at specified index
    uint64_t line_size(uint64_t idx) const {
        return line_str_size();
    }

    /// Returns character at specified position
    char_t char_at(const position & pos) const {
        assert(pos.line < lines_size() && "invalid line number");
        assert(pos.column < line_str_size() && "invalid column number");

        // calculating number of first space characters added by this model
        auto n_first_spaces = line_str_size() - line_numbers_.max_size();

        if (pos.column < n_first_spaces) {
            // first space characters added by this model
            return static_cast<char_t>(' ');
        }

        // reading line number from line numbers model
        auto orig_line = calc_original_line_number(pos.line);
        auto str = line_numbers_.line_number(orig_line);

        // calculating number of leading spaces for this line number
        assert(str.size() <= line_numbers_.max_size() && "invalid line number");
        auto num_spaces = line_numbers_.max_size() - str.size();

        if (pos.column - n_first_spaces < num_spaces) {
            return static_cast<char_t>(' ');
        }

        auto idx = pos.column - n_first_spaces - num_spaces;
        assert(idx <= str.size() && "invalid column number");
        return str[idx];
    }

    /// Returns number of characters in each line
    uint64_t line_str_size() const {
        return line_numbers_.max_size();
    }

    /// Returns max number of characters in line
    uint64_t max_line_size() const {
        return line_numbers_.max_size();
    }

    /// The signal is emitted when max number of characters in line is changed
    mutable signal<void ()> max_line_size_changed;


private:
    /// Calculates original model line number taking into account
    /// current insert/erase operations being in progress
    uint64_t calc_original_line_number(uint64_t idx) const {
        if (curr_insert_erase_lines_ != std::tuple<uint64_t, uint64_t>{0, 0}) {
            auto first = std::get<0>(curr_insert_erase_lines_);
            auto last = std::get<1>(curr_insert_erase_lines_);
            if (idx > last) {
                return idx - (last - first);
            }
        }

        return idx;
    }

    /// Called before characters inserted in the original model
    void on_before_inserted(const range & r) {
        assert(r != empty_range && "empty insert range");

        // checking if new lines were inserted
        if (r.start.line == r.end.line) {
            return;
        }

        // emulating start of characters insertion in this text model
        before_inserted(calc_insert_erase_range(r));
    }

    /// Called after characters inserted in the original model
    void on_after_inserted(const range & r) {
        assert(r != empty_range && "empty insert range");

        // checking if new lines were inserted
        if (r.start.line == r.end.line) {
            return;
        }

        if (r.end.line == lines_size() - 1) {
            // no replace emulation is required
            after_inserted(calc_insert_erase_range(r));
            after_inserted_2(calc_insert_erase_range(r));
            return;
        }

        // saving current insert line range to emulate old line numbers after the inserted range
        curr_insert_erase_lines_ = {r.start.line, r.end.line};

        // emulating end of characters insertion in this text model
        after_inserted(calc_insert_erase_range(r));
        after_inserted_2(calc_insert_erase_range(r));

        // calculating new max line size
        auto new_max_line_size = max_line_size();
        if (new_max_line_size != max_line_size_) {
            max_line_size_ = new_max_line_size;
            max_line_size_changed();
        }

        // emulating start of characters replacing in this text model
        position replace_start = {r.end.line + 1, 0};
        position replace_end = {lines_size() - 1, line_str_size()};
        range replace_range{replace_start, replace_end};
        before_replaced(replace_range);

        // resetting current insert range
        curr_insert_erase_lines_ = {0, 0};

        // emulating end of characters replacing in this text model
        after_replaced(replace_range);
    }

    /// Called before characters erased in the original model
    void on_before_erased(const range & r) {
        assert(r != empty_range && "empty erase range");

        // checking if some lines were erased
        if (r.start.line == r.end.line) {
            return;
        }

        if (r.end.line == lines_size() - 1) {
            // no replace emulation is required
            before_erased(calc_insert_erase_range(r));
            return;
        }

        // emulating start of characters replacing in this text model
        position replace_start = {r.end.line + 1, 0};
        position replace_end = {lines_size() - 1, line_str_size()};
        range replace_range{replace_start, replace_end};
        before_replaced(replace_range);

        // saving current erase line range to emulate old line numbers after the erased range
        curr_insert_erase_lines_ = {r.start.line, r.end.line};

        // emulating end of characters replacing in this text model
        after_replaced(replace_range);

        // emulating start of characters erasing in this text model
        before_erased(calc_insert_erase_range(r));
    }

    /// Called after characters erased in the original model
    void on_after_erased(const range & r) {
        assert(r != empty_range && "empty erase range");

        // checking if some lines were erased
        if (r.start.line == r.end.line) {
            return;
        }
        
        // resetting current erase range
        curr_insert_erase_lines_ = {0, 0};

        // emulating end of characters erasing in this text model
        after_erased(calc_insert_erase_range(r));
        after_erased_2(calc_insert_erase_range(r));

        // calculating new max line size
        auto new_max_line_size = max_line_size();
        if (new_max_line_size != max_line_size_) {
            max_line_size_ = new_max_line_size;
            max_line_size_changed();
        }
    }

    /// Calculates insert or erase range for line numbers text model
    range calc_insert_erase_range(const range & r) {
        assert(r.start.line < r.end.line && "invalid insert range");

        position start = {r.start.line, line_str_size()};
        position end = {r.end.line, line_str_size()};

        return {start, end};
    }


    const TextModel & text_;                ///< Reference to the original text model
    const LineNumbersModel & line_numbers_; ///< Reference to line numbers model

    static constexpr range empty_range = {{0, 0}, {0, 0}};

    /// Current line range being inserted into or erased from the original model
    std::tuple<uint64_t, uint64_t> curr_insert_erase_lines_ = {0, 0};

    size_t max_line_size_;                  ///< Current maximum line size

    scoped_signal_connection before_inserted_con_;
    scoped_signal_connection after_inserted_con_;
    scoped_signal_connection before_erased_con_;
    scoped_signal_connection after_erased_con_;
};


}