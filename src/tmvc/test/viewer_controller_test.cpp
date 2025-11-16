// Copyright (c) 2025, Alexandr Esilevich
//
// Distributed under the Boost Software License.
// See accompanying file LICENSE for license information.
//

/// \file viewer_controller_test.cpp
/// Contains unit tests for single_selection_controller class.

#include "../simple_text_model.hpp"
#include "../single_selection_controller.hpp"
#include "../single_selection_model.hpp"
#include <boost/test/unit_test.hpp>


namespace tmvc::test {


struct viewer_controller_test_fixture {
    wsimple_text_model text;
    single_selection_model<wsimple_text_model> selection{text};
    single_selection_controller<wsimple_text_model> controller{text, selection};
};


BOOST_FIXTURE_TEST_SUITE(viewer_controller_test, viewer_controller_test_fixture)


/// Tests setting position
BOOST_AUTO_TEST_CASE(set_pos) {
    text.reset(L"doc text");

    int n_pos_changed_calls = 0;
    selection.changed.connect([&n_pos_changed_calls, this] {
        ++n_pos_changed_calls;
        BOOST_CHECK_EQUAL(selection.pos().line, 0);
        BOOST_CHECK_EQUAL(selection.pos().column, 3);
        BOOST_CHECK_EQUAL(selection.anchor_pos().line, 0);
        BOOST_CHECK_EQUAL(selection.anchor_pos().column, 3);
    });

    controller.select_text({0, 3}, {0, 3});
    BOOST_CHECK_EQUAL(selection.pos().line, 0);
    BOOST_CHECK_EQUAL(selection.pos().column, 3);
    BOOST_CHECK_EQUAL(selection.anchor_pos().line, 0);
    BOOST_CHECK_EQUAL(selection.anchor_pos().column, 3);

    BOOST_CHECK_EQUAL(n_pos_changed_calls, 1);
}


/// Tests setting position with keeping anchor
BOOST_AUTO_TEST_CASE(set_pos_keep_anchor_test) {
    text.reset(L"doc text\nsecond line");
    controller.select_text({0, 3}, {0, 3});

    int n_pos_changed_calls = 0;
    selection.changed.connect([&n_pos_changed_calls, this] {
        ++n_pos_changed_calls;
        BOOST_CHECK_EQUAL(selection.pos().line, 1);
        BOOST_CHECK_EQUAL(selection.pos().column, 2);
        BOOST_CHECK_EQUAL(selection.anchor_pos().line, 0);
        BOOST_CHECK_EQUAL(selection.anchor_pos().column, 3);
    });

    controller.select_text({0, 3}, {1, 2});
    BOOST_CHECK_EQUAL(selection.pos().line, 1);
    BOOST_CHECK_EQUAL(selection.pos().column, 2);
    BOOST_CHECK_EQUAL(selection.anchor_pos().line, 0);
    BOOST_CHECK_EQUAL(selection.anchor_pos().column, 3);

    BOOST_CHECK_EQUAL(n_pos_changed_calls, 1);
}


/// Tests performing left action with no selection
BOOST_AUTO_TEST_CASE(do_left_no_selection) {
    text.reset(L"first line\nsecond line");
    controller.select_text({0, 5}, {0, 5});
    controller.do_left(false, false);

    BOOST_CHECK_EQUAL(selection.pos().line, 0);
    BOOST_CHECK_EQUAL(selection.pos().column, 4);
    BOOST_CHECK_EQUAL(selection.anchor_pos().line, 0);
    BOOST_CHECK_EQUAL(selection.anchor_pos().column, 4);
}


/// Tests performing left action at beginning of line
BOOST_AUTO_TEST_CASE(do_left_line_begin) {
    text.reset(L"first line\nsecond line");
    controller.select_text({1, 0}, {1, 0});
    controller.do_left(false, false);

    BOOST_CHECK_EQUAL(selection.pos().line, 0);
    BOOST_CHECK_EQUAL(selection.pos().column, 10);
    BOOST_CHECK_EQUAL(selection.anchor_pos().line, 0);
    BOOST_CHECK_EQUAL(selection.anchor_pos().column, 10);
}


/// Tests performing left action with keeping anchor
BOOST_AUTO_TEST_CASE(do_left_keep_anchor) {
    text.reset(L"first line\nsecond line");
    controller.select_text({0, 5}, {0, 4});
    controller.do_left(false, true);

    BOOST_CHECK_EQUAL(selection.pos().line, 0);
    BOOST_CHECK_EQUAL(selection.pos().column, 3);
    BOOST_CHECK_EQUAL(selection.anchor_pos().line, 0);
    BOOST_CHECK_EQUAL(selection.anchor_pos().column, 5);
}


/// Tests performing left action with stop keeping anchor
BOOST_AUTO_TEST_CASE(do_left_stop_keep_anchor) {
    text.reset(L"first line\nsecond line");
    controller.select_text({0, 5}, {0, 4});
    controller.do_left(false, false);

    BOOST_CHECK_EQUAL(selection.pos().line, 0);
    BOOST_CHECK_EQUAL(selection.pos().column, 3);
    BOOST_CHECK_EQUAL(selection.anchor_pos().line, 0);
    BOOST_CHECK_EQUAL(selection.anchor_pos().column, 3);
}


/// Tests performing right action with no selection
BOOST_AUTO_TEST_CASE(do_right_no_selection) {
    text.reset(L"first line\nsecond line");
    controller.select_text({0, 5}, {0, 5});
    controller.do_right(false, false);

    BOOST_CHECK_EQUAL(selection.pos().line, 0);
    BOOST_CHECK_EQUAL(selection.pos().column, 6);
    BOOST_CHECK_EQUAL(selection.anchor_pos().line, 0);
    BOOST_CHECK_EQUAL(selection.anchor_pos().column, 6);
}


/// Tests performing right action at end of line
BOOST_AUTO_TEST_CASE(do_right_line_begin) {
    text.reset(L"first line\nsecond line");
    controller.select_text({0, 10}, {0, 10});
    controller.do_right(false, false);

    BOOST_CHECK_EQUAL(selection.pos().line, 1);
    BOOST_CHECK_EQUAL(selection.pos().column, 0);
    BOOST_CHECK_EQUAL(selection.anchor_pos().line, 1);
    BOOST_CHECK_EQUAL(selection.anchor_pos().column, 0);
}


/// Tests performing right action with keeping anchor
BOOST_AUTO_TEST_CASE(do_right_keep_anchor) {
    text.reset(L"first line\nsecond line");
    controller.select_text({0, 5}, {0, 6});
    controller.do_right(false, true);

    BOOST_CHECK_EQUAL(selection.pos().line, 0);
    BOOST_CHECK_EQUAL(selection.pos().column, 7);
    BOOST_CHECK_EQUAL(selection.anchor_pos().line, 0);
    BOOST_CHECK_EQUAL(selection.anchor_pos().column, 5);
}


/// Tests performing right action with stop keeping anchor
BOOST_AUTO_TEST_CASE(do_right_stop_keep_anchor) {
    text.reset(L"first line\nsecond line");
    controller.select_text({0, 5}, {0, 6});
    controller.do_right(false, false);

    BOOST_CHECK_EQUAL(selection.pos().line, 0);
    BOOST_CHECK_EQUAL(selection.pos().column, 7);
    BOOST_CHECK_EQUAL(selection.anchor_pos().line, 0);
    BOOST_CHECK_EQUAL(selection.anchor_pos().column, 7);
}


/// Tests performing simple up action
BOOST_AUTO_TEST_CASE(do_up_simple) {
    text.reset(L"first line\nsecond\nthird line");
    controller.select_text({1, 2}, {1, 2});
    controller.do_up(false, false);

    BOOST_CHECK_EQUAL(selection.pos().line, 0);
    BOOST_CHECK_EQUAL(selection.pos().column, 2);
    BOOST_CHECK_EQUAL(selection.anchor_pos().line, 0);
    BOOST_CHECK_EQUAL(selection.anchor_pos().column, 2);
}


/// Tests performing up action on the first line
BOOST_AUTO_TEST_CASE(do_up_first_line) {
    text.reset(L"first line\nsecond\nthird line");
    controller.select_text({0, 2}, {0, 2});
    controller.do_up(false, false);

    BOOST_CHECK_EQUAL(selection.pos().line, 0);
    BOOST_CHECK_EQUAL(selection.pos().column, 2);
    BOOST_CHECK_EQUAL(selection.anchor_pos().line, 0);
    BOOST_CHECK_EQUAL(selection.anchor_pos().column, 2);
}


/// Tests performing up action with saving anchor
BOOST_AUTO_TEST_CASE(do_up_keep_anchor) {
    text.reset(L"first line\nsecond\nthird line");
    controller.select_text({1, 3}, {1, 2});
    controller.do_up(false, true);

    BOOST_CHECK_EQUAL(selection.pos().line, 0);
    BOOST_CHECK_EQUAL(selection.pos().column, 2);
    BOOST_CHECK_EQUAL(selection.anchor_pos().line, 1);
    BOOST_CHECK_EQUAL(selection.anchor_pos().column, 3);
}


/// Tests saving offset when moving up through short lines
BOOST_AUTO_TEST_CASE(do_up_save_offset) {
    text.reset(L"first line\nsecond\nthird line");
    controller.select_text({2, 7}, {2, 7});

    controller.do_up(false, false);
    BOOST_CHECK_EQUAL(selection.pos().line, 1);
    BOOST_CHECK_EQUAL(selection.pos().column, 6);
    BOOST_CHECK_EQUAL(selection.anchor_pos().line, 1);
    BOOST_CHECK_EQUAL(selection.anchor_pos().column, 6);

    controller.do_up(false, false);
    BOOST_CHECK_EQUAL(selection.pos().line, 0);
    BOOST_CHECK_EQUAL(selection.pos().column, 7);
    BOOST_CHECK_EQUAL(selection.anchor_pos().line, 0);
    BOOST_CHECK_EQUAL(selection.anchor_pos().column, 7);
}


/// Tests saving offset when moving up and pressing left on short line
BOOST_AUTO_TEST_CASE(do_up_cancel_save_offset) {
    text.reset(L"first line\nsecond\nthird line");
    controller.select_text({2, 7}, {2, 7});

    controller.do_up(false, false);
    BOOST_CHECK_EQUAL(selection.pos().line, 1);
    BOOST_CHECK_EQUAL(selection.pos().column, 6);
    BOOST_CHECK_EQUAL(selection.anchor_pos().line, 1);
    BOOST_CHECK_EQUAL(selection.anchor_pos().column, 6);

    controller.do_left(false, false);
    BOOST_CHECK_EQUAL(selection.pos().line, 1);
    BOOST_CHECK_EQUAL(selection.pos().column, 5);
    BOOST_CHECK_EQUAL(selection.anchor_pos().line, 1);
    BOOST_CHECK_EQUAL(selection.anchor_pos().column, 5);

    controller.do_up(false, false);
    BOOST_CHECK_EQUAL(selection.pos().line, 0);
    BOOST_CHECK_EQUAL(selection.pos().column, 5);
    BOOST_CHECK_EQUAL(selection.anchor_pos().line, 0);
    BOOST_CHECK_EQUAL(selection.anchor_pos().column, 5);
}


/// Tests performing simple down action
BOOST_AUTO_TEST_CASE(do_down_simple) {
    text.reset(L"first line\nsecond\nthird line");
    controller.select_text({1, 2}, {1, 2});
    controller.do_down(false, false);

    BOOST_CHECK_EQUAL(selection.pos().line, 2);
    BOOST_CHECK_EQUAL(selection.pos().column, 2);
    BOOST_CHECK_EQUAL(selection.anchor_pos().line, 2);
    BOOST_CHECK_EQUAL(selection.anchor_pos().column, 2);
}


/// Tests performing down action on the last line
BOOST_AUTO_TEST_CASE(do_down_last_line) {
    text.reset(L"first line\nsecond\nthird line");
    controller.select_text({2, 2}, {2, 2});
    controller.do_down(false, false);

    BOOST_CHECK_EQUAL(selection.pos().line, 2);
    BOOST_CHECK_EQUAL(selection.pos().column, 2);
    BOOST_CHECK_EQUAL(selection.anchor_pos().line, 2);
    BOOST_CHECK_EQUAL(selection.anchor_pos().column, 2);
}


/// Tests performing down action with saving anchor
BOOST_AUTO_TEST_CASE(do_down_keep_anchor) {
    text.reset(L"first line\nsecond\nthird line");
    controller.select_text({1, 3}, {1, 4});
    controller.do_down(false, true);

    BOOST_CHECK_EQUAL(selection.pos().line, 2);
    BOOST_CHECK_EQUAL(selection.pos().column, 4);
    BOOST_CHECK_EQUAL(selection.anchor_pos().line, 1);
    BOOST_CHECK_EQUAL(selection.anchor_pos().column, 3);
}


/// Tests saving offset when moving down through short lines
BOOST_AUTO_TEST_CASE(do_down_save_offset) {
    text.reset(L"first line\nsecond\nthird line");
    controller.select_text({0, 7}, {0, 7});

    controller.do_down(false, false);
    BOOST_CHECK_EQUAL(selection.pos().line, 1);
    BOOST_CHECK_EQUAL(selection.pos().column, 6);
    BOOST_CHECK_EQUAL(selection.anchor_pos().line, 1);
    BOOST_CHECK_EQUAL(selection.anchor_pos().column, 6);

    controller.do_down(false, false);
    BOOST_CHECK_EQUAL(selection.pos().line, 2);
    BOOST_CHECK_EQUAL(selection.pos().column, 7);
    BOOST_CHECK_EQUAL(selection.anchor_pos().line, 2);
    BOOST_CHECK_EQUAL(selection.anchor_pos().column, 7);
}


/// Tests saving offset when moving down and pressing left on short line
BOOST_AUTO_TEST_CASE(do_down_cancel_save_offset) {
    text.reset(L"first line\nsecond\nthird line");
    controller.select_text({0, 7}, {0, 7});

    controller.do_down(false, false);
    BOOST_CHECK_EQUAL(selection.pos().line, 1);
    BOOST_CHECK_EQUAL(selection.pos().column, 6);
    BOOST_CHECK_EQUAL(selection.anchor_pos().line, 1);
    BOOST_CHECK_EQUAL(selection.anchor_pos().column, 6);

    controller.do_left(false, false);
    BOOST_CHECK_EQUAL(selection.pos().line, 1);
    BOOST_CHECK_EQUAL(selection.pos().column, 5);
    BOOST_CHECK_EQUAL(selection.anchor_pos().line, 1);
    BOOST_CHECK_EQUAL(selection.anchor_pos().column, 5);

    controller.do_down(false, false);
    BOOST_CHECK_EQUAL(selection.pos().line, 2);
    BOOST_CHECK_EQUAL(selection.pos().column, 5);
    BOOST_CHECK_EQUAL(selection.anchor_pos().line, 2);
    BOOST_CHECK_EQUAL(selection.anchor_pos().column, 5);
}


/// Tests home action
BOOST_AUTO_TEST_CASE(do_home) {
    text.reset(L"first line\nsecond\nthird line");
    controller.select_text({0, 2}, {1, 3});
    controller.do_home(false, false);

    BOOST_CHECK(string(text) == L"first line\nsecond\nthird line");
    BOOST_CHECK_EQUAL(selection.pos().line, 1);
    BOOST_CHECK_EQUAL(selection.pos().column, 0);
    BOOST_CHECK_EQUAL(selection.anchor_pos().line, 1);
    BOOST_CHECK_EQUAL(selection.anchor_pos().column, 0);
}


/// Tests end action
BOOST_AUTO_TEST_CASE(do_end) {
    text.reset(L"first line\nsecond\nthird line");
    controller.select_text({0, 2}, {1, 3});
    controller.do_end(false, false);

    BOOST_CHECK(string(text) == L"first line\nsecond\nthird line");
    BOOST_CHECK_EQUAL(selection.pos().line, 1);
    BOOST_CHECK_EQUAL(selection.pos().column, 6);
    BOOST_CHECK_EQUAL(selection.anchor_pos().line, 1);
    BOOST_CHECK_EQUAL(selection.anchor_pos().column, 6);
}


/// Tests home action with shift
BOOST_AUTO_TEST_CASE(do_home_shift) {
    text.reset(L"first line\nsecond\nthird line");
    controller.select_text({0, 2}, {1, 3});
    controller.do_home(false, true);

    BOOST_CHECK(string(text) == L"first line\nsecond\nthird line");
    BOOST_CHECK_EQUAL(selection.pos().line, 1);
    BOOST_CHECK_EQUAL(selection.pos().column, 0);
    BOOST_CHECK_EQUAL(selection.anchor_pos().line, 0);
    BOOST_CHECK_EQUAL(selection.anchor_pos().column, 2);
}


/// Tests end action with shift
BOOST_AUTO_TEST_CASE(do_end_shift) {
    text.reset(L"first line\nsecond\nthird line");
    controller.select_text({0, 2}, {1, 3});
    controller.do_end(false, true);

    BOOST_CHECK(string(text) == L"first line\nsecond\nthird line");
    BOOST_CHECK_EQUAL(selection.pos().line, 1);
    BOOST_CHECK_EQUAL(selection.pos().column, 6);
    BOOST_CHECK_EQUAL(selection.anchor_pos().line, 0);
    BOOST_CHECK_EQUAL(selection.anchor_pos().column, 2);
}


/// Tests moving to next word
BOOST_AUTO_TEST_CASE(move_next_word) {
    auto txt = L"line of text ((*sd\n line2";
    text.reset(txt);
    controller.select_text({0, 0}, {0, 0});
    controller.do_right(true, false);

    BOOST_CHECK(string(text) == txt);
    BOOST_CHECK_EQUAL(selection.pos().line, 0);
    BOOST_CHECK_EQUAL(selection.pos().column, 5);
    BOOST_CHECK_EQUAL(selection.anchor_pos().line, 0);
    BOOST_CHECK_EQUAL(selection.anchor_pos().column, 5);
}


/// Tests moving to next word on space
BOOST_AUTO_TEST_CASE(move_next_word_space) {
    auto txt = L"line of text ((*sd\n line2";
    text.reset(txt);
    controller.select_text({0, 4}, {0, 4});
    controller.do_right(true, false);

    BOOST_CHECK(string(text) == txt);
    BOOST_CHECK_EQUAL(selection.pos().line, 0);
    BOOST_CHECK_EQUAL(selection.pos().column, 5);
    BOOST_CHECK_EQUAL(selection.anchor_pos().line, 0);
    BOOST_CHECK_EQUAL(selection.anchor_pos().column, 5);
}


/// Tests moving to next word with non-alnum characters
BOOST_AUTO_TEST_CASE(move_next_word_no_alnum) {
    auto txt = L"line of text ((*sd\n line2";
    text.reset(txt);
    controller.select_text({0, 8}, {0, 8});
    controller.do_right(true, false);

    BOOST_CHECK(string(text) == txt);
    BOOST_CHECK_EQUAL(selection.pos().line, 0);
    BOOST_CHECK_EQUAL(selection.pos().column, 13);
    BOOST_CHECK_EQUAL(selection.anchor_pos().line, 0);
    BOOST_CHECK_EQUAL(selection.anchor_pos().column, 13);
}


/// Tests moving to next word via non-alnum characters
BOOST_AUTO_TEST_CASE(move_next_word_via_alnum) {
    auto txt = L"line of text ((*sd\n line2";
    text.reset(txt);
    controller.select_text({0, 13}, {0, 13});
    controller.do_right(true, false);

    BOOST_CHECK(string(text) == txt);
    BOOST_CHECK_EQUAL(selection.pos().line, 0);
    BOOST_CHECK_EQUAL(selection.pos().column, 16);
    BOOST_CHECK_EQUAL(selection.anchor_pos().line, 0);
    BOOST_CHECK_EQUAL(selection.anchor_pos().column, 16);
}


/// Tests moving to next word at EOL
BOOST_AUTO_TEST_CASE(move_next_word_eol) {
    auto txt = L"line of text ((*sd\n line2";
    text.reset(txt);
    controller.select_text({0, 16}, {0, 16});
    controller.do_right(true, false);

    BOOST_CHECK(string(text) == txt);
    BOOST_CHECK_EQUAL(selection.pos().line, 0);
    BOOST_CHECK_EQUAL(selection.pos().column, 18);
    BOOST_CHECK_EQUAL(selection.anchor_pos().line, 0);
    BOOST_CHECK_EQUAL(selection.anchor_pos().column, 18);
}


/// Tests moving to next word at new line
BOOST_AUTO_TEST_CASE(move_next_word_next_line) {
    auto txt = L"line of text ((*sd\n line2";
    text.reset(txt);
    controller.select_text({0, 18}, {0, 18});
    controller.do_right(true, false);

    BOOST_CHECK(string(text) == txt);
    BOOST_CHECK_EQUAL(selection.pos().line, 1);
    BOOST_CHECK_EQUAL(selection.pos().column, 0);
    BOOST_CHECK_EQUAL(selection.anchor_pos().line, 1);
    BOOST_CHECK_EQUAL(selection.anchor_pos().column, 0);
}


/// Tests moving to previous word
BOOST_AUTO_TEST_CASE(move_prev_word) {
    auto txt = L"line of text ((*sd\n line2";
    text.reset(txt);
    controller.select_text({0, 12}, {0, 12});
    controller.do_left(true, false);

    BOOST_CHECK(string(text) == txt);
    BOOST_CHECK_EQUAL(selection.pos().line, 0);
    BOOST_CHECK_EQUAL(selection.pos().column, 8);
    BOOST_CHECK_EQUAL(selection.anchor_pos().line, 0);
    BOOST_CHECK_EQUAL(selection.anchor_pos().column, 8);
}


/// Tests moving to previous word on space
BOOST_AUTO_TEST_CASE(move_prev_word_space) {
    auto txt = L"line of text ((*sd\n line2";
    text.reset(txt);
    controller.select_text({0, 8}, {0, 8});
    controller.do_left(true, false);

    BOOST_CHECK(string(text) == txt);
    BOOST_CHECK_EQUAL(selection.pos().line, 0);
    BOOST_CHECK_EQUAL(selection.pos().column, 5);
    BOOST_CHECK_EQUAL(selection.anchor_pos().line, 0);
    BOOST_CHECK_EQUAL(selection.anchor_pos().column, 5);
}


/// Tests moving to previous word with non-alnum characters
BOOST_AUTO_TEST_CASE(move_prev_word_no_alnum) {
    auto txt = L"line of text ((*sd\n line2";
    text.reset(txt);
    controller.select_text({0, 18}, {0, 18});
    controller.do_left(true, false);

    BOOST_CHECK(string(text) == txt);
    BOOST_CHECK_EQUAL(selection.pos().line, 0);
    BOOST_CHECK_EQUAL(selection.pos().column, 16);
    BOOST_CHECK_EQUAL(selection.anchor_pos().line, 0);
    BOOST_CHECK_EQUAL(selection.anchor_pos().column, 16);
}


/// Tests moving to previous word via non-alnum characters
BOOST_AUTO_TEST_CASE(move_prev_word_via_alnum) {
    auto txt = L"line of text ((*sd\n line2";
    text.reset(txt);
    controller.select_text({0, 16}, {0, 16});
    controller.do_left(true, false);

    BOOST_CHECK(string(text) == txt);
    BOOST_CHECK_EQUAL(selection.pos().line, 0);
    BOOST_CHECK_EQUAL(selection.pos().column, 13);
    BOOST_CHECK_EQUAL(selection.anchor_pos().line, 0);
    BOOST_CHECK_EQUAL(selection.anchor_pos().column, 13);
}


/// Tests moving to previous word at line start
BOOST_AUTO_TEST_CASE(move_prev_word_start) {
    auto txt = L"line of text ((*sd\n line2";
    text.reset(txt);
    controller.select_text({0, 4}, {0, 4});
    controller.do_left(true, false);

    BOOST_CHECK(string(text) == txt);
    BOOST_CHECK_EQUAL(selection.pos().line, 0);
    BOOST_CHECK_EQUAL(selection.pos().column, 0);
    BOOST_CHECK_EQUAL(selection.anchor_pos().line, 0);
    BOOST_CHECK_EQUAL(selection.anchor_pos().column, 0);
}


/// Tests moving to previous word at prev line
BOOST_AUTO_TEST_CASE(move_prev_word_prev_line) {
    auto txt = L"line of text ((*sd\n line2";
    text.reset(txt);
    controller.select_text({1, 0}, {1, 0});
    controller.do_left(true, false);

    BOOST_CHECK(string(text) == txt);
    BOOST_CHECK_EQUAL(selection.pos().line, 0);
    BOOST_CHECK_EQUAL(selection.pos().column, 18);
    BOOST_CHECK_EQUAL(selection.anchor_pos().line, 0);
    BOOST_CHECK_EQUAL(selection.anchor_pos().column, 18);
}


/// Tests selecting all text
BOOST_AUTO_TEST_CASE(select_all) {
    text.reset(L"first line\nvery long second line");
    controller.select_all();

    BOOST_CHECK_EQUAL(selection.pos().line, 1);
    BOOST_CHECK_EQUAL(selection.pos().column, 21);
    BOOST_CHECK_EQUAL(selection.anchor_pos().line, 0);
    BOOST_CHECK_EQUAL(selection.anchor_pos().column, 0);
}




BOOST_AUTO_TEST_SUITE_END()


}
