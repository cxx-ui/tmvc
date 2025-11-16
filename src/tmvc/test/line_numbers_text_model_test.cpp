// Copyright (c) 2025, Alexandr Esilevich
//
// Distributed under the Boost Software License.
// See accompanying file LICENSE for license information.
//

/// \file line_numbers_text_model_test.cpp
/// Contains unit tests for the line_numbers_text_model class.

#include "../simple_text_model.hpp"
#include "../text_line_numbers_model.hpp"
#include "../line_numbers_text_model.hpp"
#include <boost/test/unit_test.hpp>


namespace tmvc::test {


struct line_numbers_text_model_test_fixture {
    using text_model_t = wsimple_text_model;
    using line_numbers_model_t = text_line_numbers_model<text_model_t>;
    using line_numbers_text_model_t = line_numbers_text_model <
        text_model_t,
        line_numbers_model_t
    >;

    text_model_t text{L"this is first line\nsecond line\nthird\nfourth\nlast"};
    line_numbers_model_t line_numbers{text};
    line_numbers_text_model_t line_numbers_text{text, line_numbers};

    static_assert(text_model<line_numbers_text_model_t>);
};


BOOST_FIXTURE_TEST_SUITE(line_numbers_text_model_test, line_numbers_text_model_test_fixture)


/// Tests returning number of lines
BOOST_AUTO_TEST_CASE(test_lines_size) {
    BOOST_CHECK_EQUAL(line_numbers_text.lines_size(), 5);
}


/// Tests returning line numbers
BOOST_AUTO_TEST_CASE(test_line_numbers) {
    BOOST_REQUIRE_EQUAL(line_numbers_text.lines_size(), 5);
    BOOST_CHECK(line_str(line_numbers_text, 0) == L"1");
    BOOST_CHECK(line_str(line_numbers_text, 1) == L"2");
    BOOST_CHECK(line_str(line_numbers_text, 2) == L"3");
    BOOST_CHECK(line_str(line_numbers_text, 3) == L"4");
    BOOST_CHECK(line_str(line_numbers_text, 4) == L"5");
}


/// Tests inserting and erasing characters without removing lines
BOOST_AUTO_TEST_CASE(test_insert_erase_single_line) {
    // line numbers text model should not emit signals if lines are
    // not inserted / erased

    line_numbers_text.before_inserted.connect([](auto && ...) {
        BOOST_CHECK(false);
    });

    line_numbers_text.after_inserted.connect([](auto && ...) {
        BOOST_CHECK(false);
    });

    line_numbers_text.before_erased.connect([](auto && ...) {
        BOOST_CHECK(false);
    });

    line_numbers_text.after_erased.connect([](auto && ...) {
        BOOST_CHECK(false);
    });

    line_numbers_text.before_replaced.connect([](auto && ...) {
        BOOST_CHECK(false);
    });

    line_numbers_text.after_replaced.connect([](auto && ...) {
        BOOST_CHECK(false);
    });

    text.erase({{0, 1}, {0, 2}});
    text.insert({1, 2}, L"aaaa");
}


/// Tests inserting lines
BOOST_AUTO_TEST_CASE(test_insert) {
    int n_before_inserted_called = 0;
    line_numbers_text.before_inserted.connect([&](auto && range) {
        BOOST_CHECK_EQUAL(range.start.line, 0);
        BOOST_CHECK_EQUAL(range.start.column, 1);
        BOOST_CHECK_EQUAL(range.end.line, 2);
        BOOST_CHECK_EQUAL(range.end.column, 1);

        BOOST_CHECK_EQUAL(line_numbers_text.lines_size(), 5);
        BOOST_CHECK(line_str(line_numbers_text, 0) == L"1");
        BOOST_CHECK(line_str(line_numbers_text, 1) == L"2");
        BOOST_CHECK(line_str(line_numbers_text, 2) == L"3");
        BOOST_CHECK(line_str(line_numbers_text, 3) == L"4");
        BOOST_CHECK(line_str(line_numbers_text, 4) == L"5");

        ++n_before_inserted_called;
    });

    int n_after_inserted_called = 0;
    line_numbers_text.after_inserted.connect([&](auto && range) {
        BOOST_CHECK_EQUAL(range.start.line, 0);
        BOOST_CHECK_EQUAL(range.start.column, 1);
        BOOST_CHECK_EQUAL(range.end.line, 2);
        BOOST_CHECK_EQUAL(range.end.column, 1);
        
        BOOST_CHECK_EQUAL(line_numbers_text.lines_size(), 7);
        BOOST_CHECK(line_str(line_numbers_text, 0) == L"1");
        BOOST_CHECK(line_str(line_numbers_text, 1) == L"2");
        BOOST_CHECK(line_str(line_numbers_text, 2) == L"3");
        BOOST_CHECK(line_str(line_numbers_text, 3) == L"2");
        BOOST_CHECK(line_str(line_numbers_text, 4) == L"3");
        BOOST_CHECK(line_str(line_numbers_text, 5) == L"4");
        BOOST_CHECK(line_str(line_numbers_text, 6) == L"5");

        ++n_after_inserted_called;
    });

    int n_after_inserted_2_called = 0;
    line_numbers_text.after_inserted_2.connect([&](auto && range) {
        BOOST_CHECK_EQUAL(range.start.line, 0);
        BOOST_CHECK_EQUAL(range.start.column, 1);
        BOOST_CHECK_EQUAL(range.end.line, 2);
        BOOST_CHECK_EQUAL(range.end.column, 1);
        
        BOOST_CHECK_EQUAL(line_numbers_text.lines_size(), 7);
        BOOST_CHECK(line_str(line_numbers_text, 0) == L"1");
        BOOST_CHECK(line_str(line_numbers_text, 1) == L"2");
        BOOST_CHECK(line_str(line_numbers_text, 2) == L"3");
        BOOST_CHECK(line_str(line_numbers_text, 3) == L"2");
        BOOST_CHECK(line_str(line_numbers_text, 4) == L"3");
        BOOST_CHECK(line_str(line_numbers_text, 5) == L"4");
        BOOST_CHECK(line_str(line_numbers_text, 6) == L"5");

        ++n_after_inserted_2_called;
    });

    line_numbers_text.before_erased.connect([&](auto && range) {
        BOOST_CHECK(false);
    });

    line_numbers_text.after_erased.connect([&](auto && range) {
        BOOST_CHECK(false);
    });

    int n_before_replaced_called = 0;
    line_numbers_text.before_replaced.connect([&](auto && range) {
        // replaced signals must be called after erased signals
        BOOST_CHECK_EQUAL(n_before_inserted_called, 1);
        BOOST_CHECK_EQUAL(n_after_inserted_called, 1);

        BOOST_CHECK_EQUAL(range.start.line, 3);
        BOOST_CHECK_EQUAL(range.start.column, 0);
        BOOST_CHECK_EQUAL(range.end.line, 6);
        BOOST_CHECK_EQUAL(range.end.column, 1);

        BOOST_CHECK_EQUAL(line_numbers_text.lines_size(), 7);
        BOOST_CHECK(line_str(line_numbers_text, 0) == L"1");
        BOOST_CHECK(line_str(line_numbers_text, 1) == L"2");
        BOOST_CHECK(line_str(line_numbers_text, 2) == L"3");
        BOOST_CHECK(line_str(line_numbers_text, 3) == L"2");
        BOOST_CHECK(line_str(line_numbers_text, 4) == L"3");
        BOOST_CHECK(line_str(line_numbers_text, 5) == L"4");
        BOOST_CHECK(line_str(line_numbers_text, 6) == L"5");

        ++n_before_replaced_called;
    });

    int n_after_replaced_called = 0;
    line_numbers_text.after_replaced.connect([&](auto && range) {
        // replaced signals must be called after erased signals
        BOOST_CHECK_EQUAL(n_before_inserted_called, 1);
        BOOST_CHECK_EQUAL(n_after_inserted_called, 1);

        BOOST_CHECK_EQUAL(range.start.line, 3);
        BOOST_CHECK_EQUAL(range.start.column, 0);
        BOOST_CHECK_EQUAL(range.end.line, 6);
        BOOST_CHECK_EQUAL(range.end.column, 1);

        BOOST_CHECK_EQUAL(line_numbers_text.lines_size(), 7);
        BOOST_CHECK(line_str(line_numbers_text, 0) == L"1");
        BOOST_CHECK(line_str(line_numbers_text, 1) == L"2");
        BOOST_CHECK(line_str(line_numbers_text, 2) == L"3");
        BOOST_CHECK(line_str(line_numbers_text, 3) == L"4");
        BOOST_CHECK(line_str(line_numbers_text, 4) == L"5");
        BOOST_CHECK(line_str(line_numbers_text, 5) == L"6");
        BOOST_CHECK(line_str(line_numbers_text, 6) == L"7");

        ++n_after_replaced_called;
    });

    text.insert({0, 1}, L"two\nlines\nxxx");

    BOOST_REQUIRE_EQUAL(line_numbers_text.lines_size(), 7);
    BOOST_CHECK(line_str(line_numbers_text, 0) == L"1");
    BOOST_CHECK(line_str(line_numbers_text, 1) == L"2");
    BOOST_CHECK(line_str(line_numbers_text, 2) == L"3");
    BOOST_CHECK(line_str(line_numbers_text, 3) == L"4");
    BOOST_CHECK(line_str(line_numbers_text, 4) == L"5");
    BOOST_CHECK(line_str(line_numbers_text, 5) == L"6");
    BOOST_CHECK(line_str(line_numbers_text, 6) == L"7");

    BOOST_CHECK_EQUAL(n_before_inserted_called, 1);
    BOOST_CHECK_EQUAL(n_after_inserted_called, 1);
    BOOST_CHECK_EQUAL(n_after_inserted_2_called, 1);
    BOOST_CHECK_EQUAL(n_before_replaced_called, 1);
    BOOST_CHECK_EQUAL(n_after_replaced_called, 1);
}


/// Tests inserting lines at the end
BOOST_AUTO_TEST_CASE(test_insert_end) {
    int n_before_inserted_called = 0;
    line_numbers_text.before_inserted.connect([&](auto && range) {
        BOOST_CHECK_EQUAL(range.start.line, 4);
        BOOST_CHECK_EQUAL(range.start.column, 1);
        BOOST_CHECK_EQUAL(range.end.line, 6);
        BOOST_CHECK_EQUAL(range.end.column, 1);

        BOOST_CHECK_EQUAL(line_numbers_text.lines_size(), 5);
        BOOST_CHECK(line_str(line_numbers_text, 0) == L"1");
        BOOST_CHECK(line_str(line_numbers_text, 1) == L"2");
        BOOST_CHECK(line_str(line_numbers_text, 2) == L"3");
        BOOST_CHECK(line_str(line_numbers_text, 3) == L"4");
        BOOST_CHECK(line_str(line_numbers_text, 4) == L"5");

        ++n_before_inserted_called;
    });

    int n_after_inserted_called = 0;
    line_numbers_text.after_inserted.connect([&](auto && range) {
        BOOST_CHECK_EQUAL(range.start.line, 4);
        BOOST_CHECK_EQUAL(range.start.column, 1);
        BOOST_CHECK_EQUAL(range.end.line, 6);
        BOOST_CHECK_EQUAL(range.end.column, 1);
        
        BOOST_CHECK_EQUAL(line_numbers_text.lines_size(), 7);
        BOOST_CHECK(line_str(line_numbers_text, 0) == L"1");
        BOOST_CHECK(line_str(line_numbers_text, 1) == L"2");
        BOOST_CHECK(line_str(line_numbers_text, 2) == L"3");
        BOOST_CHECK(line_str(line_numbers_text, 3) == L"4");
        BOOST_CHECK(line_str(line_numbers_text, 4) == L"5");
        BOOST_CHECK(line_str(line_numbers_text, 5) == L"6");
        BOOST_CHECK(line_str(line_numbers_text, 6) == L"7");

        ++n_after_inserted_called;
    });

    int n_after_inserted_2_called = 0;
    line_numbers_text.after_inserted_2.connect([&](auto && range) {
        BOOST_CHECK_EQUAL(range.start.line, 4);
        BOOST_CHECK_EQUAL(range.start.column, 1);
        BOOST_CHECK_EQUAL(range.end.line, 6);
        BOOST_CHECK_EQUAL(range.end.column, 1);
        
        BOOST_CHECK_EQUAL(line_numbers_text.lines_size(), 7);
        BOOST_CHECK(line_str(line_numbers_text, 0) == L"1");
        BOOST_CHECK(line_str(line_numbers_text, 1) == L"2");
        BOOST_CHECK(line_str(line_numbers_text, 2) == L"3");
        BOOST_CHECK(line_str(line_numbers_text, 3) == L"4");
        BOOST_CHECK(line_str(line_numbers_text, 4) == L"5");
        BOOST_CHECK(line_str(line_numbers_text, 5) == L"6");
        BOOST_CHECK(line_str(line_numbers_text, 6) == L"7");

        ++n_after_inserted_2_called;
    });

    line_numbers_text.before_erased.connect([&](auto && range) {
        BOOST_CHECK(false);
    });

    line_numbers_text.after_erased.connect([&](auto && range) {
        BOOST_CHECK(false);
    });

    line_numbers_text.before_replaced.connect([&](auto && range) {
        BOOST_CHECK(false);
    });

    line_numbers_text.after_replaced.connect([&](auto && range) {
        BOOST_CHECK(false);
    });

    text.insert({4, 4}, L"two\nlines\nxxx");

    BOOST_REQUIRE_EQUAL(line_numbers_text.lines_size(), 7);
    BOOST_CHECK(line_str(line_numbers_text, 0) == L"1");
    BOOST_CHECK(line_str(line_numbers_text, 1) == L"2");
    BOOST_CHECK(line_str(line_numbers_text, 2) == L"3");
    BOOST_CHECK(line_str(line_numbers_text, 3) == L"4");
    BOOST_CHECK(line_str(line_numbers_text, 4) == L"5");
    BOOST_CHECK(line_str(line_numbers_text, 5) == L"6");
    BOOST_CHECK(line_str(line_numbers_text, 6) == L"7");

    BOOST_CHECK_EQUAL(n_before_inserted_called, 1);
    BOOST_CHECK_EQUAL(n_after_inserted_called, 1);
    BOOST_CHECK_EQUAL(n_after_inserted_2_called, 1);
}


/// Tests erasing lines
BOOST_AUTO_TEST_CASE(test_erase) {
    line_numbers_text.before_inserted.connect([](auto && range) {
        BOOST_CHECK(false);
    });

    line_numbers_text.after_inserted.connect([](auto && range) {
        BOOST_CHECK(false);
    });

    int n_before_replaced_called = 0;
    int n_after_replaced_called = 0;

    int n_before_erased_called = 0;
    line_numbers_text.before_erased.connect([&](auto && range) {
        // replaced signals must be called before replaced signals
        BOOST_CHECK_EQUAL(n_before_replaced_called, 1);
        BOOST_CHECK_EQUAL(n_after_replaced_called, 1);

        BOOST_CHECK_EQUAL(range.start.line, 0);
        BOOST_CHECK_EQUAL(range.start.column, 1);
        BOOST_CHECK_EQUAL(range.end.line, 2);
        BOOST_CHECK_EQUAL(range.end.column, 1);

        BOOST_CHECK_EQUAL(line_numbers_text.lines_size(), 5);
        BOOST_CHECK(line_str(line_numbers_text, 0) == L"1");
        BOOST_CHECK(line_str(line_numbers_text, 1) == L"2");
        BOOST_CHECK(line_str(line_numbers_text, 2) == L"3");
        BOOST_CHECK(line_str(line_numbers_text, 3) == L"2");
        BOOST_CHECK(line_str(line_numbers_text, 4) == L"3");

        ++n_before_erased_called;
    });

    int n_after_erased_called = 0;
    line_numbers_text.after_erased.connect([&](auto && range) {
        // replaced signals must be called before replaced signals
        BOOST_CHECK_EQUAL(n_before_replaced_called, 1);
        BOOST_CHECK_EQUAL(n_after_replaced_called, 1);

        BOOST_CHECK_EQUAL(range.start.line, 0);
        BOOST_CHECK_EQUAL(range.start.column, 1);
        BOOST_CHECK_EQUAL(range.end.line, 2);
        BOOST_CHECK_EQUAL(range.end.column, 1);

        BOOST_REQUIRE_EQUAL(line_numbers_text.lines_size(), 3);
        BOOST_CHECK(line_str(line_numbers_text, 0) == L"1");
        BOOST_CHECK(line_str(line_numbers_text, 1) == L"2");
        BOOST_CHECK(line_str(line_numbers_text, 2) == L"3");

        ++n_after_erased_called;
    });

    int n_after_erased_2_called = 0;
    line_numbers_text.after_erased_2.connect([&](auto && range) {
        // replaced signals must be called before replaced signals
        BOOST_CHECK_EQUAL(n_before_replaced_called, 1);
        BOOST_CHECK_EQUAL(n_after_replaced_called, 1);
        BOOST_CHECK_EQUAL(n_after_erased_called, 1);

        BOOST_CHECK_EQUAL(range.start.line, 0);
        BOOST_CHECK_EQUAL(range.start.column, 1);
        BOOST_CHECK_EQUAL(range.end.line, 2);
        BOOST_CHECK_EQUAL(range.end.column, 1);

        BOOST_REQUIRE_EQUAL(line_numbers_text.lines_size(), 3);
        BOOST_CHECK(line_str(line_numbers_text, 0) == L"1");
        BOOST_CHECK(line_str(line_numbers_text, 1) == L"2");
        BOOST_CHECK(line_str(line_numbers_text, 2) == L"3");

        ++n_after_erased_2_called;
    });

    line_numbers_text.before_replaced.connect([&](auto && range) {
        BOOST_CHECK_EQUAL(range.start.line, 3);
        BOOST_CHECK_EQUAL(range.start.column, 0);
        BOOST_CHECK_EQUAL(range.end.line, 4);
        BOOST_CHECK_EQUAL(range.end.column, 1);

        BOOST_CHECK_EQUAL(line_numbers_text.lines_size(), 5);
        BOOST_CHECK(line_str(line_numbers_text, 0) == L"1");
        BOOST_CHECK(line_str(line_numbers_text, 1) == L"2");
        BOOST_CHECK(line_str(line_numbers_text, 2) == L"3");
        BOOST_CHECK(line_str(line_numbers_text, 3) == L"4");
        BOOST_CHECK(line_str(line_numbers_text, 4) == L"5");

        ++n_before_replaced_called;
    });

    line_numbers_text.after_replaced.connect([&](auto && range) {
        BOOST_CHECK_EQUAL(range.start.line, 3);
        BOOST_CHECK_EQUAL(range.start.column, 0);
        BOOST_CHECK_EQUAL(range.end.line, 4);
        BOOST_CHECK_EQUAL(range.end.column, 1);

        BOOST_CHECK_EQUAL(line_numbers_text.lines_size(), 5);
        BOOST_CHECK(line_str(line_numbers_text, 0) == L"1");
        BOOST_CHECK(line_str(line_numbers_text, 1) == L"2");
        BOOST_CHECK(line_str(line_numbers_text, 2) == L"3");
        BOOST_CHECK(line_str(line_numbers_text, 3) == L"2");
        BOOST_CHECK(line_str(line_numbers_text, 4) == L"3");

        ++n_after_replaced_called;
    });

    text.erase({{0, 1}, {2, 2}});

    BOOST_REQUIRE_EQUAL(line_numbers_text.lines_size(), 3);
    BOOST_CHECK(line_str(line_numbers_text, 0) == L"1");
    BOOST_CHECK(line_str(line_numbers_text, 1) == L"2");
    BOOST_CHECK(line_str(line_numbers_text, 2) == L"3");

    BOOST_CHECK_EQUAL(n_before_erased_called, 1);
    BOOST_CHECK_EQUAL(n_after_erased_called, 1);
    BOOST_CHECK_EQUAL(n_after_erased_2_called, 1);
    BOOST_CHECK_EQUAL(n_before_replaced_called, 1);
    BOOST_CHECK_EQUAL(n_after_replaced_called, 1);
}


/// Tests erasing lines at the end of text
BOOST_AUTO_TEST_CASE(test_erase_end) {
    line_numbers_text.before_inserted.connect([](auto && range) {
        BOOST_CHECK(false);
    });

    line_numbers_text.after_inserted.connect([](auto && range) {
        BOOST_CHECK(false);
    });

    int n_before_erased_called = 0;
    line_numbers_text.before_erased.connect([&](auto && range) {
        BOOST_CHECK_EQUAL(range.start.line, 2);
        BOOST_CHECK_EQUAL(range.start.column, 1);
        BOOST_CHECK_EQUAL(range.end.line, 4);
        BOOST_CHECK_EQUAL(range.end.column, 1);

        BOOST_CHECK_EQUAL(line_numbers_text.lines_size(), 5);
        BOOST_CHECK(line_str(line_numbers_text, 0) == L"1");
        BOOST_CHECK(line_str(line_numbers_text, 1) == L"2");
        BOOST_CHECK(line_str(line_numbers_text, 2) == L"3");
        BOOST_CHECK(line_str(line_numbers_text, 3) == L"4");
        BOOST_CHECK(line_str(line_numbers_text, 4) == L"5");

        ++n_before_erased_called;
    });

    int n_after_erased_called = 0;
    line_numbers_text.after_erased.connect([&](auto && range) {
        BOOST_CHECK_EQUAL(range.start.line, 2);
        BOOST_CHECK_EQUAL(range.start.column, 1);
        BOOST_CHECK_EQUAL(range.end.line, 4);
        BOOST_CHECK_EQUAL(range.end.column, 1);

        BOOST_REQUIRE_EQUAL(line_numbers_text.lines_size(), 3);
        BOOST_CHECK(line_str(line_numbers_text, 0) == L"1");
        BOOST_CHECK(line_str(line_numbers_text, 1) == L"2");
        BOOST_CHECK(line_str(line_numbers_text, 2) == L"3");

        ++n_after_erased_called;
    });

    int n_after_erased_2_called = 0;
    line_numbers_text.after_erased_2.connect([&](auto && range) {
        BOOST_CHECK_EQUAL(range.start.line, 2);
        BOOST_CHECK_EQUAL(range.start.column, 1);
        BOOST_CHECK_EQUAL(range.end.line, 4);
        BOOST_CHECK_EQUAL(range.end.column, 1);

        BOOST_REQUIRE_EQUAL(line_numbers_text.lines_size(), 3);
        BOOST_CHECK(line_str(line_numbers_text, 0) == L"1");
        BOOST_CHECK(line_str(line_numbers_text, 1) == L"2");
        BOOST_CHECK(line_str(line_numbers_text, 2) == L"3");

        ++n_after_erased_2_called;
    });

    line_numbers_text.before_replaced.connect([&](auto && range) {
        BOOST_CHECK(false);
    });

    line_numbers_text.after_replaced.connect([&](auto && range) {
        BOOST_CHECK(false);
    });

    text.erase({{2, 2}, {4, 4}});

    BOOST_REQUIRE_EQUAL(line_numbers_text.lines_size(), 3);
    BOOST_CHECK(line_str(line_numbers_text, 0) == L"1");
    BOOST_CHECK(line_str(line_numbers_text, 1) == L"2");
    BOOST_CHECK(line_str(line_numbers_text, 2) == L"3");

    BOOST_CHECK_EQUAL(n_before_erased_called, 1);
    BOOST_CHECK_EQUAL(n_after_erased_called, 1);
    BOOST_CHECK_EQUAL(n_after_erased_2_called, 1);
}


BOOST_AUTO_TEST_SUITE_END()


}
