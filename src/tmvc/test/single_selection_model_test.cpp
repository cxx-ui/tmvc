// Copyright (c) 2025, Alexandr Esilevich
//
// Distributed under the Boost Software License.
// See accompanying file LICENSE for license information.
//

/// \file simple_selection_model_test.cpp
/// Contains unit tests for the basic_simple_selection_model class.

#include "../simple_text_model.hpp"
#include "../single_selection_model.hpp"
#include <boost/test/unit_test.hpp>


namespace tmvc::test {


static_assert(selection_model<single_selection_model<simple_text_model>>);


struct simple_selection_model_test_fixture {
    wsimple_text_model text;
    single_selection_model<wsimple_text_model> selection{text};
};


BOOST_FIXTURE_TEST_SUITE(simple_selection_model_test, simple_selection_model_test_fixture)


/// Tests model ctor
BOOST_AUTO_TEST_CASE(ctor) {
    BOOST_CHECK_EQUAL(selection.pos().line, 0);
    BOOST_CHECK_EQUAL(selection.pos().column, 0);
    BOOST_CHECK_EQUAL(selection.anchor_pos().line, 0);
    BOOST_CHECK_EQUAL(selection.anchor_pos().column, 0);
    BOOST_CHECK(std::ranges::empty(selection.selections()));
}


/// Tests setting position and selection anchor values
BOOST_AUTO_TEST_CASE(test_set_pos_move_anchor) {
    text.reset(L"this is simple text\nsecond line");

    int n_changed_called = 0;
    selection.changed.connect([this, &n_changed_called] {
        ++n_changed_called;
        BOOST_CHECK_EQUAL(selection.pos().line, 0);
        BOOST_CHECK_EQUAL(selection.pos().column, 1);
        BOOST_CHECK_EQUAL(selection.anchor_pos().line, 1);
        BOOST_CHECK_EQUAL(selection.anchor_pos().column, 5);
    });

    selection.set_pos_and_anchor({1, 5}, {0, 1});

    BOOST_CHECK_EQUAL(selection.pos().line, 0);
    BOOST_CHECK_EQUAL(selection.pos().column, 1);
    BOOST_CHECK_EQUAL(selection.anchor_pos().line, 1);
    BOOST_CHECK_EQUAL(selection.anchor_pos().column, 5);
    BOOST_CHECK_EQUAL(n_changed_called, 1);

    BOOST_REQUIRE_EQUAL(std::ranges::size(selection.selections()), 1);
    BOOST_CHECK_EQUAL(selection.selections()[0].start.line, 0);
    BOOST_CHECK_EQUAL(selection.selections()[0].start.column, 1);
    BOOST_CHECK_EQUAL(selection.selections()[0].end.line, 1);
    BOOST_CHECK_EQUAL(selection.selections()[0].end.column, 5);
}


/// Tests setting position and selection anchor to same value
BOOST_AUTO_TEST_CASE(set_pos_move_anchor_same) {
    text.reset(L"this is simple text\nsecond line");

    int n_changed_called = 0;
    selection.changed.connect([this, &n_changed_called] {
        ++n_changed_called;
        BOOST_CHECK_EQUAL(selection.pos().line, 1);
        BOOST_CHECK_EQUAL(selection.pos().column, 5);
        BOOST_CHECK_EQUAL(selection.anchor_pos().line, 1);
        BOOST_CHECK_EQUAL(selection.anchor_pos().column, 5);
    });

    selection.set_pos_move_anchor({1, 5});

    BOOST_CHECK_EQUAL(selection.pos().line, 1);
    BOOST_CHECK_EQUAL(selection.pos().column, 5);
    BOOST_CHECK_EQUAL(selection.anchor_pos().line, 1);
    BOOST_CHECK_EQUAL(selection.anchor_pos().column, 5);
    BOOST_CHECK_EQUAL(n_changed_called, 1);
    BOOST_CHECK(std::ranges::empty(selection.selections()));
}


/// Tests setting only current position
BOOST_AUTO_TEST_CASE(set_pos) {
    text.reset(L"this is simple text\nsecond line");
    selection.set_anchor_pos({1, 1});

    int n_changed_called = 0;
    selection.changed.connect([this, &n_changed_called] {
        ++n_changed_called;
        BOOST_CHECK_EQUAL(selection.pos().line, 1);
        BOOST_CHECK_EQUAL(selection.pos().column, 5);
        BOOST_CHECK_EQUAL(selection.anchor_pos().line, 1);
        BOOST_CHECK_EQUAL(selection.anchor_pos().column, 1);
    });

    selection.set_pos_keep_anchor({1, 5});

    BOOST_CHECK_EQUAL(selection.pos().line, 1);
    BOOST_CHECK_EQUAL(selection.pos().column, 5);
    BOOST_CHECK_EQUAL(selection.anchor_pos().line, 1);
    BOOST_CHECK_EQUAL(selection.anchor_pos().column, 1);
    BOOST_CHECK_EQUAL(n_changed_called, 1);
}


/// Tests setting only anchor position
BOOST_AUTO_TEST_CASE(test_set_anchor_pos) {
    text.reset(L"this is simple text\nsecond line");
    selection.set_pos_keep_anchor({1, 1});

    int n_changed_called = 0;
    selection.changed.connect([this, &n_changed_called] {
        ++n_changed_called;
        BOOST_CHECK_EQUAL(selection.pos().line, 1);
        BOOST_CHECK_EQUAL(selection.pos().column, 1);
        BOOST_CHECK_EQUAL(selection.anchor_pos().line, 1);
        BOOST_CHECK_EQUAL(selection.anchor_pos().column, 5);
    });

    selection.set_anchor_pos({1, 5});

    BOOST_CHECK_EQUAL(selection.pos().line, 1);
    BOOST_CHECK_EQUAL(selection.pos().column, 1);
    BOOST_CHECK_EQUAL(selection.anchor_pos().line, 1);
    BOOST_CHECK_EQUAL(selection.anchor_pos().column, 5);
    BOOST_CHECK_EQUAL(n_changed_called, 1);
}


/// Tests changing current position after inserting characters before the current position
BOOST_AUTO_TEST_CASE(insert) {
    text.reset(L"the old text\nsecond line");
    selection.set_pos_move_anchor({0, 10});

    int n_after_inserted_called = 0;
    text.after_inserted.connect([this, &n_after_inserted_called](auto && ...) {
        ++n_after_inserted_called;
    });

    int n_changed_calls = 0;
    selection.changed.connect([this, &n_changed_calls, &n_after_inserted_called] {
        ++n_changed_calls;
        BOOST_CHECK_EQUAL(n_after_inserted_called, 1);
        BOOST_CHECK_EQUAL(selection.pos().line, 2);
        BOOST_CHECK_EQUAL(selection.pos().column, 11);
        BOOST_CHECK_EQUAL(selection.anchor_pos().line, 2);
        BOOST_CHECK_EQUAL(selection.anchor_pos().column, 11);
    });

    text.insert({0, 4}, L"first\nsecond\nthird");

    BOOST_CHECK_EQUAL(selection.pos().line, 2);
    BOOST_CHECK_EQUAL(selection.pos().column, 11);
    BOOST_CHECK_EQUAL(selection.anchor_pos().line, 2);
    BOOST_CHECK_EQUAL(selection.anchor_pos().column, 11);

    BOOST_CHECK_EQUAL(n_after_inserted_called, 1);
    BOOST_CHECK_EQUAL(n_changed_calls, 1);
}


/// Tests changing position after inserting single character in current position
BOOST_AUTO_TEST_CASE(insert_single) {
    text.reset(L"the old text\nsecond line");
    selection.set_pos_move_anchor({0, 10});

    int n_after_inserted_called = 0;
    text.after_inserted.connect([this, &n_after_inserted_called](auto && ...) {
        ++n_after_inserted_called;
    });

    int n_changed_calls = 0;
    selection.changed.connect([this, &n_changed_calls, &n_after_inserted_called] {
        ++n_changed_calls;
        BOOST_CHECK_EQUAL(n_after_inserted_called, 1);
        BOOST_CHECK_EQUAL(selection.pos().line, 0);
        BOOST_CHECK_EQUAL(selection.pos().column, 11);
        BOOST_CHECK_EQUAL(selection.anchor_pos().line, 0);
        BOOST_CHECK_EQUAL(selection.anchor_pos().column, 11);
    });

    text.insert({0, 10}, L"x");

    BOOST_CHECK_EQUAL(selection.pos().line, 0);
    BOOST_CHECK_EQUAL(selection.pos().column, 11);
    BOOST_CHECK_EQUAL(selection.anchor_pos().line, 0);
    BOOST_CHECK_EQUAL(selection.anchor_pos().column, 11);

    BOOST_CHECK_EQUAL(n_after_inserted_called, 1);
    BOOST_CHECK_EQUAL(n_changed_calls, 1);
}


/// Tests preseving position after inserting single character in current position
BOOST_AUTO_TEST_CASE(insert_single_preserve) {
    text.reset(L"the old text\nsecond line");
    selection.set_pos_move_anchor({0, 10});
    selection.set_move_pos_after_insert(false);

    selection.changed.connect([this] {
        BOOST_CHECK(false);
    });

    text.insert({0, 10}, L"x");

    BOOST_CHECK_EQUAL(selection.pos().line, 0);
    BOOST_CHECK_EQUAL(selection.pos().column, 10);
    BOOST_CHECK_EQUAL(selection.anchor_pos().line, 0);
    BOOST_CHECK_EQUAL(selection.anchor_pos().column, 10);
}


/// Tests changing position after inserting characters at current position
BOOST_AUTO_TEST_CASE(insert_curr_pos) {
    text.reset(L"the old text\nsecond line");
    selection.set_pos_move_anchor({0, 10});

    int n_after_inserted_called = 0;
    text.after_inserted.connect([this, &n_after_inserted_called](auto && ...) {
        ++n_after_inserted_called;
    });

    int n_changed_calls = 0;
    selection.changed.connect([this, &n_changed_calls, &n_after_inserted_called] {
        ++n_changed_calls;
        BOOST_CHECK_EQUAL(n_after_inserted_called, 1);
        BOOST_CHECK_EQUAL(selection.pos().line, 2);
        BOOST_CHECK_EQUAL(selection.pos().column, 5);
        BOOST_CHECK_EQUAL(selection.anchor_pos().line, 2);
        BOOST_CHECK_EQUAL(selection.anchor_pos().column, 5);
    });

    text.insert({0, 10}, L"first\nsecond\nthird");

    BOOST_CHECK_EQUAL(selection.pos().line, 2);
    BOOST_CHECK_EQUAL(selection.pos().column, 5);
    BOOST_CHECK_EQUAL(selection.anchor_pos().line, 2);
    BOOST_CHECK_EQUAL(selection.anchor_pos().column, 5);

    BOOST_CHECK_EQUAL(n_after_inserted_called, 1);
    BOOST_CHECK_EQUAL(n_changed_calls, 1);
}


/// Tests preserving position after inserting characters at current position
BOOST_AUTO_TEST_CASE(insert_curr_pos_preserve) {
    text.reset(L"the old text\nsecond line");
    selection.set_pos_move_anchor({0, 10});
    selection.set_move_pos_after_insert(false);

    selection.changed.connect([] {
        BOOST_CHECK(false);
    });

    text.insert({0, 10}, L"first\nsecond\nthird");

    BOOST_CHECK_EQUAL(selection.pos().line, 0);
    BOOST_CHECK_EQUAL(selection.pos().column, 10);
    BOOST_CHECK_EQUAL(selection.anchor_pos().line, 0);
    BOOST_CHECK_EQUAL(selection.anchor_pos().column, 10);
}


/// Tests changing position after deleting current selection
BOOST_AUTO_TEST_CASE(erase_selection) {
    text.reset(L"the old text\nsecond line");
    selection.set_pos_and_anchor({0, 10}, {1, 3});

    int n_before_removed_called = 0;
    text.before_erased.connect([this, &n_before_removed_called](auto && ...) {
        ++n_before_removed_called;
    });

    int n_changed_calls = 0;
    selection.changed.connect([this, &n_changed_calls, &n_before_removed_called] {
        ++n_changed_calls;
        BOOST_CHECK_EQUAL(n_before_removed_called, 1);
        BOOST_CHECK_EQUAL(selection.pos().line, 0);
        BOOST_CHECK_EQUAL(selection.pos().column, 10);
        BOOST_CHECK_EQUAL(selection.anchor_pos().line, 0);
        BOOST_CHECK_EQUAL(selection.anchor_pos().column, 10);
    });

    text.erase({{0, 10}, {1, 3}});

    BOOST_CHECK_EQUAL(selection.pos().line, 0);
    BOOST_CHECK_EQUAL(selection.pos().column, 10);
    BOOST_CHECK_EQUAL(selection.anchor_pos().line, 0);
    BOOST_CHECK_EQUAL(selection.anchor_pos().column, 10);

    BOOST_CHECK_EQUAL(n_before_removed_called, 1);
    BOOST_CHECK_EQUAL(n_changed_calls, 1);
}


/// Tests changing position after deleting characters with current position within delete range
BOOST_AUTO_TEST_CASE(erase_pos_in_range) {
    text.reset(L"the old text\nsecond line\nthird line");
    selection.set_pos_move_anchor({1, 4});

    int n_before_removed_called = 0;
    text.before_erased.connect([this, &n_before_removed_called](auto && ...) {
        ++n_before_removed_called;
    });

    int n_changed_calls = 0;
    selection.changed.connect([this, &n_changed_calls, &n_before_removed_called] {
        ++n_changed_calls;
        BOOST_CHECK_EQUAL(n_before_removed_called, 1);
        BOOST_CHECK_EQUAL(selection.pos().line, 0);
        BOOST_CHECK_EQUAL(selection.pos().column, 6);
        BOOST_CHECK_EQUAL(selection.anchor_pos().line, 0);
        BOOST_CHECK_EQUAL(selection.anchor_pos().column, 6);
    });

    text.erase({{0, 6}, {2, 2}});

    BOOST_CHECK_EQUAL(selection.pos().line, 0);
    BOOST_CHECK_EQUAL(selection.pos().column, 6);
    BOOST_CHECK_EQUAL(selection.anchor_pos().line, 0);
    BOOST_CHECK_EQUAL(selection.anchor_pos().column, 6);

    BOOST_CHECK_EQUAL(n_before_removed_called, 1);
    BOOST_CHECK_EQUAL(n_changed_calls, 1);
}


/// Tests changing position after deleting characters with current position after delete range
BOOST_AUTO_TEST_CASE(erase_pos_after_range_same_line) {
    text.reset(L"the old text\nsecond line\nthird line");
    selection.set_pos_move_anchor({2, 4});

    int n_changed_calls = 0;
    selection.changed.connect([this, &n_changed_calls] {
        ++n_changed_calls;
        BOOST_CHECK_EQUAL(selection.pos().line, 0);
        BOOST_CHECK_EQUAL(selection.pos().column, 8);
        BOOST_CHECK_EQUAL(selection.anchor_pos().line, 0);
        BOOST_CHECK_EQUAL(selection.anchor_pos().column, 8);
    });

    text.erase({{0, 6}, {2, 2}});

    BOOST_CHECK_EQUAL(selection.pos().line, 0);
    BOOST_CHECK_EQUAL(selection.pos().column, 8);
    BOOST_CHECK_EQUAL(selection.anchor_pos().line, 0);
    BOOST_CHECK_EQUAL(selection.anchor_pos().column, 8);

    BOOST_CHECK_EQUAL(n_changed_calls, 1);
}


/// Tests changing position after deleting characters with current position after delete range
/// and not on the last line of delete range
BOOST_AUTO_TEST_CASE(erase_pos_after_range) {
    text.reset(L"the old text\nsecond line\nthird line");
    selection.set_pos_keep_anchor({2, 4});

    int n_changed_calls = 0;
    selection.changed.connect([this, &n_changed_calls] {
        ++n_changed_calls;
        BOOST_CHECK_EQUAL(selection.pos().line, 1);
        BOOST_CHECK_EQUAL(selection.pos().column, 4);
        BOOST_CHECK_EQUAL(selection.anchor_pos().line, 1);
        BOOST_CHECK_EQUAL(selection.anchor_pos().column, 4);
    });

    text.erase({{0, 6}, {1, 2}});

    BOOST_CHECK_EQUAL(selection.pos().line, 1);
    BOOST_CHECK_EQUAL(selection.pos().column, 4);
    BOOST_CHECK_EQUAL(selection.anchor_pos().line, 1);
    BOOST_CHECK_EQUAL(selection.anchor_pos().column, 4);

    BOOST_CHECK_EQUAL(n_changed_calls, 1);
}


/// Tests erasing range of characters so that new position is invalid
/// before characters are removed
BOOST_AUTO_TEST_CASE(erase_invalid_pos_before_erase) {
    text.reset(L"the old text\n\nthird line");
    selection.set_pos_move_anchor({2, 4});

    int n_changed_calls = 0;
    selection.changed.connect([this, &n_changed_calls] {
        ++n_changed_calls;
        BOOST_CHECK_EQUAL(selection.pos().line, 1);
        BOOST_CHECK_EQUAL(selection.pos().column, 4);
        BOOST_CHECK_EQUAL(selection.anchor_pos().line, 1);
        BOOST_CHECK_EQUAL(selection.anchor_pos().column, 4);
    });

    text.erase({{0, 12}, {1, 0}});

    BOOST_CHECK_EQUAL(selection.pos().line, 1);
    BOOST_CHECK_EQUAL(selection.pos().column, 4);
    BOOST_CHECK_EQUAL(selection.anchor_pos().line, 1);
    BOOST_CHECK_EQUAL(selection.anchor_pos().column, 4);

    BOOST_CHECK_EQUAL(n_changed_calls, 1);
}


BOOST_AUTO_TEST_SUITE_END()


}
