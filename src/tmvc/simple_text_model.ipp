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
}


template <typename Char>
basic_simple_text_model<Char>::basic_simple_text_model(characters_range<Char> auto && chars) {
    reset(chars);
}


template <typename Char>
void basic_simple_text_model<Char>::reset(characters_range<Char> auto && chars) {
    data_.reset(chars);
}


template <typename Char>
range basic_simple_text_model<Char>::insert(const position & p,
                                            characters_range<Char> auto && chars) {
    if (std::ranges::empty(chars)) {
        return {p, p};
    }

    // inserting characters into text data
    auto ins_range = data_.insert(p, chars, [this](auto && ins_range) {
        // sending before inserted signal
        this->before_inserted(ins_range);
    });

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

    // erasing characters from text data
    data_.erase(r);

    // sending after erased signal
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

    data_.replace(p, chars);

    this->after_replaced(replace_range);
}


}
