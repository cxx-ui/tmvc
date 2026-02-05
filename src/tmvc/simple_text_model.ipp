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
    lines_.push_back(std::make_shared<line_t>());
}


template <typename Char>
basic_simple_text_model<Char>::basic_simple_text_model(characters_range<Char> auto && chars) {
    reset(chars);
}


template <typename Char>
void basic_simple_text_model<Char>::reset(characters_range<Char> auto && chars) {
    auto lines = impl::split_chars_to_lines<Char>(chars);
    lines_.clear();
    std::copy(lines.begin(), lines.end(), std::back_inserter(lines_));
}


template <typename Char>
range basic_simple_text_model<Char>::insert(const position & p,
                                            characters_range<Char> auto && chars) {
    if (std::ranges::empty(chars)) {
        return {p, p};
    }

    // creating list of new lines
    auto new_lines = impl::split_chars_to_lines<Char>(chars);
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
    line_t old_first_line_end{lines_[p.line]->begin() + p.column, lines_[p.line]->end()};
    lines_[p.line]->erase(lines_[p.line]->begin() + p.column, lines_[p.line]->end());
    lines_[p.line]->insert(lines_[p.line]->end(),
                           new_lines.front()->begin(),
                           new_lines.front()->end());

    // adding other lines
    lines_.insert(lines_.begin() + p.line + 1,
                  std::next(new_lines.begin()),
                  new_lines.end());


    // merging last inserted line with saved end of old first line
    lines_[end_line_num]->insert(lines_[end_line_num]->end(),
                                 old_first_line_end.begin(),
                                 old_first_line_end.end());

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
        auto first = lines_[r.start.line]->begin() + r.start.column;
        auto last = lines_[r.start.line]->begin() + r.end.column;
        lines_[r.start.line]->erase(first, last);
    } else {

        // removing content from first line of range and concatenating
        // rest of content from last line of range
        auto first = lines_[r.start.line]->begin() + r.start.column;
        lines_[r.start.line]->erase(first, lines_[r.start.line]->end());
        lines_[r.start.line]->insert(lines_[r.start.line]->end(),
                                     lines_[r.end.line]->begin() + r.end.column,
                                     lines_[r.end.line]->end());

        // removing lines after the first line
        lines_.erase(lines_.begin() + r.start.line + 1, lines_.begin() + r.end.line + 1);
    }

    // sending after removed signal
    this->after_erased(r);
    this->after_erased_2(r);
}


template <typename Char>
void basic_simple_text_model<Char>::replace(const position & p,
                                            characters_range<Char> auto && chars) {
    assert(p.line < lines_size() && "invalid line index for replace");

    auto sz = line_size(p.line);
    auto chars_sz = std::ranges::size(chars);

    assert(chars_sz <= sz && "invalid size of replaced characters");
    assert(sz - chars_sz >= p.column && "invalid size of replaced characters");

    range replace_range{p, {p.line, p.column + chars_sz}};
    this->before_replaced(replace_range);

    auto first = lines_[p.line]->begin() + p.column;
    auto last = lines_[p.line]->begin() + p.column + chars_sz;
    lines_[p.line]->erase(first, last);
    lines_[p.line]->insert(lines_[p.line]->begin() + p.column,
                           std::ranges::begin(chars),
                           std::ranges::end(chars));

    this->after_replaced(replace_range);
}


}
