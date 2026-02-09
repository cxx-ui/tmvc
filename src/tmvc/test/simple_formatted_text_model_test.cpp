// Copyright (c) 2025, Alexandr Esilevich
//
// Distributed under the Boost Software License.
// See accompanying file LICENSE for license information.
//

/// \file simple_formatted_text_model_test.cpp
/// Contains unit tests for the basic_simple_text_model class with formatted characters.

#include "../formatted_char.hpp"
#include "../simple_text_model.hpp"
#include <boost/test/unit_test.hpp>
#include <string_view>
#include <vector>


namespace tmvc::test {


namespace {

using fchar_t = tmvc::basic_formatted_char<wchar_t>;
using ftext_t = tmvc::basic_simple_text_model<fchar_t>;

std::vector<fchar_t> make_formatted(std::wstring_view str,
                                    const tmvc::text_format & fmt = {}) {
    std::vector<fchar_t> out;
    out.reserve(str.size());
    for (auto ch : str) {
        out.emplace_back(ch, fmt);
    }
    return out;
}

std::wstring formatted_string(const ftext_t & mdl) {
    std::wstring out;
    for (auto ch : characters(mdl)) {
        out.push_back(ch.character());
    }
    return out;
}

std::wstring formatted_line_str(const ftext_t & mdl, uint64_t line_idx) {
    std::wstring out;
    for (auto ch : line_characters(mdl, line_idx)) {
        out.push_back(ch.character());
    }
    return out;
}

std::wstring formatted_characters_str(const ftext_t & mdl, const range & r) {
    std::wstring out;
    for (auto ch : characters(mdl, r)) {
        out.push_back(ch.character());
    }
    return out;
}

void assign_formatted(ftext_t & mdl, std::wstring_view str) {
    clear(mdl);
    mdl.insert({0, 0}, make_formatted(str));
}

} // namespace


struct simple_formatted_text_model_test_fixture {
    ftext_t text;
};


BOOST_FIXTURE_TEST_SUITE(simple_formatted_text_model_test, simple_formatted_text_model_test_fixture)


/// Tests default construction
BOOST_AUTO_TEST_CASE(default_ctor) {
    std::wstring txt = formatted_string(text);
    BOOST_CHECK(txt == L"");
    BOOST_CHECK(formatted_line_str(text, 0).empty());
    BOOST_CHECK_EQUAL(end_pos(text).line, 0);
    BOOST_CHECK_EQUAL(end_pos(text).column, 0);
}


/// Tests construction with text
BOOST_AUTO_TEST_CASE(ctor) {
    ftext_t text2{make_formatted(L"AAAAA\nsss xxx")};
    std::wstring txt = formatted_string(text2);
    BOOST_CHECK(txt == L"AAAAA\nsss xxx");
    BOOST_REQUIRE_EQUAL(text2.lines_size(), 2);
    BOOST_CHECK(formatted_line_str(text2, 0) == L"AAAAA");
    BOOST_CHECK(formatted_line_str(text2, 1) == L"sss xxx");
    BOOST_CHECK_EQUAL(end_pos(text2).line, 1);
    BOOST_CHECK_EQUAL(end_pos(text2).column, 7);
}


/// Tests iterating over positions
BOOST_AUTO_TEST_CASE(iterate_positions) {
    text.reset(make_formatted(L"12\n34"));

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


/// Tests iterating over characters
BOOST_AUTO_TEST_CASE(iterate_chars) {
    text.reset(make_formatted(L"12\n34"));

    auto it = characters_begin(text);
    auto end = characters_end(text);

    BOOST_REQUIRE(it != end);
    BOOST_CHECK(it->character() == L'1');

    ++it;
    BOOST_REQUIRE(it != end);
    BOOST_CHECK(it->character() == L'2');

    ++it;
    BOOST_REQUIRE(it != end);
    BOOST_CHECK(it->character() == L'\n');

    ++it;
    BOOST_REQUIRE(it != end);
    BOOST_CHECK(it->character() == L'3');

    ++it;
    BOOST_REQUIRE(it != end);
    BOOST_CHECK(it->character() == L'4');

    ++it;
    BOOST_CHECK(it == end);
}


/// Tests getting range of characters
BOOST_AUTO_TEST_CASE(range) {
    text.reset(make_formatted(L"this is first line\nsecond line\nlast"));
    std::wstring chars = formatted_characters_str(text, {{0, 4}, {2, 2}});
    BOOST_CHECK(chars == L" is first line\nsecond line\nla");
}


/// Test getting range of characters containing single line
BOOST_AUTO_TEST_CASE(range_single_line) {
    text.reset(make_formatted(L"this is first line\nsecond line\nlast"));
    std::wstring chars = formatted_characters_str(text, {{0, 4}, {0, 8}});
    BOOST_CHECK(chars == L" is ");
}


/// Test getting range of characters containing 2 lines
BOOST_AUTO_TEST_CASE(range_2_lines) {
    text.reset(make_formatted(L"this is first line\nsecond line\nlast"));
    std::wstring chars = formatted_characters_str(text, {{0, 4}, {1, 3}});
    BOOST_CHECK(chars == L" is first line\nsec");
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
        BOOST_CHECK(formatted_string(text) == L"");
    });

    int n_after_inserted_calls = 0;
    text.after_inserted.connect([&n_after_inserted_calls, this](auto && r) {
        ++n_after_inserted_calls;
        BOOST_CHECK_EQUAL(r.start.line, 0);
        BOOST_CHECK_EQUAL(r.start.column, 0);
        BOOST_CHECK_EQUAL(r.end.line, 0);
        BOOST_CHECK_EQUAL(r.end.column, 3);
        BOOST_CHECK(formatted_string(text) == L"ZZZ");
    });

    text.before_erased.connect([](auto && r) {
        BOOST_CHECK(false);
    });

    text.after_erased.connect([](auto && r) {
        BOOST_CHECK(false);
    });

    assign_formatted(text, L"ZZZ");

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
        BOOST_CHECK(formatted_string(text) == L"");
    });

    int n_after_inserted_calls = 0;
    text.after_inserted.connect([&n_after_inserted_calls, this](auto && r) {
        ++n_after_inserted_calls;
        BOOST_CHECK_EQUAL(r.start.line, 0);
        BOOST_CHECK_EQUAL(r.start.column, 0);
        BOOST_CHECK_EQUAL(r.end.line, 1);
        BOOST_CHECK_EQUAL(r.end.column, 0);
        BOOST_CHECK(formatted_string(text) == L"ZZZ\n");
    });

    text.before_erased.connect([](auto && r) {
        BOOST_CHECK(false);
    });

    text.after_erased.connect([](auto && r) {
        BOOST_CHECK(false);
    });

    assign_formatted(text, L"ZZZ\n");

    BOOST_CHECK_EQUAL(n_before_inserted_calls, 1);
    BOOST_CHECK_EQUAL(n_after_inserted_calls, 1);

    BOOST_CHECK(formatted_string(text) == L"ZZZ\n");
    BOOST_CHECK_EQUAL(text.lines_size(), 2);
}


/// Tests replacing text
BOOST_AUTO_TEST_CASE(assign_replace) {
    text.reset(make_formatted(L"the old text\nxx"));

    int n_before_removed_calls = 0;
    text.before_erased.connect([this, &n_before_removed_calls](auto && r) {
        ++n_before_removed_calls;
        BOOST_CHECK_EQUAL(r.start.line, 0);
        BOOST_CHECK_EQUAL(r.start.column, 0);
        BOOST_CHECK_EQUAL(r.end.line, 1);
        BOOST_CHECK_EQUAL(r.end.column, 2);
        BOOST_CHECK(formatted_string(text) == L"the old text\nxx");
    });

    int n_after_removed_calls = 0;
    text.after_erased.connect([this, &n_after_removed_calls](auto && r) {
        ++n_after_removed_calls;
        BOOST_CHECK_EQUAL(r.start.line, 0);
        BOOST_CHECK_EQUAL(r.start.column, 0);
        BOOST_CHECK_EQUAL(r.end.line, 1);
        BOOST_CHECK_EQUAL(r.end.column, 2);
        BOOST_CHECK(formatted_string(text) == L"");
    });

    int n_before_inserted_calls = 0;
    text.before_inserted.connect([this, &n_before_inserted_calls](auto && r) {
        ++n_before_inserted_calls;
        BOOST_CHECK_EQUAL(r.start.line, 0);
        BOOST_CHECK_EQUAL(r.start.column, 0);
        BOOST_CHECK_EQUAL(r.end.line, 2);
        BOOST_CHECK_EQUAL(r.end.column, 3);
        BOOST_CHECK(formatted_string(text) == L"");
    });

    int n_after_inserted_calls = 0;
    text.after_inserted.connect([this, &n_after_inserted_calls](auto && r) {
        ++n_after_inserted_calls;
        BOOST_CHECK_EQUAL(r.start.line, 0);
        BOOST_CHECK_EQUAL(r.start.column, 0);
        BOOST_CHECK_EQUAL(r.end.line, 2);
        BOOST_CHECK_EQUAL(r.end.column, 3);
        BOOST_CHECK(formatted_string(text) == L"ZZZ\nxx\naaa");
    });

    assign_formatted(text, L"ZZZ\nxx\naaa");
    BOOST_CHECK(formatted_string(text) == L"ZZZ\nxx\naaa");

    BOOST_CHECK_EQUAL(n_before_inserted_calls, 1);
    BOOST_CHECK_EQUAL(n_after_inserted_calls, 1);
    BOOST_CHECK_EQUAL(n_before_removed_calls, 1);
    BOOST_CHECK_EQUAL(n_after_removed_calls, 1);
}


/// Tests inserting characters
BOOST_AUTO_TEST_CASE(insert) {
    assign_formatted(text, L"the old text\nsecond line");

    int n_before_inserted_calls = 0;
    text.before_inserted.connect([this, &n_before_inserted_calls](auto && r) {
        ++n_before_inserted_calls;
        BOOST_CHECK_EQUAL(r.start.line, 0);
        BOOST_CHECK_EQUAL(r.start.column, 4);
        BOOST_CHECK_EQUAL(r.end.line, 2);
        BOOST_CHECK_EQUAL(r.end.column, 5);
        BOOST_CHECK(formatted_string(text) == L"the old text\nsecond line");
    });

    int n_after_inserted_calls = 0;
    text.after_inserted.connect([this, &n_after_inserted_calls](auto && r) {
        ++n_after_inserted_calls;
        BOOST_CHECK_EQUAL(r.start.line, 0);
        BOOST_CHECK_EQUAL(r.start.column, 4);
        BOOST_CHECK_EQUAL(r.end.line, 2);
        BOOST_CHECK_EQUAL(r.end.column, 5);
        BOOST_CHECK(formatted_string(text) == L"the first\nsecond\nthirdold text\nsecond line");
    });

    text.before_erased.connect([this](auto && r) {
        BOOST_CHECK(false);
    });

    text.after_erased.connect([this](auto && r) {
        BOOST_CHECK(false);
    });

    text.insert({0, 4}, make_formatted(L"first\nsecond\nthird"));

    BOOST_CHECK(formatted_string(text) == L"the first\nsecond\nthirdold text\nsecond line");

    BOOST_CHECK_EQUAL(n_before_inserted_calls, 1);
    BOOST_CHECK_EQUAL(n_after_inserted_calls, 1);
}


/// Tests inserting characters with no new lines
BOOST_AUTO_TEST_CASE(insert_no_new_lines) {
    assign_formatted(text, L"the old text\nsecond line");

    int n_before_inserted_calls = 0;
    text.before_inserted.connect([this, &n_before_inserted_calls](auto && r) {
        ++n_before_inserted_calls;
        BOOST_CHECK_EQUAL(r.start.line, 0);
        BOOST_CHECK_EQUAL(r.start.column, 4);
        BOOST_CHECK_EQUAL(r.end.line, 0);
        BOOST_CHECK_EQUAL(r.end.column, 7);
        BOOST_CHECK(formatted_string(text) == L"the old text\nsecond line");
    });

    int n_after_inserted_calls = 0;
    text.after_inserted.connect([this, &n_after_inserted_calls](auto && r) {
        ++n_after_inserted_calls;
        BOOST_CHECK_EQUAL(r.start.line, 0);
        BOOST_CHECK_EQUAL(r.start.column, 4);
        BOOST_CHECK_EQUAL(r.end.line, 0);
        BOOST_CHECK_EQUAL(r.end.column, 7);
        BOOST_CHECK(formatted_string(text) == L"the xxxold text\nsecond line");
    });

    text.before_erased.connect([this](auto && r) {
        BOOST_CHECK(false);
    });

    text.after_erased.connect([this](auto && r) {
        BOOST_CHECK(false);
    });

    text.insert({0, 4}, make_formatted(L"xxx"));

    BOOST_CHECK(formatted_string(text) == L"the xxxold text\nsecond line");

    BOOST_CHECK_EQUAL(n_before_inserted_calls, 1);
    BOOST_CHECK_EQUAL(n_after_inserted_calls, 1);
}


/// Tests removing characters
BOOST_AUTO_TEST_CASE(remove_chars) {
    assign_formatted(text, L"the old text\nsecond line\nthird line");

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
        BOOST_CHECK(formatted_string(text) == L"the old text\nsecond line\nthird line");
    });

    int n_after_removed_calls = 0;
    text.after_erased.connect([this, &n_after_removed_calls](auto && r) {
        ++n_after_removed_calls;
        BOOST_CHECK_EQUAL(r.start.line, 0);
        BOOST_CHECK_EQUAL(r.start.column, 6);
        BOOST_CHECK_EQUAL(r.end.line, 2);
        BOOST_CHECK_EQUAL(r.end.column, 2);
        BOOST_CHECK(formatted_string(text) == L"the olird line");
    });

    text.erase({{0, 6}, {2, 2}});

    BOOST_CHECK(formatted_string(text) == L"the olird line");

    BOOST_CHECK_EQUAL(n_before_removed_calls, 1);
    BOOST_CHECK_EQUAL(n_after_removed_calls, 1);
}


/// Tests removing new line characters
BOOST_AUTO_TEST_CASE(remove_new_line) {
    text.reset(make_formatted(L"first line\nsecond line"));

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
    
        BOOST_CHECK(formatted_string(text) == L"first line\nsecond line");
    });

    int n_after_removed_calls = 0;
    text.after_erased.connect([this, &n_after_removed_calls](auto && r) {
        ++n_after_removed_calls;
        BOOST_CHECK_EQUAL(r.start.line, 0);
        BOOST_CHECK_EQUAL(r.start.column, 10);
        BOOST_CHECK_EQUAL(r.end.line, 1);
        BOOST_CHECK_EQUAL(r.end.column, 0);
    
        BOOST_CHECK(formatted_string(text) == L"first linesecond line");
    });

    text.erase({{0, 10}, {1, 0}});

    BOOST_CHECK(formatted_string(text) == L"first linesecond line");

    BOOST_CHECK_EQUAL(n_before_removed_calls, 1);
    BOOST_CHECK_EQUAL(n_after_removed_calls, 1);
}


/// Tests replacing characters
BOOST_AUTO_TEST_CASE(test_replace_characters) {
    assign_formatted(text, L"the old text\nsecond line");

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
        BOOST_CHECK(formatted_string(text) == L"the old text\nsecond line");
    });

    int n_after_replaced_called = 0;
    text.after_replaced.connect([this, &n_after_replaced_called](auto && r) {
        ++n_after_replaced_called;
        BOOST_CHECK_EQUAL(r.start.line, 0);
        BOOST_CHECK_EQUAL(r.start.column, 4);
        BOOST_CHECK_EQUAL(r.end.line, 0);
        BOOST_CHECK_EQUAL(r.end.column, 7);
        BOOST_CHECK(formatted_string(text) == L"the xxx text\nsecond line");
    });

    text.replace({0, 4}, make_formatted(L"xxx"));

    BOOST_CHECK(formatted_string(text) == L"the xxx text\nsecond line");

    BOOST_CHECK_EQUAL(n_before_replaced_called, 1);
    BOOST_CHECK_EQUAL(n_after_replaced_called, 1);
}


BOOST_AUTO_TEST_SUITE_END()


}
