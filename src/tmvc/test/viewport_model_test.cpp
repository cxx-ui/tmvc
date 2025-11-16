// Copyright (c) 2025, Alexandr Esilevich
//
// Distributed under the Boost Software License.
// See accompanying file LICENSE for license information.
//

/// \file viewport_model_test.cpp
/// Contains unit tests for the basic_viewport_model class.

#include "../preserving_viewport_position_model.hpp"
#include "../simple_text_model.hpp"
#include "../single_selection_model.hpp"
#include "../impl/viewport_model.hpp"
#include <boost/test/unit_test.hpp>


namespace tmvc::impl::test {


struct viewport_model_test_fixture {
    using text_model_t = wsimple_text_model;
    using selection_model_t = single_selection_model<wsimple_text_model>;
    using viewport_position_model_t = preserving_viewport_position_model<text_model_t, selection_model_t>;
    using viewport_model_t = viewport_model<text_model_t, selection_model_t, viewport_position_model_t>;

    text_model_t text{L"This is\nsimple\n\ntext"};
    selection_model_t selection{text};
    viewport_position_model_t viewport_pos{text, selection};
    viewport_model_t viewport{text, selection, viewport_pos};

    viewport_model_test_fixture() {
        viewport.set_left_margin(1.0f);
        viewport.set_right_margin(1.0f);
        viewport.set_top_margin(1.0f);
    }
};


BOOST_FIXTURE_TEST_SUITE(viewport_model_test, viewport_model_test_fixture)


/// Tests constructor
BOOST_AUTO_TEST_CASE(ctor) {
    viewport_position_model_t viewport_pos2{text, selection};
    viewport_model_t viewport2{text, selection, viewport_pos2};

    BOOST_CHECK_EQUAL(viewport2.width(), 0.0f);
    BOOST_CHECK_EQUAL(viewport2.height(), 0.0f);
    BOOST_CHECK_EQUAL(viewport2.max_height(), 0.0f);
    BOOST_CHECK_EQUAL(viewport2.left_margin(), 0.0f);
    BOOST_CHECK_EQUAL(viewport2.right_margin(), 0.0f);
    BOOST_CHECK_EQUAL(viewport2.top_margin(), 0.0f);
    BOOST_CHECK_EQUAL(viewport2.vertical_pos(), 0);
    BOOST_CHECK_EQUAL(viewport2.horizontal_pos(), 0.0f);
    BOOST_CHECK_EQUAL(viewport2.max_vertical_pos(), 4);
    BOOST_CHECK_EQUAL(viewport2.max_horizontal_pos(), 0.0f);
    BOOST_CHECK_EQUAL(viewport2.visible_lines_count(), 0);
    BOOST_CHECK_EQUAL(viewport2.max_visible_lines_count(), 0);
}


/// Tests setting viewport size that can display entire text
BOOST_AUTO_TEST_CASE(set_size_all_text) {
    viewport.set_size(9.0f, 5.0f, 6.0f);

    BOOST_CHECK_EQUAL(viewport.width(), 9.0f);
    BOOST_CHECK_EQUAL(viewport.height(), 5.0f);
    BOOST_CHECK_EQUAL(viewport.max_height(), 6.0f);
    BOOST_CHECK_EQUAL(viewport.left_margin(), 1.0f);
    BOOST_CHECK_EQUAL(viewport.right_margin(), 1.0f);
    BOOST_CHECK_EQUAL(viewport.top_margin(), 1.0f);
    BOOST_CHECK_EQUAL(viewport.vertical_pos(), 0);
    BOOST_CHECK_EQUAL(viewport.horizontal_pos(), 0.0f);
    BOOST_CHECK_EQUAL(viewport.max_vertical_pos(), 0);
    BOOST_CHECK_EQUAL(viewport.max_horizontal_pos(), 0.0f);
    BOOST_CHECK_EQUAL(viewport.visible_lines_count(), 4);
    BOOST_CHECK_EQUAL(viewport.max_visible_lines_count(), 5);
}


/// Tests setting viewport size smaller than vertical text size
BOOST_AUTO_TEST_CASE(set_size_vertical_scroll) {
    viewport.set_size(9.0f, 4.5f, 6.5f);

    BOOST_CHECK_EQUAL(viewport.width(), 9.0f);
    BOOST_CHECK_EQUAL(viewport.height(), 4.5f);
    BOOST_CHECK_EQUAL(viewport.max_height(), 6.5f);
    BOOST_CHECK_EQUAL(viewport.left_margin(), 1.0f);
    BOOST_CHECK_EQUAL(viewport.right_margin(), 1.0f);
    BOOST_CHECK_EQUAL(viewport.top_margin(), 1.0f);
    BOOST_CHECK_EQUAL(viewport.vertical_pos(), 0);
    BOOST_CHECK_EQUAL(viewport.horizontal_pos(), 0.0f);
    BOOST_CHECK_EQUAL(viewport.max_vertical_pos(), 1);
    BOOST_CHECK_EQUAL(viewport.max_horizontal_pos(), 0.0f);
    BOOST_CHECK_EQUAL(viewport.visible_lines_count(), 3);
    BOOST_CHECK_EQUAL(viewport.max_visible_lines_count(), 6);
}


/// Tests setting viewport size smaller than horizontal text size
BOOST_AUTO_TEST_CASE(set_size_horizontal_scroll) {
    viewport.set_size(8.0f, 5.0f, 6.0f);

    BOOST_CHECK_EQUAL(viewport.width(), 8.0f);
    BOOST_CHECK_EQUAL(viewport.height(), 5.0f);
    BOOST_CHECK_EQUAL(viewport.max_height(), 6.0f);
    BOOST_CHECK_EQUAL(viewport.left_margin(), 1.0f);
    BOOST_CHECK_EQUAL(viewport.right_margin(), 1.0f);
    BOOST_CHECK_EQUAL(viewport.top_margin(), 1.0f);
    BOOST_CHECK_EQUAL(viewport.vertical_pos(), 0);
    BOOST_CHECK_EQUAL(viewport.horizontal_pos(), 0.0f);
    BOOST_CHECK_EQUAL(viewport.max_vertical_pos(), 0);
    BOOST_CHECK_EQUAL(viewport.max_horizontal_pos(), 1.0f);
    BOOST_CHECK_EQUAL(viewport.visible_lines_count(), 4);
    BOOST_CHECK_EQUAL(viewport.max_visible_lines_count(), 5);
}


/// Tests that viewport makes position visible after position change to a value after current viewport
BOOST_AUTO_TEST_CASE(position_vertical_change_visible_after) {
    viewport.set_size(9.0f, 3.5f, 5.5f);
    BOOST_CHECK_EQUAL(viewport.max_vertical_pos(), 2);

    selection.set_pos_keep_anchor({3, 0});

    BOOST_CHECK_EQUAL(viewport.vertical_pos(), 2);
}


/// Tests that viewport makes position visible after position change to a value before current viewport
BOOST_AUTO_TEST_CASE(position_vertical_change_visible_before) {
    viewport.set_size(9.0f, 3.5f, 5.5f);
    BOOST_CHECK_EQUAL(viewport.max_vertical_pos(), 2);

    viewport_pos.set_pos(1);
    selection.set_pos_keep_anchor({0, 1});

    BOOST_CHECK_EQUAL(viewport.vertical_pos(), 0);
}


/// Tests that viewport makes horizontal position visible after selection position is changed
/// to a value after current viewport
BOOST_AUTO_TEST_CASE(position_horizontal_change_visible_after) {
    assign(text, L"This is really very long line");
    viewport.set_size(9.0f, 9.0f, 10.0f);

    BOOST_CHECK_EQUAL(viewport.max_horizontal_pos(), 22);

    selection.set_pos_keep_anchor({0, 10});

    BOOST_CHECK_EQUAL(viewport.horizontal_pos(), 6.5f);
}


/// Tests getting text position
BOOST_AUTO_TEST_CASE(get_text_pos) {
    auto pos = viewport.text_pos(5, 2.5, false);
    BOOST_CHECK_EQUAL(pos.line, 1);
    BOOST_CHECK_EQUAL(pos.column, 4);
}


/// Tests getting text position with overwrite flag
BOOST_AUTO_TEST_CASE(get_text_pos_overwrite) {
    auto pos = viewport.text_pos(5, 2.5, true);
    BOOST_CHECK_EQUAL(pos.line, 1);
    BOOST_CHECK_EQUAL(pos.column, 4);
}


/// Tests preserving viewport position when removing characters before current selection
BOOST_AUTO_TEST_CASE(remove_chars_before_preserve_pos) {
    assign(text, L"line one\nline two\nline three\nline four\nline five\nline six\nline seven\nline eight");
    viewport.set_size(10.0, 4.0, 5.0);

    viewport_pos.set_pos(3);
    selection.set_pos_and_anchor({4, 1}, {4, 1});

    // removing lines 2 and 3
    text.erase({{1, 0}, {3, 0}});

    BOOST_CHECK_EQUAL(viewport.vertical_pos(), 1);
    BOOST_CHECK_EQUAL(selection.pos().line, 2);
    BOOST_CHECK_EQUAL(selection.pos().column, 1);
}


/// Tests preserving viewport position when inserting characters before current selection
BOOST_AUTO_TEST_CASE(insert_chars_before_preserve_pos) {
    assign(text, L"line one\nline four\nline five\nline six\nline seven\nline eight");
    viewport.set_size(10.0, 3.0, 4.0);

    viewport_pos.set_pos(1);
    selection.set_pos_and_anchor({2, 1}, {2, 1});

    text.insert({1, 0}, L"line two\nline three\n");

    BOOST_CHECK_EQUAL(viewport.vertical_pos(), 3);
    BOOST_CHECK_EQUAL(selection.pos().line, 4);
    BOOST_CHECK_EQUAL(selection.pos().column, 1);
}


BOOST_AUTO_TEST_SUITE_END()


};
