// Copyright (c) 2025, Alexandr Esilevich
//
// Distributed under the Boost Software License.
// See accompanying file LICENSE for license information.
//

/// \file key_controller_test.cpp
/// Contains unit tests for process_selection_key_event and process_edit_key_event.

#include "../impl/key_controller.hpp"
#include <boost/test/unit_test.hpp>
#include <optional>
#include <ranges>
#include <string>
#include <QGuiApplication>


namespace tmvc::qt::test {

namespace {

// ---------------------------------------------------------------------------
// Mock controllers
// ---------------------------------------------------------------------------

struct mock_selection_controller {
    using char_t = wchar_t;

    struct nav_args {
        bool ctrl = false;  ///<
        bool shift = false; ///<
        bool alt = false;   ///<
        std::optional<position> pos; ///<
    };

    bool copy_called = false;      ///<
    bool select_all_called = false; ///<

    std::optional<nav_args> last_left;  ///<
    std::optional<nav_args> last_right; ///<
    std::optional<nav_args> last_up;    ///<
    std::optional<nav_args> last_down;  ///<
    std::optional<nav_args> last_home;  ///<
    std::optional<nav_args> last_end;   ///<

    bool can_copy() { return true; }

    std::wstring copy() {
        copy_called = true;
        return {};
    }

    void select_all() { select_all_called = true; }

    void select_text(const position &, const position &) {}

    basic_context_menu<wchar_t> create_context_menu() { return {}; }

    void do_left(bool c, bool s, bool a, std::optional<position> p = {}) {
        last_left = {c, s, a, p};
    }

    void do_right(bool c, bool s, bool a, std::optional<position> p = {}) {
        last_right = {c, s, a, p};
    }

    void do_up(bool c, bool s, bool a, std::optional<position> p = {}) {
        last_up = {c, s, a, p};
    }

    void do_down(bool c, bool s, bool a, std::optional<position> p = {}) {
        last_down = {c, s, a, p};
    }

    void do_home(bool c, bool s, bool a) { last_home = {c, s, a, {}}; }

    void do_end(bool c, bool s, bool a) { last_end = {c, s, a, {}}; }

    std::optional<nav_args> last_page_up;   ///<
    std::optional<nav_args> last_page_down; ///<

    void do_page_up(bool c, bool s, bool a, std::optional<position> p = {}) {
        last_page_up = {c, s, a, p};
    }

    void do_page_down(bool c, bool s, bool a, std::optional<position> p = {}) {
        last_page_down = {c, s, a, p};
    }
};


struct mock_nav_controller: public mock_selection_controller {
    struct nav3_args {
        bool ctrl = false;  ///<
        bool alt = false;   ///<
        bool shift = false; ///<
        std::optional<position> pos; ///<
    };

    std::optional<nav3_args> last_move_next_word;  ///<
    std::optional<nav3_args> last_move_prev_word;  ///<
    std::optional<nav3_args> last_move_line_start; ///<
    std::optional<nav3_args> last_move_line_end;   ///<
    std::optional<nav3_args> last_move_prev_line;  ///<
    std::optional<nav3_args> last_move_next_line;  ///<
    std::optional<nav3_args> last_move_block_start; ///<
    std::optional<nav3_args> last_move_block_end;   ///<

    void move_next_word(bool c, bool a, bool s) {
        last_move_next_word = {c, a, s, {}};
    }

    void move_prev_word(bool c, bool a, bool s) {
        last_move_prev_word = {c, a, s, {}};
    }

    void move_line_start(bool c, bool a, bool s, const std::optional<position> & p) {
        last_move_line_start = {c, a, s, p};
    }

    void move_line_end(bool c, bool a, bool s, const std::optional<position> & p) {
        last_move_line_end = {c, a, s, p};
    }

    void move_prev_line(bool c, bool a, bool s, const std::optional<position> & p) {
        last_move_prev_line = {c, a, s, p};
    }

    void move_next_line(bool c, bool a, bool s, const std::optional<position> & p) {
        last_move_next_line = {c, a, s, p};
    }

    void move_block_start(bool c, bool a, bool s) {
        last_move_block_start = {c, a, s, {}};
    }

    void move_block_end(bool c, bool a, bool s) {
        last_move_block_end = {c, a, s, {}};
    }
};


struct mock_edit_controller: public mock_selection_controller {
    bool cut_called = false;  ///<
    bool undo_called = false; ///<
    bool redo_called = false; ///<

    std::optional<nav_args> last_backspace; ///<
    std::optional<nav_args> last_delete;    ///<
    std::optional<nav_args> last_enter;     ///<
    std::optional<nav_args> last_tab;       ///<

    std::optional<wchar_t> last_char; ///<
    std::wstring pasted;              ///<

    bool is_overwrite_mode() { return false; }
    bool can_cut() { return true; }
    bool can_delete() { return true; }
    bool can_undo() { return true; }
    bool can_redo() { return true; }

    std::wstring cut() { cut_called = true; return {}; }

    template <std::ranges::input_range R>
    void paste(R && chars) { pasted.assign(std::ranges::begin(chars), std::ranges::end(chars)); }

    void delete_() {}
    void undo() { undo_called = true; }
    void redo() { redo_called = true; }

    void do_backspace(bool c, bool s, bool a) { last_backspace = {c, s, a, {}}; }
    void do_delete(bool c, bool s, bool a) { last_delete = {c, s, a, {}}; }
    void do_enter(bool c, bool s, bool a) { last_enter = {c, s, a, {}}; }
    void do_tab(bool c, bool s, bool a) { last_tab = {c, s, a, {}}; }
    void do_insert(bool, bool, bool) {}
    void do_char(wchar_t ch) { last_char = ch; }
};


// ---------------------------------------------------------------------------
// Helpers
// ---------------------------------------------------------------------------

/// Creates a key press event for the first key combination of a standard sequence.
QKeyEvent make_standard_key_event(QKeySequence::StandardKey sk) {
    auto combo = QKeySequence{sk}[0];
    return QKeyEvent{QEvent::KeyPress, combo.key(), combo.keyboardModifiers()};
}


// ---------------------------------------------------------------------------
// Suite fixtures
// ---------------------------------------------------------------------------

/// Base fixture that owns the QGuiApplication for the lifetime of the test process.
/// Both suite fixtures inherit from this to share a single application instance.
struct qt_app_fixture {
    qt_app_fixture() {
        if (!app_) {
            qputenv("QT_QPA_PLATFORM", "offscreen");
            static int argc = 1;
            static char arg0[] = "test";
            static char * argv[] = {arg0, nullptr};
            app_ = new QGuiApplication{argc, argv};
        }
    }

    // raw pointer: QGuiApplication must outlive all Qt static objects destroyed on exit
    inline static QGuiApplication * app_ = nullptr; ///<
};


/// Fixture for process_selection_key_event tests. Provides a fresh mock controller
/// per test case.
struct selection_suite_fixture: public qt_app_fixture {
    mock_selection_controller ctrl; ///<
};


/// Fixture for process_edit_key_event tests. Provides a fresh mock controller
/// per test case.
struct edit_suite_fixture: public qt_app_fixture {
    mock_edit_controller ctrl; ///<
};


/// Fixture for navigation concept tests. Provides a fresh mock_nav_controller per test case.
struct nav_suite_fixture: public qt_app_fixture {
    mock_nav_controller ctrl; ///<
};

} // namespace


// ---------------------------------------------------------------------------
// process_selection_key_event
// ---------------------------------------------------------------------------

BOOST_FIXTURE_TEST_SUITE(process_selection_key_event_test, selection_suite_fixture)

BOOST_AUTO_TEST_CASE(copy_is_handled_and_calls_controller_copy) {
    auto e = make_standard_key_event(QKeySequence::Copy);
    BOOST_CHECK(impl::process_selection_key_event(ctrl, &e));
    BOOST_CHECK(ctrl.copy_called);
}

BOOST_AUTO_TEST_CASE(select_all_is_handled_and_calls_controller_select_all) {
    auto e = make_standard_key_event(QKeySequence::SelectAll);
    BOOST_CHECK(impl::process_selection_key_event(ctrl, &e));
    BOOST_CHECK(ctrl.select_all_called);
}

BOOST_AUTO_TEST_CASE(left_arrow_plain_is_handled) {
    QKeyEvent e{QEvent::KeyPress, Qt::Key_Left, Qt::NoModifier};
    BOOST_CHECK(impl::process_selection_key_event(ctrl, &e));
    BOOST_REQUIRE(ctrl.last_left.has_value());
    BOOST_CHECK(!ctrl.last_left->ctrl);
    BOOST_CHECK(!ctrl.last_left->shift);
}

BOOST_AUTO_TEST_CASE(left_arrow_ctrl_passes_ctrl_true) {
    QKeyEvent e{QEvent::KeyPress, Qt::Key_Left, Qt::ControlModifier};
    impl::process_selection_key_event(ctrl, &e);
    BOOST_REQUIRE(ctrl.last_left.has_value());
    BOOST_CHECK(ctrl.last_left->ctrl);
    BOOST_CHECK(!ctrl.last_left->shift);
}

BOOST_AUTO_TEST_CASE(left_arrow_shift_passes_shift_true) {
    QKeyEvent e{QEvent::KeyPress, Qt::Key_Left, Qt::ShiftModifier};
    impl::process_selection_key_event(ctrl, &e);
    BOOST_REQUIRE(ctrl.last_left.has_value());
    BOOST_CHECK(!ctrl.last_left->ctrl);
    BOOST_CHECK(ctrl.last_left->shift);
}

BOOST_AUTO_TEST_CASE(right_arrow_plain_is_handled) {
    QKeyEvent e{QEvent::KeyPress, Qt::Key_Right, Qt::NoModifier};
    BOOST_CHECK(impl::process_selection_key_event(ctrl, &e));
    BOOST_REQUIRE(ctrl.last_right.has_value());
    BOOST_CHECK(!ctrl.last_right->ctrl);
}

BOOST_AUTO_TEST_CASE(right_arrow_ctrl_passes_ctrl_true) {
    QKeyEvent e{QEvent::KeyPress, Qt::Key_Right, Qt::ControlModifier};
    impl::process_selection_key_event(ctrl, &e);
    BOOST_REQUIRE(ctrl.last_right.has_value());
    BOOST_CHECK(ctrl.last_right->ctrl);
}

BOOST_AUTO_TEST_CASE(up_arrow_plain_is_handled) {
    QKeyEvent e{QEvent::KeyPress, Qt::Key_Up, Qt::NoModifier};
    BOOST_CHECK(impl::process_selection_key_event(ctrl, &e));
    BOOST_REQUIRE(ctrl.last_up.has_value());
    BOOST_CHECK(!ctrl.last_up->ctrl);
}

BOOST_AUTO_TEST_CASE(down_arrow_plain_is_handled) {
    QKeyEvent e{QEvent::KeyPress, Qt::Key_Down, Qt::NoModifier};
    BOOST_CHECK(impl::process_selection_key_event(ctrl, &e));
    BOOST_REQUIRE(ctrl.last_down.has_value());
    BOOST_CHECK(!ctrl.last_down->ctrl);
}

BOOST_AUTO_TEST_CASE(home_plain_is_handled_with_ctrl_false) {
    QKeyEvent e{QEvent::KeyPress, Qt::Key_Home, Qt::NoModifier};
    BOOST_CHECK(impl::process_selection_key_event(ctrl, &e));
    BOOST_REQUIRE(ctrl.last_home.has_value());
    BOOST_CHECK(!ctrl.last_home->ctrl);
}

BOOST_AUTO_TEST_CASE(home_ctrl_passes_ctrl_true) {
    QKeyEvent e{QEvent::KeyPress, Qt::Key_Home, Qt::ControlModifier};
    impl::process_selection_key_event(ctrl, &e);
    BOOST_REQUIRE(ctrl.last_home.has_value());
    BOOST_CHECK(ctrl.last_home->ctrl);
}

BOOST_AUTO_TEST_CASE(end_plain_is_handled_with_ctrl_false) {
    QKeyEvent e{QEvent::KeyPress, Qt::Key_End, Qt::NoModifier};
    BOOST_CHECK(impl::process_selection_key_event(ctrl, &e));
    BOOST_REQUIRE(ctrl.last_end.has_value());
    BOOST_CHECK(!ctrl.last_end->ctrl);
}

BOOST_AUTO_TEST_CASE(end_ctrl_passes_ctrl_true) {
    QKeyEvent e{QEvent::KeyPress, Qt::Key_End, Qt::ControlModifier};
    impl::process_selection_key_event(ctrl, &e);
    BOOST_REQUIRE(ctrl.last_end.has_value());
    BOOST_CHECK(ctrl.last_end->ctrl);
}

BOOST_AUTO_TEST_CASE(page_up_is_handled_and_calls_do_page_up) {
    auto e = make_standard_key_event(QKeySequence::MoveToPreviousPage);
    BOOST_CHECK(impl::process_selection_key_event(ctrl, &e));
    BOOST_CHECK(ctrl.last_page_up.has_value());
}

BOOST_AUTO_TEST_CASE(page_down_is_handled_and_calls_do_page_down) {
    auto e = make_standard_key_event(QKeySequence::MoveToNextPage);
    BOOST_CHECK(impl::process_selection_key_event(ctrl, &e));
    BOOST_CHECK(ctrl.last_page_down.has_value());
}

BOOST_AUTO_TEST_CASE(select_page_up_is_handled_and_calls_do_page_up) {
    auto e = make_standard_key_event(QKeySequence::SelectPreviousPage);
    BOOST_CHECK(impl::process_selection_key_event(ctrl, &e));
    BOOST_CHECK(ctrl.last_page_up.has_value());
    BOOST_CHECK(ctrl.last_page_up->shift);
}

BOOST_AUTO_TEST_CASE(select_page_down_is_handled_and_calls_do_page_down) {
    auto e = make_standard_key_event(QKeySequence::SelectNextPage);
    BOOST_CHECK(impl::process_selection_key_event(ctrl, &e));
    BOOST_CHECK(ctrl.last_page_down.has_value());
    BOOST_CHECK(ctrl.last_page_down->shift);
}

BOOST_AUTO_TEST_CASE(unknown_key_is_not_handled) {
    QKeyEvent e{QEvent::KeyPress, Qt::Key_F1, Qt::NoModifier};
    BOOST_CHECK(!impl::process_selection_key_event(ctrl, &e));
}

BOOST_AUTO_TEST_SUITE_END()


// ---------------------------------------------------------------------------
// process_edit_key_event
// ---------------------------------------------------------------------------

BOOST_FIXTURE_TEST_SUITE(process_edit_key_event_test, edit_suite_fixture)

BOOST_AUTO_TEST_CASE(cut_is_handled_and_calls_controller_cut) {
    auto e = make_standard_key_event(QKeySequence::Cut);
    BOOST_CHECK(impl::process_edit_key_event(ctrl, &e));
    BOOST_CHECK(ctrl.cut_called);
}

BOOST_AUTO_TEST_CASE(paste_is_handled_and_calls_controller_paste) {
    QGuiApplication::clipboard()->setText("hello");
    auto e = make_standard_key_event(QKeySequence::Paste);
    BOOST_CHECK(impl::process_edit_key_event(ctrl, &e));
    BOOST_CHECK(ctrl.pasted == L"hello");
}

BOOST_AUTO_TEST_CASE(undo_is_handled) {
    auto e = make_standard_key_event(QKeySequence::Undo);
    BOOST_CHECK(impl::process_edit_key_event(ctrl, &e));
    BOOST_CHECK(ctrl.undo_called);
}

BOOST_AUTO_TEST_CASE(backspace_plain_is_handled_with_ctrl_false) {
    QKeyEvent e{QEvent::KeyPress, Qt::Key_Backspace, Qt::NoModifier};
    BOOST_CHECK(impl::process_edit_key_event(ctrl, &e));
    BOOST_REQUIRE(ctrl.last_backspace.has_value());
    BOOST_CHECK(!ctrl.last_backspace->ctrl);
}

BOOST_AUTO_TEST_CASE(backspace_ctrl_passes_ctrl_true) {
    QKeyEvent e{QEvent::KeyPress, Qt::Key_Backspace, Qt::ControlModifier};
    impl::process_edit_key_event(ctrl, &e);
    BOOST_REQUIRE(ctrl.last_backspace.has_value());
    BOOST_CHECK(ctrl.last_backspace->ctrl);
}

BOOST_AUTO_TEST_CASE(delete_plain_is_handled_with_ctrl_false) {
    QKeyEvent e{QEvent::KeyPress, Qt::Key_Delete, Qt::NoModifier};
    BOOST_CHECK(impl::process_edit_key_event(ctrl, &e));
    BOOST_REQUIRE(ctrl.last_delete.has_value());
    BOOST_CHECK(!ctrl.last_delete->ctrl);
}

BOOST_AUTO_TEST_CASE(delete_ctrl_passes_ctrl_true) {
    QKeyEvent e{QEvent::KeyPress, Qt::Key_Delete, Qt::ControlModifier};
    impl::process_edit_key_event(ctrl, &e);
    BOOST_REQUIRE(ctrl.last_delete.has_value());
    BOOST_CHECK(ctrl.last_delete->ctrl);
}

BOOST_AUTO_TEST_CASE(return_key_is_handled) {
    QKeyEvent e{QEvent::KeyPress, Qt::Key_Return, Qt::NoModifier};
    BOOST_CHECK(impl::process_edit_key_event(ctrl, &e));
    BOOST_CHECK(ctrl.last_enter.has_value());
}

BOOST_AUTO_TEST_CASE(tab_key_is_handled) {
    QKeyEvent e{QEvent::KeyPress, Qt::Key_Tab, Qt::NoModifier};
    BOOST_CHECK(impl::process_edit_key_event(ctrl, &e));
    BOOST_CHECK(ctrl.last_tab.has_value());
}

BOOST_AUTO_TEST_CASE(printable_char_without_modifier_calls_do_char) {
    QKeyEvent e{QEvent::KeyPress, Qt::Key_A, Qt::NoModifier, "a"};
    BOOST_CHECK(impl::process_edit_key_event(ctrl, &e));
    BOOST_REQUIRE(ctrl.last_char.has_value());
    BOOST_CHECK_EQUAL(*ctrl.last_char, L'a');
}

BOOST_AUTO_TEST_CASE(printable_char_with_ctrl_is_not_inserted) {
    QKeyEvent e{QEvent::KeyPress, Qt::Key_A, Qt::ControlModifier, "a"};
    impl::process_edit_key_event(ctrl, &e);
    BOOST_CHECK(!ctrl.last_char.has_value());
}

BOOST_AUTO_TEST_CASE(navigation_delegated_to_selection_handler) {
    QKeyEvent e{QEvent::KeyPress, Qt::Key_Left, Qt::NoModifier};
    BOOST_CHECK(impl::process_edit_key_event(ctrl, &e));
    BOOST_CHECK(ctrl.last_left.has_value());
}

BOOST_AUTO_TEST_SUITE_END()

// ---------------------------------------------------------------------------
// process_selection_key_event — navigation concepts
// ---------------------------------------------------------------------------

BOOST_FIXTURE_TEST_SUITE(process_selection_key_event_nav_test, nav_suite_fixture)

BOOST_AUTO_TEST_CASE(move_to_next_word_calls_move_next_word_with_shift_false) {
    auto e = make_standard_key_event(QKeySequence::MoveToNextWord);
    BOOST_CHECK(impl::process_selection_key_event(ctrl, &e));
    BOOST_REQUIRE(ctrl.last_move_next_word.has_value());
    BOOST_CHECK(!ctrl.last_move_next_word->shift);
}

BOOST_AUTO_TEST_CASE(select_next_word_calls_move_next_word_with_shift_true) {
    auto e = make_standard_key_event(QKeySequence::SelectNextWord);
    BOOST_CHECK(impl::process_selection_key_event(ctrl, &e));
    BOOST_REQUIRE(ctrl.last_move_next_word.has_value());
    BOOST_CHECK(ctrl.last_move_next_word->shift);
}

BOOST_AUTO_TEST_CASE(move_to_previous_word_calls_move_prev_word_with_shift_false) {
    auto e = make_standard_key_event(QKeySequence::MoveToPreviousWord);
    BOOST_CHECK(impl::process_selection_key_event(ctrl, &e));
    BOOST_REQUIRE(ctrl.last_move_prev_word.has_value());
    BOOST_CHECK(!ctrl.last_move_prev_word->shift);
}

BOOST_AUTO_TEST_CASE(select_previous_word_calls_move_prev_word_with_shift_true) {
    auto e = make_standard_key_event(QKeySequence::SelectPreviousWord);
    BOOST_CHECK(impl::process_selection_key_event(ctrl, &e));
    BOOST_REQUIRE(ctrl.last_move_prev_word.has_value());
    BOOST_CHECK(ctrl.last_move_prev_word->shift);
}

BOOST_AUTO_TEST_CASE(move_to_start_of_line_calls_move_line_start_with_shift_false) {
    auto e = make_standard_key_event(QKeySequence::MoveToStartOfLine);
    BOOST_CHECK(impl::process_selection_key_event(ctrl, &e));
    BOOST_REQUIRE(ctrl.last_move_line_start.has_value());
    BOOST_CHECK(!ctrl.last_move_line_start->shift);
}

BOOST_AUTO_TEST_CASE(select_start_of_line_calls_move_line_start_with_shift_true) {
    auto e = make_standard_key_event(QKeySequence::SelectStartOfLine);
    BOOST_CHECK(impl::process_selection_key_event(ctrl, &e));
    BOOST_REQUIRE(ctrl.last_move_line_start.has_value());
    BOOST_CHECK(ctrl.last_move_line_start->shift);
}

BOOST_AUTO_TEST_CASE(move_to_end_of_line_calls_move_line_end_with_shift_false) {
    auto e = make_standard_key_event(QKeySequence::MoveToEndOfLine);
    BOOST_CHECK(impl::process_selection_key_event(ctrl, &e));
    BOOST_REQUIRE(ctrl.last_move_line_end.has_value());
    BOOST_CHECK(!ctrl.last_move_line_end->shift);
}

BOOST_AUTO_TEST_CASE(select_end_of_line_calls_move_line_end_with_shift_true) {
    auto e = make_standard_key_event(QKeySequence::SelectEndOfLine);
    BOOST_CHECK(impl::process_selection_key_event(ctrl, &e));
    BOOST_REQUIRE(ctrl.last_move_line_end.has_value());
    BOOST_CHECK(ctrl.last_move_line_end->shift);
}

BOOST_AUTO_TEST_CASE(move_to_start_of_block_calls_move_block_start_with_shift_false) {
    if (QKeySequence{QKeySequence::MoveToStartOfBlock}.isEmpty()) {
        return;
    }

    auto e = make_standard_key_event(QKeySequence::MoveToStartOfBlock);
    BOOST_CHECK(impl::process_selection_key_event(ctrl, &e));
    BOOST_REQUIRE(ctrl.last_move_block_start.has_value());
    BOOST_CHECK(!ctrl.last_move_block_start->shift);
}

BOOST_AUTO_TEST_CASE(select_start_of_block_calls_move_block_start_with_shift_true) {
    if (QKeySequence{QKeySequence::SelectStartOfBlock}.isEmpty()) {
        return;
    }

    auto e = make_standard_key_event(QKeySequence::SelectStartOfBlock);
    BOOST_CHECK(impl::process_selection_key_event(ctrl, &e));
    BOOST_REQUIRE(ctrl.last_move_block_start.has_value());
    BOOST_CHECK(ctrl.last_move_block_start->shift);
}

BOOST_AUTO_TEST_CASE(move_to_end_of_block_calls_move_block_end_with_shift_false) {
    if (QKeySequence{QKeySequence::MoveToEndOfBlock}.isEmpty()) {
        return;
    }

    auto e = make_standard_key_event(QKeySequence::MoveToEndOfBlock);
    BOOST_CHECK(impl::process_selection_key_event(ctrl, &e));
    BOOST_REQUIRE(ctrl.last_move_block_end.has_value());
    BOOST_CHECK(!ctrl.last_move_block_end->shift);
}

BOOST_AUTO_TEST_CASE(select_end_of_block_calls_move_block_end_with_shift_true) {
    if (QKeySequence{QKeySequence::SelectEndOfBlock}.isEmpty()) {
        return;
    }

    auto e = make_standard_key_event(QKeySequence::SelectEndOfBlock);
    BOOST_CHECK(impl::process_selection_key_event(ctrl, &e));
    BOOST_REQUIRE(ctrl.last_move_block_end.has_value());
    BOOST_CHECK(ctrl.last_move_block_end->shift);
}

BOOST_AUTO_TEST_CASE(alt_up_calls_move_block_start_with_shift_false) {
    QKeyEvent e{QEvent::KeyPress, Qt::Key_Up, Qt::AltModifier};
    BOOST_CHECK(impl::process_selection_key_event(ctrl, &e));
    BOOST_REQUIRE(ctrl.last_move_block_start.has_value());
    BOOST_CHECK(!ctrl.last_move_block_start->shift);
}

BOOST_AUTO_TEST_CASE(shift_alt_up_calls_move_block_start_with_shift_true) {
    QKeyEvent e{QEvent::KeyPress, Qt::Key_Up, Qt::AltModifier | Qt::ShiftModifier};
    BOOST_CHECK(impl::process_selection_key_event(ctrl, &e));
    BOOST_REQUIRE(ctrl.last_move_block_start.has_value());
    BOOST_CHECK(ctrl.last_move_block_start->shift);
}

BOOST_AUTO_TEST_CASE(alt_down_calls_move_block_end_with_shift_false) {
    QKeyEvent e{QEvent::KeyPress, Qt::Key_Down, Qt::AltModifier};
    BOOST_CHECK(impl::process_selection_key_event(ctrl, &e));
    BOOST_REQUIRE(ctrl.last_move_block_end.has_value());
    BOOST_CHECK(!ctrl.last_move_block_end->shift);
}

BOOST_AUTO_TEST_CASE(shift_alt_down_calls_move_block_end_with_shift_true) {
    QKeyEvent e{QEvent::KeyPress, Qt::Key_Down, Qt::AltModifier | Qt::ShiftModifier};
    BOOST_CHECK(impl::process_selection_key_event(ctrl, &e));
    BOOST_REQUIRE(ctrl.last_move_block_end.has_value());
    BOOST_CHECK(ctrl.last_move_block_end->shift);
}

BOOST_AUTO_TEST_CASE(move_to_previous_line_calls_move_prev_line_with_shift_false) {
    auto e = make_standard_key_event(QKeySequence::MoveToPreviousLine);
    BOOST_CHECK(impl::process_selection_key_event(ctrl, &e));
    BOOST_REQUIRE(ctrl.last_move_prev_line.has_value());
    BOOST_CHECK(!ctrl.last_move_prev_line->shift);
}

BOOST_AUTO_TEST_CASE(select_previous_line_calls_move_prev_line_with_shift_true) {
    auto e = make_standard_key_event(QKeySequence::SelectPreviousLine);
    BOOST_CHECK(impl::process_selection_key_event(ctrl, &e));
    BOOST_REQUIRE(ctrl.last_move_prev_line.has_value());
    BOOST_CHECK(ctrl.last_move_prev_line->shift);
}

BOOST_AUTO_TEST_CASE(move_to_next_line_calls_move_next_line_with_shift_false) {
    auto e = make_standard_key_event(QKeySequence::MoveToNextLine);
    BOOST_CHECK(impl::process_selection_key_event(ctrl, &e));
    BOOST_REQUIRE(ctrl.last_move_next_line.has_value());
    BOOST_CHECK(!ctrl.last_move_next_line->shift);
}

BOOST_AUTO_TEST_CASE(select_next_line_calls_move_next_line_with_shift_true) {
    auto e = make_standard_key_event(QKeySequence::SelectNextLine);
    BOOST_CHECK(impl::process_selection_key_event(ctrl, &e));
    BOOST_REQUIRE(ctrl.last_move_next_line.has_value());
    BOOST_CHECK(ctrl.last_move_next_line->shift);
}

BOOST_AUTO_TEST_SUITE_END()

} // namespace tmvc::qt::test
