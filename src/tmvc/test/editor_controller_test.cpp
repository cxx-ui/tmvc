// Copyright (c) 2025, Alexandr Esilevich
//
// Distributed under the Boost Software License.
// See accompanying file LICENSE for license information.
//

/// \file editor_controller_test.cpp
/// Contains unit tests for single_edit_controller class.

#include "../simple_text_model.hpp"
#include "../single_selection_model.hpp"
#include "../single_edit_controller.hpp"
#include <boost/test/unit_test.hpp>


namespace tmvc::test {


struct editor_controller_test_fixture {
    wsimple_text_model text;
    single_selection_model<wsimple_text_model> selection{text};
    modification_history<wchar_t> history;
    single_edit_controller<wsimple_text_model> controller{text, selection, history};
};


BOOST_FIXTURE_TEST_SUITE(editor_controller_test, editor_controller_test_fixture)


/// Tests replacing text with set_text
BOOST_AUTO_TEST_CASE(set_text_replace) {
    controller.set_text(L"the old text\nxx");

    unsigned int pos_changed_sig_num = 0;
    selection.changed.connect([this, &pos_changed_sig_num] {
        if (pos_changed_sig_num == 0) {
            BOOST_CHECK_EQUAL(selection.pos().line, 2);
            BOOST_CHECK_EQUAL(selection.pos().column, 3);
            BOOST_CHECK_EQUAL(selection.anchor_pos().line, 2);
            BOOST_CHECK_EQUAL(selection.anchor_pos().column, 3);
        } else {
            BOOST_CHECK_EQUAL(selection.pos().line, 0);
            BOOST_CHECK_EQUAL(selection.pos().column, 0);
            BOOST_CHECK_EQUAL(selection.anchor_pos().line, 0);
            BOOST_CHECK_EQUAL(selection.anchor_pos().column, 0);
        }

        ++pos_changed_sig_num;
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

    controller.set_text(L"ZZZ\nxx\naaa");
    BOOST_CHECK(string(text) == L"ZZZ\nxx\naaa");

    BOOST_CHECK_EQUAL(n_before_inserted_calls, 1);
    BOOST_CHECK_EQUAL(n_after_inserted_calls, 1);
    BOOST_CHECK_EQUAL(n_before_removed_calls, 1);
    BOOST_CHECK_EQUAL(n_after_removed_calls, 1);
    BOOST_CHECK_EQUAL(pos_changed_sig_num, 2);
}


/// Tests inserting single character in current position
BOOST_AUTO_TEST_CASE(insert_chars_single) {
    controller.set_text(L"the old text\nsecond line");
    controller.select_text({0, 10}, {0, 10});

    int n_pos_changed_calls = 0;
    selection.changed.connect([this, &n_pos_changed_calls] {
        ++n_pos_changed_calls;
        BOOST_CHECK_EQUAL(selection.pos().line, 0);
        BOOST_CHECK_EQUAL(selection.pos().column, 11);
        BOOST_CHECK_EQUAL(selection.anchor_pos().line, 0);
        BOOST_CHECK_EQUAL(selection.anchor_pos().column, 11);
    });

    int n_before_inserted_calls = 0;
    text.before_inserted.connect([this, &n_before_inserted_calls](auto && r) {
        ++n_before_inserted_calls;
        BOOST_CHECK(string(text) == L"the old text\nsecond line");
    });

    int n_after_inserted_calls = 0;
    text.after_inserted.connect([this, &n_after_inserted_calls](auto && r) {
        ++n_after_inserted_calls;
        BOOST_CHECK(string(text) == L"the old texxt\nsecond line");
    });

    text.before_erased.connect([this](auto && r) {
        BOOST_CHECK(false);
    });

    text.after_erased.connect([this](auto && r) {
        BOOST_CHECK(false);
    });

    controller.do_char(L'x');

    BOOST_CHECK_EQUAL(selection.pos().line, 0);
    BOOST_CHECK_EQUAL(selection.pos().column, 11);
    BOOST_CHECK_EQUAL(selection.anchor_pos().line, 0);
    BOOST_CHECK_EQUAL(selection.anchor_pos().column, 11);
    BOOST_CHECK(string(text) == L"the old texxt\nsecond line");

    BOOST_CHECK_EQUAL(n_before_inserted_calls, 1);
    BOOST_CHECK_EQUAL(n_after_inserted_calls, 1);
    BOOST_CHECK_EQUAL(n_pos_changed_calls, 1);
}


/// Tests inserting characters at current position
BOOST_AUTO_TEST_CASE(insert_chars_curr_pos) {
    controller.set_text(L"the old text\nsecond line");
    controller.select_text({0, 10}, {0, 10});

    int n_pos_changed_calls = 0;
    selection.changed.connect([this, &n_pos_changed_calls] {
        ++n_pos_changed_calls;
        BOOST_CHECK_EQUAL(selection.pos().line, 2);
        BOOST_CHECK_EQUAL(selection.pos().column, 5);
        BOOST_CHECK_EQUAL(selection.anchor_pos().line, 2);
        BOOST_CHECK_EQUAL(selection.anchor_pos().column, 5);
    });

    int n_before_inserted_calls = 0;
    text.before_inserted.connect([this, &n_before_inserted_calls](auto && r) {
        ++n_before_inserted_calls;
        BOOST_CHECK(string(text) == L"the old text\nsecond line");
    });

    int n_after_inserted_calls = 0;
    text.after_inserted.connect([this, &n_after_inserted_calls](auto && r) {
        ++n_after_inserted_calls;
        BOOST_CHECK(string(text) == L"the old tefirst\nsecond\nthirdxt\nsecond line");
    });

    text.before_erased.connect([this](auto && r) {
        BOOST_CHECK(false);
    });

    text.after_erased.connect([this](auto && r) {
        BOOST_CHECK(false);
    });

    controller.paste(std::wstring{L"first\nsecond\nthird"});

    BOOST_CHECK_EQUAL(selection.pos().line, 2);
    BOOST_CHECK_EQUAL(selection.pos().column, 5);
    BOOST_CHECK_EQUAL(selection.anchor_pos().line, 2);
    BOOST_CHECK_EQUAL(selection.anchor_pos().column, 5);
    BOOST_CHECK(string(text) == L"the old tefirst\nsecond\nthirdxt\nsecond line");

    BOOST_CHECK_EQUAL(n_before_inserted_calls, 1);
    BOOST_CHECK_EQUAL(n_after_inserted_calls, 1);
    BOOST_CHECK_EQUAL(n_pos_changed_calls, 1);
}


/// Tests inserting characters instead of selection
BOOST_AUTO_TEST_CASE(insert_chars_selection) {
    controller.set_text(L"the old text\nsecond line");
    controller.select_text({0, 10}, {1, 3});

    int n_pos_changed_calls = 0;
    selection.changed.connect([this, &n_pos_changed_calls] {
        ++n_pos_changed_calls;

        if (n_pos_changed_calls == 1) {
            // the first call is after deleting selection
            BOOST_CHECK_EQUAL(selection.pos().line, 0);
            BOOST_CHECK_EQUAL(selection.pos().column, 10);
            BOOST_CHECK_EQUAL(selection.anchor_pos().line, 0);
            BOOST_CHECK_EQUAL(selection.anchor_pos().column, 10);
        } else {
            // the second call is after inserting characters
            BOOST_CHECK_EQUAL(selection.pos().line, 2);
            BOOST_CHECK_EQUAL(selection.pos().column, 5);
            BOOST_CHECK_EQUAL(selection.anchor_pos().line, 2);
            BOOST_CHECK_EQUAL(selection.anchor_pos().column, 5);
        }
    });

    int n_before_inserted_calls = 0;
    text.before_inserted.connect([this, &n_before_inserted_calls](auto && r) {
        ++n_before_inserted_calls;
        BOOST_CHECK(string(text) == L"the old teond line");
    });

    int n_after_inserted_calls = 0;
    text.after_inserted.connect([this, &n_after_inserted_calls](auto && r) {
        ++n_after_inserted_calls;
        BOOST_CHECK(string(text) == L"the old tefirst\nsecond\nthirdond line");
    });

    int n_before_removed_calls = 0;
    text.before_erased.connect([this, &n_before_removed_calls](auto && r) {
        ++n_before_removed_calls;
        BOOST_CHECK(string(text) == L"the old text\nsecond line");
    });

    int n_after_removed_calls = 0;
    text.after_erased.connect([this, &n_after_removed_calls](auto && r) {
        ++n_after_removed_calls;
        BOOST_CHECK(string(text) == L"the old teond line");
    });

    controller.paste(std::wstring{L"first\nsecond\nthird"});

    BOOST_CHECK_EQUAL(selection.pos().line, 2);
    BOOST_CHECK_EQUAL(selection.pos().column, 5);
    BOOST_CHECK_EQUAL(selection.anchor_pos().line, 2);
    BOOST_CHECK_EQUAL(selection.anchor_pos().column, 5);
    BOOST_CHECK(string(text) == L"the old tefirst\nsecond\nthirdond line");

    BOOST_CHECK_EQUAL(n_before_inserted_calls, 1);
    BOOST_CHECK_EQUAL(n_after_inserted_calls, 1);
    BOOST_CHECK_EQUAL(n_before_removed_calls, 1);
    BOOST_CHECK_EQUAL(n_after_removed_calls, 1);
    BOOST_CHECK_EQUAL(n_pos_changed_calls, 2);
}


/// Tests performing delete action with selected range
BOOST_AUTO_TEST_CASE(do_delete_selection) {
    controller.set_text(L"first line\nsecond line");
    controller.select_text({0, 4}, {1, 2});
    controller.do_delete(false, false);

    BOOST_CHECK(string(text) == L"firscond line");
    BOOST_CHECK_EQUAL(selection.pos().line, 0);
    BOOST_CHECK_EQUAL(selection.pos().column, 4);
    BOOST_CHECK_EQUAL(selection.anchor_pos().line, 0);
    BOOST_CHECK_EQUAL(selection.anchor_pos().column, 4);
}


/// Tests performing delete without selection
BOOST_AUTO_TEST_CASE(do_delete_no_selection) {
    controller.set_text(L"first line\nsecond line");
    controller.select_text({0, 4}, {0, 4});
    controller.do_delete(false, false);

    BOOST_CHECK(string(text) == L"firs line\nsecond line");
    BOOST_CHECK_EQUAL(selection.pos().line, 0);
    BOOST_CHECK_EQUAL(selection.pos().column, 4);
    BOOST_CHECK_EQUAL(selection.anchor_pos().line, 0);
    BOOST_CHECK_EQUAL(selection.anchor_pos().column, 4);
}


/// Tests performing delete at the end of document
BOOST_AUTO_TEST_CASE(do_delete_doc_end) {
    controller.set_text(L"first line\nsecond line");
    controller.select_text({1, 11}, {1, 11});
    controller.do_delete(false, false);

    BOOST_CHECK(string(text) == L"first line\nsecond line");
    BOOST_CHECK_EQUAL(selection.pos().line, 1);
    BOOST_CHECK_EQUAL(selection.pos().column, 11);
    BOOST_CHECK_EQUAL(selection.anchor_pos().line, 1);
    BOOST_CHECK_EQUAL(selection.anchor_pos().column, 11);
}


/// Tests performing backspace action with selected range
BOOST_AUTO_TEST_CASE(do_backspace_selection) {
    controller.set_text(L"first line\nsecond line");
    controller.select_text({0, 4}, {1, 2});
    controller.do_backspace(false, false);

    BOOST_CHECK(string(text) == L"firscond line");
    BOOST_CHECK_EQUAL(selection.pos().line, 0);
    BOOST_CHECK_EQUAL(selection.pos().column, 4);
    BOOST_CHECK_EQUAL(selection.anchor_pos().line, 0);
    BOOST_CHECK_EQUAL(selection.anchor_pos().column, 4);
}


/// Tests performing backspace without selection
BOOST_AUTO_TEST_CASE(do_backspace_no_selection) {
    controller.set_text(L"first line\nsecond line");
    controller.select_text({0, 4}, {0, 4});
    controller.do_backspace(false, false);

    BOOST_CHECK(string(text) == L"firt line\nsecond line");
    BOOST_CHECK_EQUAL(selection.pos().line, 0);
    BOOST_CHECK_EQUAL(selection.pos().column, 3);
    BOOST_CHECK_EQUAL(selection.anchor_pos().line, 0);
    BOOST_CHECK_EQUAL(selection.anchor_pos().column, 3);
}


/// Tests performing backspace of new line
BOOST_AUTO_TEST_CASE(do_backspace_new_line) {
    controller.set_text(L"first line\nsecond line");
    controller.select_text({1, 0}, {1, 0});

    controller.do_backspace(false, false);

    BOOST_CHECK(string(text) == L"first linesecond line");
    BOOST_CHECK_EQUAL(selection.pos().line, 0);
    BOOST_CHECK_EQUAL(selection.pos().column, 10);
    BOOST_CHECK_EQUAL(selection.anchor_pos().line, 0);
    BOOST_CHECK_EQUAL(selection.anchor_pos().column, 10);
}


/// Tests performing backspace at the beginning of document
BOOST_AUTO_TEST_CASE(do_backspace_doc_begin) {
    controller.set_text(L"first line\nsecond line");
    controller.select_text({0, 0}, {0, 0});
    controller.do_backspace(false, false);

    BOOST_CHECK(string(text) == L"first line\nsecond line");
    BOOST_CHECK_EQUAL(selection.pos().line, 0);
    BOOST_CHECK_EQUAL(selection.pos().column, 0);
    BOOST_CHECK_EQUAL(selection.anchor_pos().line, 0);
    BOOST_CHECK_EQUAL(selection.anchor_pos().column, 0);
}


/// Tests enter action
BOOST_AUTO_TEST_CASE(do_enter) {
    controller.set_text(L"first line\nsecond\nthird line");
    controller.select_text({1, 3}, {1, 3});
    controller.do_enter(false, false);

    BOOST_CHECK(string(text) == L"first line\nsec\nond\nthird line");
    BOOST_CHECK_EQUAL(selection.pos().line, 2);
    BOOST_CHECK_EQUAL(selection.pos().column, 0);
    BOOST_CHECK_EQUAL(selection.anchor_pos().line, 2);
    BOOST_CHECK_EQUAL(selection.anchor_pos().column, 0);
}


/// Tests enter action with selection
BOOST_AUTO_TEST_CASE(do_enter_selection) {
    controller.set_text(L"first line\nsecond\nthird line");
    controller.select_text({0, 3}, {2, 5});
    controller.do_enter(false, false);

    BOOST_CHECK(string(text) == L"fir\n line");
    BOOST_CHECK_EQUAL(selection.pos().line, 1);
    BOOST_CHECK_EQUAL(selection.pos().column, 0);
    BOOST_CHECK_EQUAL(selection.anchor_pos().line, 1);
    BOOST_CHECK_EQUAL(selection.anchor_pos().column, 0);
}


/// Tests tab action without expanding spaces
BOOST_AUTO_TEST_CASE(do_tab_no_expand) {
    controller.set_text(L"first line\nsecond\nthird line");
    controller.select_text({1, 0}, {1, 0});
    controller.do_tab(false, false);

    BOOST_CHECK(string(text) == L"first line\n\tsecond\nthird line");
    BOOST_CHECK_EQUAL(selection.pos().line, 1);
    BOOST_CHECK_EQUAL(selection.pos().column, 1);
    BOOST_CHECK_EQUAL(selection.anchor_pos().line, 1);
    BOOST_CHECK_EQUAL(selection.anchor_pos().column, 1);
}


/// Tests tab action at the beginning of line
BOOST_AUTO_TEST_CASE(do_tab_line_start) {
    controller.set_expand_tabs(true);
    controller.set_tab_size(4);
    controller.set_text(L"first line\nsecond\nthird line");
    controller.select_text({1, 0}, {1, 0});
    controller.do_tab(false, false);

    BOOST_CHECK(string(text) == L"first line\n    second\nthird line");
    BOOST_CHECK_EQUAL(selection.pos().line, 1);
    BOOST_CHECK_EQUAL(selection.pos().column, 4);
    BOOST_CHECK_EQUAL(selection.anchor_pos().line, 1);
    BOOST_CHECK_EQUAL(selection.anchor_pos().column, 4);
}


/// Tests tab action at the middle of line
BOOST_AUTO_TEST_CASE(do_tab_line_middle) {
    controller.set_expand_tabs(true);
    controller.set_tab_size(4);
    controller.set_text(L"first line\nsecond\nthird line");
    controller.select_text({1, 1}, {1, 1});
    controller.do_tab(false, false);

    BOOST_CHECK(string(text) == L"first line\ns   econd\nthird line");
    BOOST_CHECK_EQUAL(selection.pos().line, 1);
    BOOST_CHECK_EQUAL(selection.pos().column, 4);
    BOOST_CHECK_EQUAL(selection.anchor_pos().line, 1);
    BOOST_CHECK_EQUAL(selection.anchor_pos().column, 4);
}


/// Tests tab action with selection
BOOST_AUTO_TEST_CASE(do_tab_selection) {
    controller.set_expand_tabs(true);
    controller.set_tab_size(4);
    controller.set_text(L"first line\nsecond\nthird line");
    controller.select_text({1, 1}, {1, 2});
    controller.do_tab(false, false);

    BOOST_CHECK(string(text) == L"first line\ns   cond\nthird line");
    BOOST_CHECK_EQUAL(selection.pos().line, 1);
    BOOST_CHECK_EQUAL(selection.pos().column, 4);
    BOOST_CHECK_EQUAL(selection.anchor_pos().line, 1);
    BOOST_CHECK_EQUAL(selection.anchor_pos().column, 4);
}


/// Tests char action
BOOST_AUTO_TEST_CASE(do_char) {
    controller.set_text(L"first line\nsecond\nthird line");
    controller.select_text({1, 1}, {1, 1});
    controller.do_char('X');

    BOOST_CHECK(string(text) == L"first line\nsXecond\nthird line");
    BOOST_CHECK_EQUAL(selection.pos().line, 1);
    BOOST_CHECK_EQUAL(selection.pos().column, 2);
    BOOST_CHECK_EQUAL(selection.anchor_pos().line, 1);
    BOOST_CHECK_EQUAL(selection.anchor_pos().column, 2);
}


/// Tests char action with overwrite mode
BOOST_AUTO_TEST_CASE(do_char_overwrite) {
    controller.set_text(L"first line\nsecond\nthird line");
    controller.set_overwrite_mode(true);
    controller.select_text({1, 1}, {1, 1});
    controller.do_char('X');

    BOOST_CHECK(string(text) == L"first line\nsXcond\nthird line");
    BOOST_CHECK_EQUAL(selection.pos().line, 1);
    BOOST_CHECK_EQUAL(selection.pos().column, 2);
    BOOST_CHECK_EQUAL(selection.anchor_pos().line, 1);
    BOOST_CHECK_EQUAL(selection.anchor_pos().column, 2);
}


/// Tests char action with overwrite mode at the end of line
BOOST_AUTO_TEST_CASE(do_char_overwrite_end_line) {
    controller.set_text(L"first line\nsecond\nthird line");
    controller.set_overwrite_mode(true);
    controller.select_text({1, 6}, {1, 6});
    controller.do_char('X');

    BOOST_CHECK(string(text) == L"first line\nsecondX\nthird line");
    BOOST_CHECK_EQUAL(selection.pos().line, 1);
    BOOST_CHECK_EQUAL(selection.pos().column, 7);
    BOOST_CHECK_EQUAL(selection.anchor_pos().line, 1);
    BOOST_CHECK_EQUAL(selection.anchor_pos().column, 7);
}


/// Tests setting position after inserting new line
BOOST_AUTO_TEST_CASE(set_pos_after_insert_line) {
    controller.set_text(L"first line\nvery long second line");
    controller.select_text({0, 10}, {0, 10});
    controller.do_char(L'\n');
    controller.select_text({1, 0}, {1, 0});
    controller.do_char(L'a');
    controller.select_text({1, 1}, {1, 1});
    controller.do_char(L'a');
    controller.select_text({1, 2}, {1, 2});
    controller.do_char(L'a');
    controller.select_text({1, 3}, {1, 3});
    controller.do_char(L'a');
    controller.select_text({1, 4}, {1, 4});

    controller.select_text({2, 20}, {2, 20});

    BOOST_CHECK(string(text) == L"first line\naaaa\nvery long second line");
    BOOST_CHECK_EQUAL(selection.pos().line, 2);
    BOOST_CHECK_EQUAL(selection.pos().column, 20);
    BOOST_CHECK_EQUAL(selection.anchor_pos().line, 2);
    BOOST_CHECK_EQUAL(selection.anchor_pos().column, 20);
}


/// Tests preserving indentation
BOOST_AUTO_TEST_CASE(do_enter_indent) {
    controller.set_text(L"  zzz\n\t   first line\nsecond line");
    controller.select_text({1, 14}, {1, 14});
    controller.do_enter(false, false);

    BOOST_CHECK(string(text) == L"  zzz\n\t   first line\n\t   \nsecond line");
    BOOST_CHECK_EQUAL(selection.pos().line, 2);
    BOOST_CHECK_EQUAL(selection.pos().column, 4);
    BOOST_CHECK_EQUAL(selection.anchor_pos().line, 2);
    BOOST_CHECK_EQUAL(selection.anchor_pos().column, 4);
}


/// Tests adding indent after {
BOOST_AUTO_TEST_CASE(do_enter_indent_add) {
    controller.set_expand_tabs(true);
    controller.set_tab_size(4);
    controller.set_text(L"  zzz\n\t   first line {\t \nsecond line");
    controller.select_text({1, 18}, {1, 18});
    controller.do_enter(false, false);

    BOOST_CHECK(string(text) == L"  zzz\n\t   first line {\t \n\t       \nsecond line");
    BOOST_CHECK_EQUAL(selection.pos().line, 2);
    BOOST_CHECK_EQUAL(selection.pos().column, 8);
    BOOST_CHECK_EQUAL(selection.anchor_pos().line, 2);
    BOOST_CHECK_EQUAL(selection.anchor_pos().column, 8);
}


/// Tests auto indent on enter in the middle of line with {
BOOST_AUTO_TEST_CASE(do_enter_indent_no_add) {
    controller.set_expand_tabs(true);
    controller.set_tab_size(4);
    controller.set_text(L"  zzz\n\t   first line {\t \nsecond line");
    controller.select_text({1, 8}, {1, 8});
    controller.do_enter(false, false);

    BOOST_CHECK(string(text) == L"  zzz\n\t   firs\n\t   t line {\t \nsecond line");
    BOOST_CHECK_EQUAL(selection.pos().line, 2);
    BOOST_CHECK_EQUAL(selection.pos().column, 4);
    BOOST_CHECK_EQUAL(selection.anchor_pos().line, 2);
    BOOST_CHECK_EQUAL(selection.anchor_pos().column, 4);
}


/// Tests preserving indentation on enter with selection
BOOST_AUTO_TEST_CASE(do_enter_indent_sel) {
    controller.set_text(L"\t   first line\n\nsecond line");
    controller.select_text({2, 0}, {2, 2});
    controller.do_enter(false, false);

    BOOST_CHECK(string(text) == L"\t   first line\n\n\n\t   cond line");
    BOOST_CHECK_EQUAL(selection.pos().line, 3);
    BOOST_CHECK_EQUAL(selection.pos().column, 4);
    BOOST_CHECK_EQUAL(selection.anchor_pos().line, 3);
    BOOST_CHECK_EQUAL(selection.anchor_pos().column, 4);
}


/// Tests preserving indentation with no previous line
BOOST_AUTO_TEST_CASE(do_enter_indent_no_prev_line) {
    controller.set_text(L"\t  \n\nsecond line");
    controller.select_text({1, 0}, {1, 0});
    controller.do_enter(false, false);

    BOOST_CHECK(string(text) == L"\t  \n\n\nsecond line");
    BOOST_CHECK_EQUAL(selection.pos().line, 2);
    BOOST_CHECK_EQUAL(selection.pos().column, 0);
    BOOST_CHECK_EQUAL(selection.anchor_pos().line, 2);
    BOOST_CHECK_EQUAL(selection.anchor_pos().column, 0);
}


/// Tests removing indent after inserting } on empty line
BOOST_AUTO_TEST_CASE(remove_indent) {
    controller.set_expand_tabs(true);
    controller.set_tab_size(4);
    controller.set_text(L"    first_line\n    ");
    controller.select_text({1, 4}, {1, 4});
    controller.do_char(L'}');

    BOOST_CHECK(string(text) == L"    first_line\n}");
    BOOST_CHECK_EQUAL(selection.pos().line, 1);
    BOOST_CHECK_EQUAL(selection.pos().column, 1);
    BOOST_CHECK_EQUAL(selection.anchor_pos().line, 1);
    BOOST_CHECK_EQUAL(selection.anchor_pos().column, 1);
}


/// Tests removing indent after inserting } on empty line after added indent
BOOST_AUTO_TEST_CASE(remove_indent_after_add) {
    controller.set_expand_tabs(true);
    controller.set_tab_size(4);
    controller.set_text(L"    first_line {\n        ");
    controller.select_text({1, 8}, {1, 8});
    controller.do_char(L'}');

    BOOST_CHECK(string(text) == L"    first_line {\n    }");
    BOOST_CHECK_EQUAL(selection.pos().line, 1);
    BOOST_CHECK_EQUAL(selection.pos().column, 5);
    BOOST_CHECK_EQUAL(selection.anchor_pos().line, 1);
    BOOST_CHECK_EQUAL(selection.anchor_pos().column, 5);
}


/// Tests inserting char after added indent
BOOST_AUTO_TEST_CASE(insert_char_after_add_indent) {
    controller.set_expand_tabs(true);
    controller.set_tab_size(4);
    controller.set_text(L"    first_line {\n        ");
    controller.select_text({1, 8}, {1, 8});
    controller.do_char(L'z');

    BOOST_CHECK(string(text) == L"    first_line {\n        z");
    BOOST_CHECK_EQUAL(selection.pos().line, 1);
    BOOST_CHECK_EQUAL(selection.pos().column, 9);
    BOOST_CHECK_EQUAL(selection.anchor_pos().line, 1);
    BOOST_CHECK_EQUAL(selection.anchor_pos().column, 9);
}


/// Tests removing spaces from line containing only spaces after pressing enter
BOOST_AUTO_TEST_CASE(remove_spaces_after_enter) {
    controller.set_text(L"\tline of text\n   \t   ");
    
    controller.select_text({1, 5}, {1, 5});
    controller.do_enter(false, false);

    BOOST_CHECK(string(text) == L"\tline of text\n\n\t");
    BOOST_CHECK_EQUAL(selection.pos().line, 2);
    BOOST_CHECK_EQUAL(selection.pos().column, 1);
    BOOST_CHECK_EQUAL(selection.anchor_pos().line, 2);
    BOOST_CHECK_EQUAL(selection.anchor_pos().column, 1);
}


/// Tests replacing selected characters with tab
BOOST_AUTO_TEST_CASE(replace_with_tab) {
    controller.set_text(L"\tline of text\n\tline 2");
    controller.select_text({0, 2}, {0, 5});
    controller.do_tab(false, false);

    BOOST_CHECK(string(text) == L"\tl\t of text\n\tline 2");
    BOOST_CHECK_EQUAL(selection.pos().line, 0);
    BOOST_CHECK_EQUAL(selection.pos().column, 3);
    BOOST_CHECK_EQUAL(selection.anchor_pos().line, 0);
    BOOST_CHECK_EQUAL(selection.anchor_pos().column, 3);
}


/// Tests adding indent if complete line is selected
BOOST_AUTO_TEST_CASE(add_indent_single_line) {
    controller.set_text(L"\tline of text\n\tline 2");
    controller.select_text({0, 0}, {0, 13});
    controller.do_tab(false, false);

    BOOST_CHECK(string(text) == L"\t\tline of text\n\tline 2");
    BOOST_CHECK_EQUAL(selection.pos().line, 0);
    BOOST_CHECK_EQUAL(selection.pos().column, 14);
    BOOST_CHECK_EQUAL(selection.anchor_pos().line, 0);
    BOOST_CHECK_EQUAL(selection.anchor_pos().column, 0);
}


/// Tests removing indent if complete line is selected
BOOST_AUTO_TEST_CASE(remove_indent_single_line) {
    controller.set_text(L"\tline of text\n\tline 2");
    controller.select_text({0, 0}, {0, 13});
    controller.do_tab(false, true);

    BOOST_CHECK(string(text) == L"line of text\n\tline 2");
    BOOST_CHECK_EQUAL(selection.pos().line, 0);
    BOOST_CHECK_EQUAL(selection.pos().column, 12);
    BOOST_CHECK_EQUAL(selection.anchor_pos().line, 0);
    BOOST_CHECK_EQUAL(selection.anchor_pos().column, 0);
}


/// Tests adding indent if multiple lines selected
BOOST_AUTO_TEST_CASE(add_indent_multi_line) {
    controller.set_text(L"\tline of text\n\tline 2\n\tline3");
    controller.select_text({0, 5}, {1, 2});
    controller.do_tab(false, false);

    BOOST_CHECK(string(text) == L"\t\tline of text\n\t\tline 2\n\tline3");
    BOOST_CHECK_EQUAL(selection.pos().line, 1);
    BOOST_CHECK_EQUAL(selection.pos().column, 3);
    BOOST_CHECK_EQUAL(selection.anchor_pos().line, 0);
    BOOST_CHECK_EQUAL(selection.anchor_pos().column, 6);
}


/// Tests removing indent if multiple lines selected
BOOST_AUTO_TEST_CASE(remove_indent_multi_line) {
    controller.set_text(L"\tline of text\n\tline 2\n\tline3");
    controller.select_text({0, 5}, {1, 2});
    controller.do_tab(false, true);

    BOOST_CHECK(string(text) == L"line of text\nline 2\n\tline3");
    BOOST_CHECK_EQUAL(selection.pos().line, 1);
    BOOST_CHECK_EQUAL(selection.pos().column, 1);
    BOOST_CHECK_EQUAL(selection.anchor_pos().line, 0);
    BOOST_CHECK_EQUAL(selection.anchor_pos().column, 4);
}


/// Tests removing indent for multiple lines with expand tabs
BOOST_AUTO_TEST_CASE(remove_indent_multi_line_expand_tabs) {
    controller.set_text(L"        line of text\n        line 2");
    controller.set_expand_tabs(true);
    controller.select_text({0, 4}, {1, 14});
    controller.do_tab(false, true);

    BOOST_CHECK(string(text) == L"    line of text\n    line 2");
    BOOST_CHECK_EQUAL(selection.pos().line, 1);
    BOOST_CHECK_EQUAL(selection.pos().column, 10);
    BOOST_CHECK_EQUAL(selection.anchor_pos().line, 0);
    BOOST_CHECK_EQUAL(selection.anchor_pos().column, 0);
}


/// Tests inserting indentation spaces in empty line
BOOST_AUTO_TEST_CASE(insert_indent_spaces_empty_line) {
    auto txt = L"";
    controller.set_expand_tabs(true);
    controller.set_tab_size(4);
    controller.set_text(txt);
    controller.select_text({0, 0}, {0, 0});
    controller.do_tab(false, false);

    BOOST_CHECK(string(text) == L"    ");
    BOOST_CHECK_EQUAL(selection.pos().line, 0);
    BOOST_CHECK_EQUAL(selection.pos().column, 4);
    BOOST_CHECK_EQUAL(selection.anchor_pos().line, 0);
    BOOST_CHECK_EQUAL(selection.anchor_pos().column, 4);
}


/// Tests removing indent from middle of line
BOOST_AUTO_TEST_CASE(do_shift_tab_line_middle) {
    controller.set_expand_tabs(true);
    controller.set_tab_size(4);
    controller.set_text(L"first line\ns       econd\nthird line");
    controller.select_text({1, 7}, {1, 7});
    controller.do_tab(false, true);

    BOOST_CHECK(string(text) == L"first line\ns    econd\nthird line");
    BOOST_CHECK_EQUAL(selection.pos().line, 1);
    BOOST_CHECK_EQUAL(selection.pos().column, 4);
    BOOST_CHECK_EQUAL(selection.anchor_pos().line, 1);
    BOOST_CHECK_EQUAL(selection.anchor_pos().column, 4);
}


/// Second test for removing indent from middle of line
BOOST_AUTO_TEST_CASE(do_shift_tab_line_middle_2) {
    controller.set_expand_tabs(true);
    controller.set_tab_size(4);
    controller.set_text(L"first line\ns    econd\nthird line");
    controller.select_text({1, 4}, {1, 4});
    controller.do_tab(false, true);

    BOOST_CHECK(string(text) == L"first line\ns econd\nthird line");
    BOOST_CHECK_EQUAL(selection.pos().line, 1);
    BOOST_CHECK_EQUAL(selection.pos().column, 1);
    BOOST_CHECK_EQUAL(selection.anchor_pos().line, 1);
    BOOST_CHECK_EQUAL(selection.anchor_pos().column, 1);
}


/// Tests removing \r characters from input sequence
BOOST_AUTO_TEST_CASE(remove_r_char) {
    controller.set_text(L"aaaaa\r\nbbbbb");
    BOOST_CHECK(string(text) == L"aaaaa\nbbbbb");
}



/// Tests performing undo and redo after inserting characters
BOOST_AUTO_TEST_CASE(do_insert_undo_redo) {
    controller.set_text(L"first line\nsecond\nthird line");

    int n_can_undo_changed = 0;
    history.can_undo_changed.connect([this, &n_can_undo_changed] {
        ++n_can_undo_changed;

        if (n_can_undo_changed == 1) {
            BOOST_CHECK(string(text) == L"first line\nseaacond\nthird line");
            BOOST_CHECK(controller.can_undo());
            BOOST_CHECK(!controller.can_redo());
            BOOST_CHECK(history.changed());
        } else if (n_can_undo_changed == 2) {
            BOOST_CHECK(string(text) == L"first line\nsecond\nthird line");
            BOOST_CHECK(!controller.can_undo());
            BOOST_CHECK(controller.can_redo());
            BOOST_CHECK(!history.changed());
        } else if (n_can_undo_changed == 3) {
            BOOST_CHECK(string(text) == L"first line\nseaacond\nthird line");
            BOOST_CHECK(controller.can_undo());
            BOOST_CHECK(!controller.can_redo());
            BOOST_CHECK(history.changed());
        } else {
            BOOST_CHECK(false);
        }
    });


    int n_can_redo_changed = 0;
    history.can_redo_changed.connect([this, &n_can_redo_changed] {
        ++n_can_redo_changed;

        if (n_can_redo_changed == 1) {
            BOOST_CHECK(string(text) == L"first line\nsecond\nthird line");
            BOOST_CHECK(!controller.can_undo());
            BOOST_CHECK(controller.can_redo());
            BOOST_CHECK(!history.changed());
        } else if (n_can_redo_changed == 2) {
            BOOST_CHECK(string(text) == L"first line\nseaacond\nthird line");
            BOOST_CHECK(controller.can_undo());
            BOOST_CHECK(!controller.can_redo());
            BOOST_CHECK(history.changed());
        } else {
            BOOST_CHECK(false);
        }
    });

    int n_changed_changed = 0;
    history.changed_changed.connect([this, &n_changed_changed] {
        ++n_changed_changed;

        if (n_changed_changed == 1) {
            BOOST_CHECK(string(text) == L"first line\nseaacond\nthird line");
            BOOST_CHECK(history.changed());
        } else if (n_changed_changed == 2) {
            BOOST_CHECK(string(text) == L"first line\nsecond\nthird line");
            BOOST_CHECK(!history.changed());
        } else if (n_changed_changed == 3) {
            BOOST_CHECK(string(text) == L"first line\nseaacond\nthird line");
            BOOST_CHECK(history.changed());
        }
    });


    controller.select_text({1, 2}, {1, 2});
    controller.paste(std::wstring{L"aa"});

    BOOST_CHECK(string(text) == L"first line\nseaacond\nthird line");
    BOOST_CHECK(controller.can_undo());
    BOOST_CHECK(!controller.can_redo());
    BOOST_CHECK(history.changed());

    BOOST_REQUIRE(controller.can_undo());
    controller.undo();
    BOOST_CHECK(string(text) == L"first line\nsecond\nthird line");
    BOOST_CHECK(!controller.can_undo());
    BOOST_CHECK(controller.can_redo());
    BOOST_CHECK(!history.changed());

    BOOST_REQUIRE(controller.can_redo());
    controller.redo();
    BOOST_CHECK(string(text) == L"first line\nseaacond\nthird line");
    BOOST_CHECK(controller.can_undo());
    BOOST_CHECK(!controller.can_redo());
    BOOST_CHECK(history.changed());


    BOOST_CHECK_EQUAL(n_can_undo_changed, 3);
    BOOST_CHECK_EQUAL(n_can_redo_changed, 2);
    BOOST_CHECK_EQUAL(n_changed_changed, 3);
}


/// Tests performing undo and redo after deleting characters
BOOST_AUTO_TEST_CASE(do_delete_undo_redo) {
    controller.set_text(L"first line\nsecond\nthird line");

    int n_can_undo_changed = 0;
    history.can_undo_changed.connect([this, &n_can_undo_changed] {
        ++n_can_undo_changed;

        if (n_can_undo_changed == 1) {
            BOOST_CHECK(string(text) == L"first line\nsend\nthird line");
            BOOST_CHECK(controller.can_undo());
            BOOST_CHECK(!controller.can_redo());
            BOOST_CHECK(history.changed());
        } else if (n_can_undo_changed == 2) {
            BOOST_CHECK(string(text) == L"first line\nsecond\nthird line");
            BOOST_CHECK(!controller.can_undo());
            BOOST_CHECK(controller.can_redo());
            BOOST_CHECK(!history.changed());
        } else if (n_can_undo_changed == 3) {
            BOOST_CHECK(string(text) == L"first line\nsend\nthird line");
            BOOST_CHECK(controller.can_undo());
            BOOST_CHECK(!controller.can_redo());
            BOOST_CHECK(history.changed());
        } else {
            BOOST_CHECK(false);
        }
    });


    int n_can_redo_changed = 0;
    history.can_redo_changed.connect([this, &n_can_redo_changed] {
        ++n_can_redo_changed;

        if (n_can_redo_changed == 1) {
            BOOST_CHECK(string(text) == L"first line\nsecond\nthird line");
            BOOST_CHECK(!controller.can_undo());
            BOOST_CHECK(controller.can_redo());
            BOOST_CHECK(!history.changed());
        } else if (n_can_redo_changed == 2) {
            BOOST_CHECK(string(text) == L"first line\nsend\nthird line");
            BOOST_CHECK(controller.can_undo());
            BOOST_CHECK(!controller.can_redo());
            BOOST_CHECK(history.changed());
        } else {
            BOOST_CHECK(false);
        }
    });


    controller.select_text({1, 2}, {1, 4});
    controller.do_delete(false, false);

    BOOST_CHECK(string(text) == L"first line\nsend\nthird line");
    BOOST_CHECK(controller.can_undo());
    BOOST_CHECK(!controller.can_redo());
    BOOST_CHECK(history.changed());

    BOOST_REQUIRE(controller.can_undo());
    controller.undo();
    BOOST_CHECK(string(text) == L"first line\nsecond\nthird line");
    BOOST_CHECK(!controller.can_undo());
    BOOST_CHECK(controller.can_redo());
    BOOST_CHECK(!history.changed());

    BOOST_REQUIRE(controller.can_redo());
    controller.redo();
    BOOST_CHECK(string(text) == L"first line\nsend\nthird line");
    BOOST_CHECK(controller.can_undo());
    BOOST_CHECK(!controller.can_redo());
    BOOST_CHECK(history.changed());


    BOOST_CHECK_EQUAL(n_can_undo_changed, 3);
    BOOST_CHECK_EQUAL(n_can_redo_changed, 2);
}


/// Tests removing spaces from line containing only spaces in before_save
BOOST_AUTO_TEST_CASE(remove_spaces_before_save) {
    controller.set_text(L"\tline of text\n   \t   ");
    
    controller.select_text({1, 5}, {1, 5});
    controller.do_before_save();

    BOOST_CHECK(string(text) == L"\tline of text\n");
    BOOST_CHECK_EQUAL(selection.pos().line, 1);
    BOOST_CHECK_EQUAL(selection.pos().column, 0);
    BOOST_CHECK_EQUAL(selection.anchor_pos().line, 1);
    BOOST_CHECK_EQUAL(selection.anchor_pos().column, 0);
}


/// Tests switching overwrite mode with insert key
BOOST_AUTO_TEST_CASE(switch_overwrite) {
    controller.set_text(L"\tline of text\n   \t   ");
    BOOST_CHECK(!controller.is_overwrite_mode());

    controller.do_insert(false, false);
    BOOST_CHECK(controller.is_overwrite_mode());

    controller.do_insert(false, false);
    BOOST_CHECK(!controller.is_overwrite_mode());
}


/// Tests resetting up/down position after inserting character
BOOST_AUTO_TEST_CASE(insert_char_reset_up_down) {
    controller.set_text(L"first line\nsecond");
    controller.select_text({0, 10}, {0, 10});

    controller.do_down(false, false);
    controller.do_char(L'x');
    controller.do_up(false, false);

    BOOST_CHECK_EQUAL(selection.pos().line, 0);
    BOOST_CHECK_EQUAL(selection.pos().column, 7);
    BOOST_CHECK_EQUAL(selection.anchor_pos().line, 0);
    BOOST_CHECK_EQUAL(selection.anchor_pos().column, 7);
    BOOST_CHECK(string(text) == L"first line\nsecondx");
}


/// Tests resetting up/down position after deleting character
BOOST_AUTO_TEST_CASE(delete_char_reset_up_down) {
    controller.set_text(L"first line\nsecond");
    controller.select_text({0, 10}, {0, 10});

    controller.do_down(false, false);
    controller.do_backspace(false, false);
    controller.do_up(false, false);

    BOOST_CHECK_EQUAL(selection.pos().line, 0);
    BOOST_CHECK_EQUAL(selection.pos().column, 5);
    BOOST_CHECK_EQUAL(selection.anchor_pos().line, 0);
    BOOST_CHECK_EQUAL(selection.anchor_pos().column, 5);
    BOOST_CHECK(string(text) == L"first line\nsecon");
}


BOOST_AUTO_TEST_SUITE_END()


}
