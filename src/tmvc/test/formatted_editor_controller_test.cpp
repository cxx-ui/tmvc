// Copyright (c) 2025, Alexandr Esilevich
//
// Distributed under the Boost Software License.
// See accompanying file LICENSE for license information.
//

/// \file formatted_editor_controller_test.cpp
/// Contains unit tests for single_edit_controller with formatted characters.

#include "../formatted_char.hpp"
#include "../simple_text_model.hpp"
#include "../single_selection_model.hpp"
#include "../single_edit_controller.hpp"
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
        out.push_back(fchar_t{ch, fmt});
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

} // namespace


struct formatted_editor_controller_test_fixture {
    ftext_t text;
    single_selection_model<ftext_t> selection{text};
    modification_history<fchar_t> history;
    single_edit_controller<ftext_t> controller{text, selection, history};
};


BOOST_FIXTURE_TEST_SUITE(formatted_editor_controller_test, formatted_editor_controller_test_fixture)


/// Tests replacing text with set_text
BOOST_AUTO_TEST_CASE(set_text_replace) {
    controller.set_text(make_formatted(L"the old text\nxx"));

    int n_before_inserted_calls = 0;
    text.before_inserted.connect([this, &n_before_inserted_calls](auto && r) {
        ++n_before_inserted_calls;
        BOOST_CHECK_EQUAL(r.start.line, 0);
        BOOST_CHECK_EQUAL(r.start.column, 0);
        BOOST_CHECK(formatted_string(text).empty());
    });

    int n_after_inserted_calls = 0;
    text.after_inserted.connect([this, &n_after_inserted_calls](auto && r) {
        ++n_after_inserted_calls;
        BOOST_CHECK_EQUAL(r.start.line, 0);
        BOOST_CHECK_EQUAL(r.start.column, 0);
        BOOST_CHECK(formatted_string(text) == L"ZZZ\nxx\naaa");
    });

    controller.set_text(make_formatted(L"ZZZ\nxx\naaa"));
    BOOST_CHECK(formatted_string(text) == L"ZZZ\nxx\naaa");
    BOOST_CHECK_EQUAL(selection.pos().line, 0);
    BOOST_CHECK_EQUAL(selection.pos().column, 0);

    BOOST_CHECK_EQUAL(n_before_inserted_calls, 1);
    BOOST_CHECK_EQUAL(n_after_inserted_calls, 1);
}


/// Tests inserting single formatted character in current position
BOOST_AUTO_TEST_CASE(insert_chars_single) {
    controller.set_text(make_formatted(L"the old text\nsecond line"));
    controller.select_text({0, 10}, {0, 10});

    tmvc::text_format fmt{};
    fmt.set_bold(true);
    controller.do_char(fchar_t{L'x', fmt});

    BOOST_CHECK_EQUAL(selection.pos().line, 0);
    BOOST_CHECK_EQUAL(selection.pos().column, 11);
    BOOST_CHECK(formatted_string(text) == L"the old texxt\nsecond line");
    BOOST_CHECK_EQUAL(text.char_at({0, 10}).format().bold(), true);
}


/// Tests inserting formatted characters at current position
BOOST_AUTO_TEST_CASE(insert_chars_curr_pos) {
    controller.set_text(make_formatted(L"the old text\nsecond line"));
    controller.select_text({0, 10}, {0, 10});

    tmvc::text_format fmt{};
    fmt.set_italic(true);
    controller.paste(make_formatted(L"first\nsecond\nthird", fmt));

    BOOST_CHECK_EQUAL(selection.pos().line, 2);
    BOOST_CHECK_EQUAL(selection.pos().column, 5);
    BOOST_CHECK(formatted_string(text) == L"the old tefirst\nsecond\nthirdxt\nsecond line");
    BOOST_CHECK_EQUAL(text.char_at({0, 10}).format().italic(), true);
}


BOOST_AUTO_TEST_SUITE_END()


}
