// Copyright (c) 2025, Alexandr Esilevich
//
// Distributed under the Boost Software License.
// See accompanying file LICENSE for license information.
//

/// \file simple_text_model.ipp
/// Contains implementation of the simple_text_model class.

#include "impl/utils.hpp"


namespace tmvc {


template <typename Char>
basic_simple_text_model<Char>::basic_simple_text_model() {
    lines_.push_back(std::make_shared<string_t>());
}


template <typename Char>
basic_simple_text_model<Char>::basic_simple_text_model(const string_t & text) {
    reset(text);
}


template <typename Char>
void basic_simple_text_model<Char>::reset(const string_t & text) {
    auto lines = impl::split_string_to_lines(text);
    lines_.clear();
    std::copy(lines.begin(), lines.end(), std::back_inserter(lines_));
}


template <typename Char>
range basic_simple_text_model<Char>::insert(const position & p, const string_t & chars) {
    if (chars.empty()) {
        return {p, p};
    }

    // creating list of new lines
    auto new_lines = impl::split_string_to_lines(chars);
    assert(!new_lines.empty() && "list of lines is empty");

    // calculating end position of inserted characters
    auto end_line_num = p.line + new_lines.size() - 1;
    auto end_column_num = new_lines.size() == 1 ? p.column + new_lines.back()->size() : new_lines.back()->size();
    position end_pos{end_line_num, end_column_num };
    range ins_range{p, end_pos};

    // sending before inserted signal
    this->before_inserted(ins_range);

    // reserving space in lines vector
    lines_.reserve(lines_.size() + new_lines.size() - 1);

    // saving end of first line and replacing it with added first line
    string_t old_first_line_end = lines_[p.line]->substr(p.column);
    lines_[p.line]->erase(p.column);
    lines_[p.line]->append(*new_lines.front());

    // adding other lines
    lines_.insert(lines_.begin() + p.line + 1,
                  std::next(new_lines.begin()),
                  new_lines.end());


    // merging last inserted line with saved end of old first line
    lines_[end_line_num]->append(old_first_line_end);

    // sending after inserted signals
    this->after_inserted(ins_range);
    this->after_inserted_2(ins_range);

    return ins_range;
}


template <typename Char>
void basic_simple_text_model<Char>::erase(const range & r) {
    assert(pos_is_valid(*this, r.start) && "start position should be valid");
    assert(pos_is_valid(*this, r.end) && "end position should be valid");

    if (r.start == r.end) {
        // empty range
        return;
    }

    // sending before removed signal
    this->before_erased(r);

    if (r.start.line == r.end.line) {
        // all range is in single line
        lines_[r.start.line]->erase(r.start.column, r.end.column - r.start.column);
    } else {

        // removing content from first line of range and cocatenating
        // rest of content from last line of range
        lines_[r.start.line]->erase(r.start.column);
        lines_[r.start.line]->append(lines_[r.end.line]->begin() + r.end.column,
                                     lines_[r.end.line]->end());

        // removing lines after the first line
        lines_.erase(lines_.begin() + r.start.line + 1, lines_.begin() + r.end.line + 1);
    }

    // sending after removed signal
    this->after_erased(r);
    this->after_erased_2(r);
}


template <typename Char>
void basic_simple_text_model<Char>::replace(const position & p, const string_t & chars) {
    assert(p.line < lines_size() && "invalid line index for replace");

    auto sz = line_size(p.line);

    assert(chars.size() <= sz && "invalid size of replaced characters");
    assert(sz - chars.size() >= p.column && "invalid size of replaced characters");

    range replace_range{p, {p.line, p.column + chars.size()}};
    this->before_replaced(replace_range);
    lines_[p.line]->replace(p.column, chars.size(), chars);
    this->after_replaced(replace_range);
}


}
