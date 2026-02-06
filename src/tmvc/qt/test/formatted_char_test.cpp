// Copyright (c) 2025, Alexandr Esilevich
//
// Distributed under the Boost Software License.
// See accompanying file LICENSE for license information.
//

/// \file formatted_char_test.cpp
/// Contains tests for formatted character Qt conversion helpers.

#include "../impl/formatted_char.hpp"
#include <boost/test/unit_test.hpp>
#include <QTextCursor>
#include <QTextDocument>
#include <QFont>
#include <string>
#include <vector>


namespace tmvc::qt::test {


BOOST_AUTO_TEST_SUITE(formatted_char_test)


BOOST_AUTO_TEST_CASE(to_qt_char_format_from_formatted_char) {
    tmvc::text_format fmt{tmvc::color{1, 2, 3},
                          tmvc::color{4, 5, 6},
                          true,
                          false,
                          true,
                          false};
    tmvc::wformatted_char ch{L'A', fmt};

    auto qt_fmt = impl::formatted_char_to_qt_char_format(ch);

    auto fg = qt_fmt.foreground().color();
    auto bg = qt_fmt.background().color();

    BOOST_CHECK_EQUAL(fg.red(), 1);
    BOOST_CHECK_EQUAL(fg.green(), 2);
    BOOST_CHECK_EQUAL(fg.blue(), 3);

    BOOST_CHECK_EQUAL(bg.red(), 4);
    BOOST_CHECK_EQUAL(bg.green(), 5);
    BOOST_CHECK_EQUAL(bg.blue(), 6);

    BOOST_CHECK(qt_fmt.fontWeight() >= QFont::Bold);
    BOOST_CHECK(!qt_fmt.fontItalic());
    BOOST_CHECK(qt_fmt.fontUnderline());
    BOOST_CHECK(!qt_fmt.fontStrikeOut());
}


BOOST_AUTO_TEST_CASE(from_qt_char_format_to_text_format) {
    QTextCharFormat qt_fmt;
    qt_fmt.setForeground(QBrush{QColor{10, 20, 30}});
    qt_fmt.setBackground(QBrush{QColor{40, 50, 60}});
    qt_fmt.setFontWeight(QFont::Bold);
    qt_fmt.setFontItalic(true);
    qt_fmt.setFontUnderline(true);
    qt_fmt.setFontStrikeOut(true);

    auto fmt = impl::qt_char_format_to_text_format(qt_fmt);

    BOOST_CHECK(fmt.foreground().has_value());
    BOOST_CHECK(fmt.background().has_value());
    BOOST_CHECK((*fmt.foreground() == tmvc::color{10, 20, 30}));
    BOOST_CHECK((*fmt.background() == tmvc::color{40, 50, 60}));
    BOOST_CHECK(fmt.bold());
    BOOST_CHECK(fmt.italic());
    BOOST_CHECK(fmt.underline());
    BOOST_CHECK(fmt.strikethrough());
}


BOOST_AUTO_TEST_CASE(roundtrip_format_and_formatted_char) {
    tmvc::text_format fmt{tmvc::color{11, 22, 33},
                          tmvc::color{44, 55, 66},
                          false,
                          true,
                          false,
                          true};

    auto qt_fmt = impl::text_format_to_qt_char_format(fmt);
    auto rt_fmt = impl::qt_char_format_to_text_format(qt_fmt);

    BOOST_CHECK(rt_fmt == fmt);

    auto fch = impl::qt_char_to_formatted_char(L'Z', qt_fmt);
    BOOST_CHECK_EQUAL(fch.character(), L'Z');
    BOOST_CHECK(fch.format() == fmt);
}

BOOST_AUTO_TEST_CASE(insert_formatted_chars_inserts_text_and_formats) {
    QTextDocument doc;
    QTextCursor cursor(&doc);

    tmvc::text_format fmt_a{tmvc::color{1, 2, 3},
                            tmvc::color{4, 5, 6},
                            true,
                            false,
                            false,
                            false};
    tmvc::text_format fmt_b{tmvc::color{7, 8, 9},
                            tmvc::color{10, 11, 12},
                            false,
                            true,
                            true,
                            true};

    std::vector<tmvc::wformatted_char> chars;
    chars.emplace_back(L'A', fmt_a);
    chars.emplace_back(L'B', fmt_b);

    impl::insert_formatted_chars(cursor, chars);

    BOOST_CHECK(doc.toPlainText().toStdWString() == std::wstring{L"AB"});

    QTextCursor check(&doc);
    check.setPosition(0);
    check.movePosition(QTextCursor::Right, QTextCursor::KeepAnchor, 1);
    auto fmt0 = impl::qt_char_format_to_text_format(check.charFormat());
    BOOST_CHECK(fmt0 == fmt_a);

    check.setPosition(1);
    check.movePosition(QTextCursor::Right, QTextCursor::KeepAnchor, 1);
    auto fmt1 = impl::qt_char_format_to_text_format(check.charFormat());
    BOOST_CHECK(fmt1 == fmt_b);
}

BOOST_AUTO_TEST_CASE(read_formatted_chars_reads_text_and_formats) {
    QTextDocument doc;
    QTextCursor cursor(&doc);

    tmvc::text_format fmt_a{tmvc::color{1, 2, 3},
                            tmvc::color{4, 5, 6},
                            true,
                            false,
                            false,
                            false};
    tmvc::text_format fmt_b{tmvc::color{7, 8, 9},
                            tmvc::color{10, 11, 12},
                            false,
                            true,
                            true,
                            true};

    std::vector<tmvc::wformatted_char> chars;
    chars.emplace_back(L'A', fmt_a);
    chars.emplace_back(L'B', fmt_b);

    impl::insert_formatted_chars(cursor, chars);

    QTextCursor sel(&doc);
    sel.setPosition(0);
    sel.setPosition(2, QTextCursor::KeepAnchor);

    auto out = impl::read_formatted_chars<wchar_t>(sel);

    BOOST_REQUIRE_EQUAL(out.size(), 2U);
    BOOST_CHECK_EQUAL(out[0].character(), L'A');
    BOOST_CHECK(out[0].format() == fmt_a);
    BOOST_CHECK_EQUAL(out[1].character(), L'B');
    BOOST_CHECK(out[1].format() == fmt_b);
}


BOOST_AUTO_TEST_SUITE_END()


}
