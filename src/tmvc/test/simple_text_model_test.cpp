// Copyright (c) 2025, Alexandr Esilevich
//
// Distributed under the Boost Software License.
// See accompanying file LICENSE for license information.
//

/// \file simple_text_model_test.cpp
/// Contains unit tests for the basic_simple_text_model class.

#include "../simple_text_model.hpp"
#include <boost/test/unit_test.hpp>


namespace tmvc::test {


static_assert(text_model<simple_text_model>);
static_assert(text_model<wsimple_text_model>);

using line_chars_range = decltype(line_characters(std::declval<simple_text_model>(), 0));
static_assert(std::ranges::random_access_range<line_chars_range>);


struct simple_text_model_test_fixture {
    wsimple_text_model text;
};


BOOST_FIXTURE_TEST_SUITE(simple_text_model_test, simple_text_model_test_fixture)


/// Tests default construction
BOOST_AUTO_TEST_CASE(default_ctor) {
    std::wstring txt = string(text);
    BOOST_CHECK(txt == L"");
    BOOST_CHECK(line_str(text, 0).empty());
    BOOST_CHECK_EQUAL(end_pos(text).line, 0);
    BOOST_CHECK_EQUAL(end_pos(text).column, 0);
}


/// Tests construction with text
BOOST_AUTO_TEST_CASE(ctor) {
    wsimple_text_model text2{L"AAAAA\nsss xxx"};
    std::wstring txt = string(text2);
    BOOST_CHECK(txt == L"AAAAA\nsss xxx");
    BOOST_REQUIRE_EQUAL(text2.lines_size(), 2);
    BOOST_CHECK(line_str(text2, 0) == L"AAAAA");
    BOOST_CHECK(line_str(text2, 1) == L"sss xxx");
    BOOST_CHECK_EQUAL(end_pos(text2).line, 1);
    BOOST_CHECK_EQUAL(end_pos(text2).column, 7);
}


/// Tests iterating over positions
BOOST_AUTO_TEST_CASE(iterate_positions) {
    text.reset(L"12\n34");

    auto it = positions_begin(text);
    auto end = positions_end(text);

    BOOST_REQUIRE(it != end);
    BOOST_CHECK_EQUAL(it->line, 0);
    BOOST_CHECK_EQUAL(it->column, 0);

    ++it;
    BOOST_REQUIRE(it != end);
    BOOST_CHECK_EQUAL(it->line, 0);
    BOOST_CHECK_EQUAL(it->column, 1);

    ++it;
    BOOST_REQUIRE(it != end);
    BOOST_CHECK_EQUAL(it->line, 0);
    BOOST_CHECK_EQUAL(it->column, 2);

    ++it;
    BOOST_CHECK_EQUAL(it->line, 1);
    BOOST_CHECK_EQUAL(it->column, 0);

    ++it;
    BOOST_CHECK_EQUAL(it->line, 1);
    BOOST_CHECK_EQUAL(it->column, 1);

    ++it;
    BOOST_CHECK(it == end);
    BOOST_CHECK_EQUAL(it->line, 1);
    BOOST_CHECK_EQUAL(it->column, 2);
}

/// Tests advance_pos with zero offset
BOOST_AUTO_TEST_CASE(advance_pos_zero) {
    text.reset(L"12\n34");
    auto pos = advance_pos(text, {0, 1}, 0);
    BOOST_CHECK_EQUAL(pos.line, 0);
    BOOST_CHECK_EQUAL(pos.column, 1);
}

/// Tests advance_pos on same line
BOOST_AUTO_TEST_CASE(advance_pos_same_line) {
    text.reset(L"12345");
    auto pos = advance_pos(text, {0, 1}, 3);
    BOOST_CHECK_EQUAL(pos.line, 0);
    BOOST_CHECK_EQUAL(pos.column, 4);
}

/// Tests advance_pos across line break
BOOST_AUTO_TEST_CASE(advance_pos_multiline) {
    text.reset(L"12\n345");
    auto pos = advance_pos(text, {0, 1}, 3);
    BOOST_CHECK_EQUAL(pos.line, 1);
    BOOST_CHECK_EQUAL(pos.column, 1);
}

/// Tests advance_pos across multiple line boundaries
BOOST_AUTO_TEST_CASE(advance_pos_multiple_lines) {
    text.reset(L"ab\ncd\nefgh");
    auto pos = advance_pos(text, {0, 0}, 7);
    BOOST_CHECK_EQUAL(pos.line, 2);
    BOOST_CHECK_EQUAL(pos.column, 1);
}


/// Tests iterating over characters
BOOST_AUTO_TEST_CASE(iterate_chars) {
    text.reset(L"12\n34");

    auto it = characters_begin(text);
    auto end = characters_end(text);

    BOOST_REQUIRE(it != end);
    BOOST_CHECK(*it == L'1');

    ++it;
    BOOST_REQUIRE(it != end);
    BOOST_CHECK(*it == L'2');

    ++it;
    BOOST_REQUIRE(it != end);
    BOOST_CHECK(*it == L'\n');

    ++it;
    BOOST_REQUIRE(it != end);
    BOOST_CHECK(*it == L'3');

    ++it;
    BOOST_REQUIRE(it != end);
    BOOST_CHECK(*it == L'4');

    ++it;
    BOOST_CHECK(it == end);
}


/// Tests getting range of characters
BOOST_AUTO_TEST_CASE(range) {
    text.reset(L"this is first line\nsecond line\nlast");
    std::wstring chars = characters_str(text, {{0, 4}, {2, 2}});
    BOOST_CHECK(chars == L" is first line\nsecond line\nla");
}


/// Test getting range of characters containing single line
BOOST_AUTO_TEST_CASE(range_single_line) {
    text.reset(L"this is first line\nsecond line\nlast");
    std::wstring chars = characters_str(text, {{0, 4}, {0, 8}});
    BOOST_CHECK(chars == L" is ");
}


/// Test getting range of characters containing 2 lines
BOOST_AUTO_TEST_CASE(range_2_lines) {
    text.reset(L"this is first line\nsecond line\nlast");
    std::wstring chars = characters_str(text, {{0, 4}, {1, 3}});
    BOOST_CHECK(chars == L" is first line\nsec");
}

/// Tests line_characters size and indexing
BOOST_AUTO_TEST_CASE(line_characters_size_and_index) {
    text.reset(L"abc\nxyz");
    auto line = line_characters(text, 0);
    BOOST_CHECK_EQUAL(std::ranges::size(line), 3);
    BOOST_CHECK_EQUAL(line[1], L'b');
}

/// Tests line_characters empty line behavior
BOOST_AUTO_TEST_CASE(line_characters_empty) {
    text.reset(L"\nabc");
    auto line0 = line_characters(text, 0);
    auto line1 = line_characters(text, 1);
    BOOST_CHECK(line0.empty());
    BOOST_CHECK_EQUAL(std::ranges::size(line0), 0);
    BOOST_CHECK(!line1.empty());
    BOOST_CHECK_EQUAL(std::ranges::size(line1), 3);
    BOOST_CHECK_EQUAL(line1[0], L'a');
}

/// Tests line_characters iterator access and arithmetic
BOOST_AUTO_TEST_CASE(line_characters_iterators) {
    text.reset(L"hello");
    auto line = line_characters(text, 0);
    auto it = line.begin();
    auto end = line.end();
    BOOST_CHECK(it != end);
    BOOST_CHECK_EQUAL(*it, L'h');
    ++it;
    BOOST_CHECK_EQUAL(*it, L'e');
    it += 2;
    BOOST_CHECK_EQUAL(*it, L'l');
    it = it - 1;
    BOOST_CHECK_EQUAL(*it, L'l');
    BOOST_CHECK_EQUAL(end - line.begin(), 5);
    BOOST_CHECK_EQUAL((line.begin() + 4)[0], L'o');
}

/// Tests line_characters on last line without trailing newline
BOOST_AUTO_TEST_CASE(line_characters_last_line) {
    text.reset(L"first\nsecond");
    auto last = line_characters(text, 1);
    BOOST_CHECK_EQUAL(std::ranges::size(last), 6);
    BOOST_CHECK_EQUAL(last[0], L's');
    BOOST_CHECK_EQUAL(last[5], L'd');
}

/// Tests line_characters with formatted changes in the model
BOOST_AUTO_TEST_CASE(line_characters_after_edit) {
    text.reset(L"ab\ncd");
    text.insert({0, 2}, L"X");
    auto line0 = line_characters(text, 0);
    BOOST_CHECK_EQUAL(std::ranges::size(line0), 3);
    BOOST_CHECK_EQUAL(line0[2], L'X');
}


/// Tests setting text
BOOST_AUTO_TEST_CASE(assign_test) {
    int n_before_inserted_calls = 0;
    text.before_inserted.connect([&n_before_inserted_calls, this](auto && r) {
        ++n_before_inserted_calls;
        BOOST_CHECK_EQUAL(r.start.line, 0);
        BOOST_CHECK_EQUAL(r.start.column, 0);
        BOOST_CHECK_EQUAL(r.end.line, 0);
        BOOST_CHECK_EQUAL(r.end.column, 3);
        BOOST_CHECK(string(text) == L"");
    });

    int n_after_inserted_calls = 0;
    text.after_inserted.connect([&n_after_inserted_calls, this](auto && r) {
        ++n_after_inserted_calls;
        BOOST_CHECK_EQUAL(r.start.line, 0);
        BOOST_CHECK_EQUAL(r.start.column, 0);
        BOOST_CHECK_EQUAL(r.end.line, 0);
        BOOST_CHECK_EQUAL(r.end.column, 3);
        BOOST_CHECK(string(text) == L"ZZZ");
    });

    text.before_erased.connect([](auto && r) {
        BOOST_CHECK(false);
    });

    text.after_erased.connect([](auto && r) {
        BOOST_CHECK(false);
    });

    assign(text, L"ZZZ");

    BOOST_CHECK_EQUAL(n_before_inserted_calls, 1);
    BOOST_CHECK_EQUAL(n_after_inserted_calls, 1);
}


/// Tests setting text with new line at end
BOOST_AUTO_TEST_CASE(assign_eol_end) {
    int n_before_inserted_calls = 0;
    text.before_inserted.connect([&n_before_inserted_calls, this](auto && r) {
        ++n_before_inserted_calls;
        BOOST_CHECK_EQUAL(r.start.line, 0);
        BOOST_CHECK_EQUAL(r.start.column, 0);
        BOOST_CHECK_EQUAL(r.end.line, 1);
        BOOST_CHECK_EQUAL(r.end.column, 0);
        BOOST_CHECK(string(text) == L"");
    });

    int n_after_inserted_calls = 0;
    text.after_inserted.connect([&n_after_inserted_calls, this](auto && r) {
        ++n_after_inserted_calls;
        BOOST_CHECK_EQUAL(r.start.line, 0);
        BOOST_CHECK_EQUAL(r.start.column, 0);
        BOOST_CHECK_EQUAL(r.end.line, 1);
        BOOST_CHECK_EQUAL(r.end.column, 0);
        BOOST_CHECK(string(text) == L"ZZZ\n");
    });

    text.before_erased.connect([](auto && r) {
        BOOST_CHECK(false);
    });

    text.after_erased.connect([](auto && r) {
        BOOST_CHECK(false);
    });

    assign(text, L"ZZZ\n");

    BOOST_CHECK_EQUAL(n_before_inserted_calls, 1);
    BOOST_CHECK_EQUAL(n_after_inserted_calls, 1);

    BOOST_CHECK(string(text) == L"ZZZ\n");
    BOOST_CHECK_EQUAL(lines(text).size(), 2);
}


/// Tests replacing text
BOOST_AUTO_TEST_CASE(assign_replace) {
    text.reset(L"the old text\nxx");

    int n_before_removed_calls = 0;
    text.before_erased.connect([this, &n_before_removed_calls](auto && r) {
        ++n_before_removed_calls;
        BOOST_CHECK_EQUAL(r.start.line, 0);
        BOOST_CHECK_EQUAL(r.start.column, 0);
        BOOST_CHECK_EQUAL(r.end.line, 1);
        BOOST_CHECK_EQUAL(r.end.column, 2);
        BOOST_CHECK(string(text) == L"the old text\nxx");
    });

    int n_after_removed_calls = 0;
    text.after_erased.connect([this, &n_after_removed_calls](auto && r) {
        ++n_after_removed_calls;
        BOOST_CHECK_EQUAL(r.start.line, 0);
        BOOST_CHECK_EQUAL(r.start.column, 0);
        BOOST_CHECK_EQUAL(r.end.line, 1);
        BOOST_CHECK_EQUAL(r.end.column, 2);
        BOOST_CHECK(string(text) == L"");
    });

    int n_before_inserted_calls = 0;
    text.before_inserted.connect([this, &n_before_inserted_calls](auto && r) {
        ++n_before_inserted_calls;
        BOOST_CHECK_EQUAL(r.start.line, 0);
        BOOST_CHECK_EQUAL(r.start.column, 0);
        BOOST_CHECK_EQUAL(r.end.line, 2);
        BOOST_CHECK_EQUAL(r.end.column, 3);
        BOOST_CHECK(string(text) == L"");
    });

    int n_after_inserted_calls = 0;
    text.after_inserted.connect([this, &n_after_inserted_calls](auto && r) {
        ++n_after_inserted_calls;
        BOOST_CHECK_EQUAL(r.start.line, 0);
        BOOST_CHECK_EQUAL(r.start.column, 0);
        BOOST_CHECK_EQUAL(r.end.line, 2);
        BOOST_CHECK_EQUAL(r.end.column, 3);
        BOOST_CHECK(string(text) == L"ZZZ\nxx\naaa");
    });

    assign(text, L"ZZZ\nxx\naaa");
    BOOST_CHECK(string(text) == L"ZZZ\nxx\naaa");

    BOOST_CHECK_EQUAL(n_before_inserted_calls, 1);
    BOOST_CHECK_EQUAL(n_after_inserted_calls, 1);
    BOOST_CHECK_EQUAL(n_before_removed_calls, 1);
    BOOST_CHECK_EQUAL(n_after_removed_calls, 1);
}


/// Tests inserting characters
BOOST_AUTO_TEST_CASE(insert) {
    assign(text, L"the old text\nsecond line");

    int n_before_inserted_calls = 0;
    text.before_inserted.connect([this, &n_before_inserted_calls](auto && r) {
        ++n_before_inserted_calls;
        BOOST_CHECK_EQUAL(r.start.line, 0);
        BOOST_CHECK_EQUAL(r.start.column, 4);
        BOOST_CHECK_EQUAL(r.end.line, 2);
        BOOST_CHECK_EQUAL(r.end.column, 5);
        BOOST_CHECK(string(text) == L"the old text\nsecond line");
    });

    int n_after_inserted_calls = 0;
    text.after_inserted.connect([this, &n_after_inserted_calls](auto && r) {
        ++n_after_inserted_calls;
        BOOST_CHECK_EQUAL(r.start.line, 0);
        BOOST_CHECK_EQUAL(r.start.column, 4);
        BOOST_CHECK_EQUAL(r.end.line, 2);
        BOOST_CHECK_EQUAL(r.end.column, 5);
        BOOST_CHECK(string(text) == L"the first\nsecond\nthirdold text\nsecond line");
    });

    text.before_erased.connect([this](auto && r) {
        BOOST_CHECK(false);
    });

    text.after_erased.connect([this](auto && r) {
        BOOST_CHECK(false);
    });

    text.insert({0, 4}, L"first\nsecond\nthird");

    BOOST_CHECK(string(text) == L"the first\nsecond\nthirdold text\nsecond line");

    BOOST_CHECK_EQUAL(n_before_inserted_calls, 1);
    BOOST_CHECK_EQUAL(n_after_inserted_calls, 1);
}


/// Tests inserting characters with no new lines
BOOST_AUTO_TEST_CASE(insert_no_new_lines) {
    assign(text, L"the old text\nsecond line");

    int n_before_inserted_calls = 0;
    text.before_inserted.connect([this, &n_before_inserted_calls](auto && r) {
        ++n_before_inserted_calls;
        BOOST_CHECK_EQUAL(r.start.line, 0);
        BOOST_CHECK_EQUAL(r.start.column, 4);
        BOOST_CHECK_EQUAL(r.end.line, 0);
        BOOST_CHECK_EQUAL(r.end.column, 7);
        BOOST_CHECK(string(text) == L"the old text\nsecond line");
    });

    int n_after_inserted_calls = 0;
    text.after_inserted.connect([this, &n_after_inserted_calls](auto && r) {
        ++n_after_inserted_calls;
        BOOST_CHECK_EQUAL(r.start.line, 0);
        BOOST_CHECK_EQUAL(r.start.column, 4);
        BOOST_CHECK_EQUAL(r.end.line, 0);
        BOOST_CHECK_EQUAL(r.end.column, 7);
        BOOST_CHECK(string(text) == L"the xxxold text\nsecond line");
    });

    text.before_erased.connect([this](auto && r) {
        BOOST_CHECK(false);
    });

    text.after_erased.connect([this](auto && r) {
        BOOST_CHECK(false);
    });

    text.insert({0, 4}, L"xxx");

    BOOST_CHECK(string(text) == L"the xxxold text\nsecond line");

    BOOST_CHECK_EQUAL(n_before_inserted_calls, 1);
    BOOST_CHECK_EQUAL(n_after_inserted_calls, 1);
}


/// Tests removing characters
BOOST_AUTO_TEST_CASE(remove_chars) {
    assign(text, L"the old text\nsecond line\nthird line");

    text.before_inserted.connect([](auto && ...) {
        BOOST_CHECK(false);
    });

    text.after_inserted.connect([](auto && ...) {
        BOOST_CHECK(false);
    });

    int n_before_removed_calls = 0;
    text.before_erased.connect([this, &n_before_removed_calls](auto && r) {
        ++n_before_removed_calls;
        BOOST_CHECK_EQUAL(r.start.line, 0);
        BOOST_CHECK_EQUAL(r.start.column, 6);
        BOOST_CHECK_EQUAL(r.end.line, 2);
        BOOST_CHECK_EQUAL(r.end.column, 2);
        BOOST_CHECK(string(text) == L"the old text\nsecond line\nthird line");
    });

    int n_after_removed_calls = 0;
    text.after_erased.connect([this, &n_after_removed_calls](auto && r) {
        ++n_after_removed_calls;
        BOOST_CHECK_EQUAL(r.start.line, 0);
        BOOST_CHECK_EQUAL(r.start.column, 6);
        BOOST_CHECK_EQUAL(r.end.line, 2);
        BOOST_CHECK_EQUAL(r.end.column, 2);
        BOOST_CHECK(string(text) == L"the olird line");
    });

    text.erase({{0, 6}, {2, 2}});

    BOOST_CHECK(string(text) == L"the olird line");

    BOOST_CHECK_EQUAL(n_before_removed_calls, 1);
    BOOST_CHECK_EQUAL(n_after_removed_calls, 1);
}


/// Tests removing new line characters
BOOST_AUTO_TEST_CASE(remove_new_line) {
    text.reset(L"first line\nsecond line");

    text.before_inserted.connect([this](auto && r) {
        BOOST_CHECK(false);
    });

    text.after_inserted.connect([this](auto && r) {
        BOOST_CHECK(false);
    });

    int n_before_removed_calls = 0;
    text.before_erased.connect([this, &n_before_removed_calls](auto && r) {
        ++n_before_removed_calls;
        BOOST_CHECK_EQUAL(r.start.line, 0);
        BOOST_CHECK_EQUAL(r.start.column, 10);
        BOOST_CHECK_EQUAL(r.end.line, 1);
        BOOST_CHECK_EQUAL(r.end.column, 0);
    
        BOOST_CHECK(string(text) == L"first line\nsecond line");
    });

    int n_after_removed_calls = 0;
    text.after_erased.connect([this, &n_after_removed_calls](auto && r) {
        ++n_after_removed_calls;
        BOOST_CHECK_EQUAL(r.start.line, 0);
        BOOST_CHECK_EQUAL(r.start.column, 10);
        BOOST_CHECK_EQUAL(r.end.line, 1);
        BOOST_CHECK_EQUAL(r.end.column, 0);
    
        BOOST_CHECK(string(text) == L"first linesecond line");
    });

    text.erase({{0, 10}, {1, 0}});

    BOOST_CHECK(string(text) == L"first linesecond line");

    BOOST_CHECK_EQUAL(n_before_removed_calls, 1);
    BOOST_CHECK_EQUAL(n_after_removed_calls, 1);
}


/// Tests replacing characters
BOOST_AUTO_TEST_CASE(test_replace_characters) {
    assign(text, L"the old text\nsecond line");

    text.before_inserted.connect([this](auto && r) {
        BOOST_CHECK(false);
    });

    text.after_inserted.connect([this](auto && r) {
        BOOST_CHECK(false);
    });

    text.before_erased.connect([this](auto && r) {
        BOOST_CHECK(false);
    });

    text.after_erased.connect([this](auto && r) {
        BOOST_CHECK(false);
    });

    int n_before_replaced_called = 0;
    text.before_replaced.connect([this, &n_before_replaced_called](auto && r) {
        ++n_before_replaced_called;
        BOOST_CHECK_EQUAL(r.start.line, 0);
        BOOST_CHECK_EQUAL(r.start.column, 4);
        BOOST_CHECK_EQUAL(r.end.line, 0);
        BOOST_CHECK_EQUAL(r.end.column, 7);
        BOOST_CHECK(string(text) == L"the old text\nsecond line");
    });

    int n_after_replaced_called = 0;
    text.after_replaced.connect([this, &n_after_replaced_called](auto && r) {
        ++n_after_replaced_called;
        BOOST_CHECK_EQUAL(r.start.line, 0);
        BOOST_CHECK_EQUAL(r.start.column, 4);
        BOOST_CHECK_EQUAL(r.end.line, 0);
        BOOST_CHECK_EQUAL(r.end.column, 7);
        BOOST_CHECK(string(text) == L"the xxx text\nsecond line");
    });

    text.replace({0, 4}, L"xxx");

    BOOST_CHECK(string(text) == L"the xxx text\nsecond line");

    BOOST_CHECK_EQUAL(n_before_replaced_called, 1);
    BOOST_CHECK_EQUAL(n_after_replaced_called, 1);
}


/// Tests free replace helper (erase + insert)
BOOST_AUTO_TEST_CASE(test_replace_range_helper) {
    assign(text, L"the old text\nsecond line");

    int n_before_inserted = 0;
    int n_after_inserted = 0;
    int n_before_erased = 0;
    int n_after_erased = 0;
    int n_before_replaced = 0;
    int n_after_replaced = 0;

    text.before_inserted.connect([&](auto &&) { ++n_before_inserted; });
    text.after_inserted.connect([&](auto &&) { ++n_after_inserted; });
    text.before_erased.connect([&](auto &&) { ++n_before_erased; });
    text.after_erased.connect([&](auto &&) { ++n_after_erased; });
    text.before_replaced.connect([&](auto &&) { ++n_before_replaced; });
    text.after_replaced.connect([&](auto &&) { ++n_after_replaced; });

    std::vector<wchar_t> repl{L'n', L'e', L'w'};
    tmvc::replace(text, {{0, 4}, {0, 7}}, repl);

    BOOST_CHECK(string(text) == L"the new text\nsecond line");
    BOOST_CHECK_EQUAL(n_before_erased, 1);
    BOOST_CHECK_EQUAL(n_after_erased, 1);
    BOOST_CHECK_EQUAL(n_before_inserted, 1);
    BOOST_CHECK_EQUAL(n_after_inserted, 1);
    BOOST_CHECK_EQUAL(n_before_replaced, 0);
    BOOST_CHECK_EQUAL(n_after_replaced, 0);
}


BOOST_AUTO_TEST_SUITE_END()


}
