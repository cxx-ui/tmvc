// Copyright (c) 2025, Alexandr Esilevich
//
// Distributed under the Boost Software License.
// See accompanying file LICENSE for license information.
//

/// \file text_model.hpp
/// Contains definition of text model concepts and related functions.

#pragma once

#include "position.hpp"
#include "range.hpp"
#include "signals.hpp"
#include "std_character.hpp"
#include "text_model_signals.hpp"
#include <cctype>
#include <cwctype>
#include <iterator>
#include <ranges>
#include <string>


namespace tmvc {


/// Represents text model character (standard character or type with character() method)
template <typename Char>
concept text_model_character = std_character<Char> ||
requires(const Char & c) {
    { c.character() } -> std_character;
    { c == std::declval<char>() } -> std::convertible_to<bool>;
    { std::declval<char>() == c } -> std::convertible_to<bool>;
};


/// Type of range signals used in text model
using text_model_range_signal = signal<void (const range &)>;


/// Represents read only text model
template <typename TextModel>
concept text_model = requires(const TextModel & mdl) {
    /// Character type
    typename TextModel::char_t;
    requires text_model_character<typename TextModel::char_t>;

    // Returns number of lines in model
    { mdl.lines_size() } -> std::convertible_to<uint64_t>;

    // Returns length of line at specified index
    { mdl.line_size(std::declval<uint64_t>()) } -> std::convertible_to<uint64_t>;

    /// Returns character at specified position
    { mdl.char_at(std::declval<position>()) } -> std::convertible_to<typename TextModel::char_t>;


    //////////////////////////////////////////////////
    // Signals

    // The signal is emitted before characters are inserted in range
    { mdl.before_inserted } -> std::convertible_to<text_model_range_signal &>;

    // The signal is emitted after characters are inserted in range
    { mdl.after_inserted } -> std::convertible_to<text_model_range_signal &>;

    /// Additional signal that is emitted after the after_inserted signal. It is used
    /// for implementing advanced logic when users need to execute some actions
    /// after all clients processed the after_inserted signal.
    { mdl.after_inserted_2 } -> std::convertible_to<text_model_range_signal &>;

    /// The signal is emitted before characters are removed in specified range
    { mdl.before_erased } -> std::convertible_to<text_model_range_signal &>;

    /// The signal is emitted after characters are removed in specified range
    { mdl.after_erased } -> std::convertible_to<text_model_range_signal &>;

    /// Additional signal that is emitted after the after_erased signal. It is used
    /// for implementing advanced logic when users need to execute some actions
    /// after all clients processed the after_inserted signal.
    { mdl.after_erased_2 } -> std::convertible_to<text_model_range_signal &>;

    /// The signal is emitted before characters are replaced in specified range
    { mdl.before_replaced } -> std::convertible_to<text_model_range_signal &>;

    /// The signal is emitted after characters are replaced in specified range
    { mdl.after_replaced } -> std::convertible_to<text_model_range_signal &>;
};


/// Text model with known maximum line size
template <typename TextModel>
concept text_model_with_max_line_size = text_model<TextModel> && requires (const TextModel & mdl) {
    /// Returns maximum line size in text model
    { mdl.max_line_size() } -> std::convertible_to<uint64_t>;

    /// The signal is emitted when maximum line size in text model is changed
    { mdl.max_line_size_changed } -> std::convertible_to<signal<void ()> &>;
};


/// Alias for type of std string for characters in text model
template <text_model TextModel>
requires std_character<typename TextModel::char_t>
using text_model_string = std::basic_string<typename TextModel::char_t>;


////////////////////////////////////////////////////////////////////////////////


/// Iterator over positions in text model
template <typename TextModel>
class text_model_position_const_iterator:
    public boost::iterator_facade <
        text_model_position_const_iterator<TextModel>,
        position,
        std::bidirectional_iterator_tag,
        position,
        int64_t
    >
{
public:
    /// Constructs invalid iterator
    text_model_position_const_iterator() {}

    /// Constructs iterator for specified model and position
    explicit text_model_position_const_iterator(const TextModel * mdl, const position & p):
        model_{mdl}, pos_{p} {}

    /// Copy constructor
    text_model_position_const_iterator(const text_model_position_const_iterator &) = default;

    /// Returns text position stored in iterator
    position pos() const {
        return pos_;
    }

    /// Moves iterator to the next position
    void increment() {
        assert(model_ != nullptr && "invalid iterator");
        pos_ = next_pos(*model_, pos_);
    }

    /// Moves iterator to the previous position
    void decrement() {
        assert(model_ != nullptr && "invalid iterator");
        pos_ = prev_pos(*model_, pos_);
    }

    /// Compares this iterator with another
    bool equal(const text_model_position_const_iterator & other) const {
        return pos() == other.pos();
    }

    /// Dereferences iterator
    position dereference() const {
        return pos();
    }

private:
    const TextModel * model_ = nullptr;     ///< Pointer to text model
    position pos_;                          ///< Position in text model
};


/// Returns true if position is correct for text model
bool pos_is_valid(const text_model auto & mdl, const position & p) {
    if (p.line >= mdl.lines_size()) {
        return false;
    }

    return p.column <= mdl.line_size(p.line);
}


/// Returns begin position for text model (always returns {0, 0})
position begin_pos(const text_model auto & mdl) {
    return {0, 0};
}


/// Returns end position for text model
position end_pos(const text_model auto & mdl) {
    assert(mdl.lines_size() != 0 && "text model must contain at least single line");
    auto last_line_idx = mdl.lines_size() - 1;
    return {last_line_idx, mdl.line_size(last_line_idx)};
}

/// Returns position after specified position in text model
position next_pos(const text_model auto & mdl, const position & p) {
    assert(pos_is_valid(mdl, p) && "position should be valid");

    if (mdl.line_size(p.line) > p.column) {
        return {p.line, p.column + 1};
    } else {
        assert(p.line + 1 < mdl.lines_size() && "can't get next position for the end position");
        return {p.line + 1, 0};
    }
}

/// Returns position before specified position in text model
position prev_pos(const text_model auto & mdl, const position & p) {
    assert(pos_is_valid(mdl, p) && "position should be valid");

    if (p.column > 0) {
        return {p.line, p.column - 1};
    } else {
        assert(p.line > 0 && "can't get prev position for the beginning position");
        return {p.line - 1, mdl.line_size(p.line - 1)};
    }
}


/// Returns iterator pointing to position of start of text in text model
auto positions_begin(const text_model auto & mdl) {
    return text_model_position_const_iterator{&mdl, begin_pos(mdl)};
}

/// Returns iterator pointing to position of end of text in text model
auto positions_end(const text_model auto & mdl) {
    return text_model_position_const_iterator{&mdl, end_pos(mdl)};
}

/// Returns iterator pointing to specified position
auto positions_iterator_at(const text_model auto & mdl, const position & p) {
    return text_model_position_const_iterator{&mdl, p};
}

/// Returns range of positions in text model
auto positions(const text_model auto & mdl) {
    return std::ranges::subrange(positions_begin(mdl), positions_end(mdl));
}

/// Returns range of positions for specified text range
auto positions(const text_model auto & mdl, const range & r) {
    return std::ranges::subrange(positions_iterator_at(mdl, r.start), positions_iterator_at(mdl, r.end));
}


////////////////////////////////////////////////////////////////////////////////


/// Const iterator over characters in text model
template <typename TextModel>
class text_model_characters_const_iterator:
    public boost::iterator_adaptor <
        text_model_characters_const_iterator<TextModel>,
        text_model_position_const_iterator<TextModel>,
        typename TextModel::char_t,
        std::bidirectional_iterator_tag,
        typename TextModel::char_t,
        int64_t
    >,
    public std::bidirectional_iterator_tag
{
    using base_type = boost::iterator_adaptor <
        text_model_characters_const_iterator<TextModel>,
        text_model_position_const_iterator<TextModel>,
        typename TextModel::char_t,
        std::bidirectional_iterator_tag,
        typename TextModel::char_t,
        int64_t
    >;

public:
    /// Constructs invalid iterator
    text_model_characters_const_iterator() = default;

    /// Copy constructor
    text_model_characters_const_iterator(const text_model_characters_const_iterator &) = default;

    /// Constructs iterator from position iterator
    explicit text_model_characters_const_iterator(const TextModel & mdl,
                                                  const text_model_position_const_iterator<TextModel> & it):
        base_type{it}, mdl_{&mdl} {}

    /// Constructs iterator with specified reference to text model and text position
    explicit text_model_characters_const_iterator(const TextModel & mdl, const position & pos):
        text_model_characters_const_iterator{mdl, positions_iterator_at(mdl, pos)} {}

    /// Returns position of character which this iterator points to
    position pos() const {
        return *this->base();
    }

    /// Dereferences iterator
    typename TextModel::char_t dereference() const {
        assert(mdl_ != nullptr && "dereference of invalid iterator");
        assert(this->base() != positions_end(*mdl_) && "can't get character at the end position");

        // checking for the end of line
        if (this->base()->column == mdl_->line_size(this->base()->line)) {
            return static_cast<typename TextModel::char_t>('\n');
        }

        return mdl_->char_at(*this->base());
    }

private:
    const TextModel * mdl_ = nullptr;       ///< Pointer to text model
};


/// Returns const characters iterator pointing to the beginning of text
auto characters_begin(const text_model auto & mdl) {
    return text_model_characters_const_iterator{mdl, begin_pos(mdl)};
}


/// Returns const characters iterator pointing to the end of text
auto characters_end(const text_model auto & mdl) {
    return text_model_characters_const_iterator{mdl, end_pos(mdl)};
}


/// Returns const characters iterator for specified positions iterator
template <text_model TextModel>
auto characters_iterator_at(const TextModel & mdl, const text_model_position_const_iterator<TextModel> & it) {
    return text_model_characters_const_iterator{mdl, it};
}


/// Returns const characters iterator pointing to specified position
auto characters_iterator_at(const text_model auto & mdl, const position & pos) {
    assert(pos_is_valid(mdl, pos) && "invalid position in text");
    return text_model_characters_const_iterator{mdl, pos};
}


/// Returns range containing characters from specified text range
auto characters(const text_model auto & mdl, const range & r) {
    return std::ranges::subrange(characters_iterator_at(mdl, r.start),
                                 characters_iterator_at(mdl, r.end));
}


/// Returns range containing characters of whole model
auto characters(const text_model auto & mdl) {
    return characters(mdl, {begin_pos(mdl), end_pos(mdl)});
}


/// Returns string containing characters from specified range
template <text_model TextModel>
requires std_character<typename TextModel::char_t>
auto characters_str(const TextModel & mdl, const range & r) {
    auto chars = characters(mdl, r);
    return text_model_string<TextModel>{std::ranges::begin(chars), std::ranges::end(chars)};
}


/// Returns string containing text of whole model
template <text_model TextModel>
requires std_character<typename TextModel::char_t>
auto characters_str(const TextModel & mdl) {
    auto chars = characters(mdl);
    return text_model_string<TextModel>{std::ranges::begin(chars), std::ranges::end(chars)};
}


/// Returns vector containing characters from specified range
template <text_model TextModel>
auto characters_vector(const TextModel & mdl, const range & r) {
    auto chars = characters(mdl, r) | std::ranges::views::common;
    return std::vector<typename TextModel::char_t>{chars.begin(), chars.end()};
}


/// Returns vector containing text of whole model
template <text_model TextModel>
auto characters_vector(const TextModel & mdl) {
    auto chars = characters(mdl) | std::ranges::views::common;
    return std::vector<typename TextModel::char_t>{chars.begin(), chars.end()};
}


/// Returns character pointed by specified character iterator
template <text_model TextModel>
auto character_at(const TextModel & mdl, const text_model_position_const_iterator<TextModel> & it) {
    return mdl.char_at(*it);
}


/// Returns string containing text of whole model (same as characters_str())
auto string(const text_model auto & mdl) {
    return characters_str(mdl);
}


/// Returns range containing all characters of specified line
template <text_model TextModel>
class line_characters_view {
public:
    using char_t = typename TextModel::char_t;

    /// Constructs view for specified model and line
    line_characters_view(const TextModel & mdl, uint64_t line_idx):
        mdl_{&mdl}, line_idx_{line_idx} {}

    /// Returns number of characters in line
    size_t size() const {
        return mdl_->line_size(line_idx_);
    }

    /// Returns true if line is empty
    bool empty() const {
        return size() == 0;
    }

    /// Returns character at specified index
    char_t operator[](size_t idx) const {
        assert(idx < size() && "invalid character index");
        return mdl_->char_at({line_idx_, idx});
    }

    /// Random access iterator over line characters
    class iterator {
    public:
        using iterator_category = std::random_access_iterator_tag;
        using iterator_concept = std::random_access_iterator_tag;
        using value_type = char_t;
        using difference_type = std::ptrdiff_t;
        using reference = value_type;
        using pointer = void;

        iterator() = default;
        iterator(const TextModel * mdl, uint64_t line_idx, size_t idx):
            mdl_{mdl}, line_idx_{line_idx}, idx_{idx} {}

        reference operator*() const {
            return mdl_->char_at({line_idx_, idx_});
        }

        reference operator[](difference_type n) const {
            return mdl_->char_at({line_idx_, idx_ + n});
        }

        iterator & operator++() { ++idx_; return *this; }
        iterator & operator--() { --idx_; return *this; }
        iterator operator++(int) { auto tmp = *this; ++(*this); return tmp; }
        iterator operator--(int) { auto tmp = *this; --(*this); return tmp; }

        iterator & operator+=(difference_type n) { idx_ += n; return *this; }
        iterator & operator-=(difference_type n) { idx_ -= n; return *this; }

        friend iterator operator+(iterator it, difference_type n) { it += n; return it; }
        friend iterator operator+(difference_type n, iterator it) { it += n; return it; }
        friend iterator operator-(iterator it, difference_type n) { it -= n; return it; }
        friend difference_type operator-(const iterator & a, const iterator & b) {
            return static_cast<difference_type>(a.idx_) - static_cast<difference_type>(b.idx_);
        }

        friend bool operator==(const iterator & a, const iterator & b) {
            return a.idx_ == b.idx_ && a.mdl_ == b.mdl_ && a.line_idx_ == b.line_idx_;
        }
        friend bool operator!=(const iterator & a, const iterator & b) { return !(a == b); }
        friend bool operator<(const iterator & a, const iterator & b) { return a.idx_ < b.idx_; }
        friend bool operator>(const iterator & a, const iterator & b) { return b < a; }
        friend bool operator<=(const iterator & a, const iterator & b) { return !(b < a); }
        friend bool operator>=(const iterator & a, const iterator & b) { return !(a < b); }

    private:
        const TextModel * mdl_ = nullptr;
        uint64_t line_idx_ = 0;
        size_t idx_ = 0;
    };

    /// Returns iterator to the beginning
    iterator begin() const { return iterator{mdl_, line_idx_, 0}; }
    /// Returns iterator to the end
    iterator end() const { return iterator{mdl_, line_idx_, size()}; }

private:
    const TextModel * mdl_ = nullptr;
    uint64_t line_idx_ = 0;
};

/// Returns range containing all characters of specified line
template <text_model TextModel>
auto line_characters(const TextModel & mdl, uint64_t line_idx) {
    assert(line_idx < mdl.lines_size() && "invalid line number");
    return line_characters_view<TextModel>{mdl, line_idx};
}

/// Returns vector containing all characters of specified line
template <text_model TextModel>
auto line_chars_vector(const TextModel & mdl, uint64_t line_idx) {
    auto chars = line_characters(mdl, line_idx) | std::ranges::views::common;
    return std::vector<typename TextModel::char_t>{chars.begin(), chars.end()};
}


/// Returns string containing all characters of specified line
template <text_model TextModel>
requires std_character<typename TextModel::char_t>
auto line_str(const TextModel & mdl, uint64_t line_idx) {
    auto chars = line_characters(mdl, line_idx);
    return text_model_string<TextModel>{std::ranges::begin(chars), std::ranges::end(chars)};
}


/// Returns range containing all lines represented as strings
auto lines(const text_model auto & mdl) {
    auto line_numbers = std::ranges::iota_view<uint64_t, uint64_t>{uint64_t{0}, mdl.lines_size()};
    auto fn = [&mdl](uint64_t line_idx) {
        return line_str(mdl, line_idx);
    };
    return line_numbers | std::ranges::views::transform(fn);
}


/// Returns true if character at specified position is space
template <text_model TextModel>
bool char_is_space_at(const TextModel & mdl, const position & pos) {
    auto ch = mdl.char_at(pos);
    auto value = [&] {
        if constexpr (text_model_character<decltype(ch)> && !std_character<decltype(ch)>) {
            return ch.character();
        } else {
            return ch;
        }
    }();

    if constexpr (std::is_same_v<decltype(value), wchar_t>) {
        return std::iswspace(value);
    } else if constexpr (std::is_same_v<decltype(value), char>) {
        return std::isspace(static_cast<unsigned char>(value));
    } else {
        // for now, cast all other character types to char. This should
        // work in most cases even for multibyte unicode string.
        return std::isspace(static_cast<unsigned char>(static_cast<char>(value)));
    }
}


/// Returns true if character at specified position is alphanumeric
template <text_model TextModel>
bool char_is_alnum_at(const TextModel & mdl, const position & pos) {
    auto ch = mdl.char_at(pos);
    auto value = [&] {
        if constexpr (text_model_character<decltype(ch)> && !std_character<decltype(ch)>) {
            return ch.character();
        } else {
            return ch;
        }
    }();

    using value_t = decltype(value);

    // we need convert several bytes in multibyte string to single code point
    // to check character traits for multibyte encodings
    static_assert(std::is_same_v<value_t, wchar_t> || std::is_same_v<value_t, char>,
                    "checking for alphanumeric is not supported for multibyte encodings");

    if constexpr (std::is_same_v<value_t, wchar_t>) {
        return std::iswalnum(value);
    } else if constexpr (std::is_same_v<value_t, char>) {
        return std::isalnum(static_cast<unsigned char>(value));
    }
}


}
