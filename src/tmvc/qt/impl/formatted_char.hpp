// Copyright (c) 2025, Alexandr Esilevich
//
// Distributed under the Boost Software License.
// See accompanying file LICENSE for license information.
//

/// \file formatted_char.hpp
/// Contains conversion between formatted characters and Qt text formats.

#pragma once

#include "../../formatted_char.hpp"
#include "string.hpp"
#include <algorithm>
#include <cstdint>
#include <optional>
#include <QBrush>
#include <QColor>
#include <QTextCursor>
#include <QFont>
#include <QTextCharFormat>


namespace tmvc::qt::impl {


/// Converts tmvc color to QColor
inline QColor color_to_qt_color(const color & c) {
    return QColor{static_cast<int>(c.r()),
                  static_cast<int>(c.g()),
                  static_cast<int>(c.b())};
}

/// Converts QColor to tmvc color
inline color qt_color_to_color(const QColor & c) {
    if (!c.isValid()) {
        return {};
    }

    auto to_byte = [](int value) {
        return static_cast<std::uint8_t>(std::clamp(value, 0, 255));
    };

    return color{to_byte(c.red()), to_byte(c.green()), to_byte(c.blue())};
}

/// Converts tmvc text_format to QTextCharFormat
inline QTextCharFormat text_format_to_qt_char_format(const text_format & fmt) {
    QTextCharFormat out;
    if (fmt.foreground()) {
        out.setForeground(QBrush{color_to_qt_color(*fmt.foreground())});
    } else {
        out.clearForeground();
    }
    if (fmt.background()) {
        out.setBackground(QBrush{color_to_qt_color(*fmt.background())});
    } else {
        out.clearBackground();
    }
    out.setFontWeight(fmt.bold() ? QFont::Bold : QFont::Normal);
    out.setFontItalic(fmt.italic());
    out.setFontUnderline(fmt.underline());
    out.setFontStrikeOut(fmt.strikethrough());
    return out;
}

/// Converts formatted character to QTextCharFormat
template <std_character Char>
inline QTextCharFormat formatted_char_to_qt_char_format(const basic_formatted_char<Char> & ch) {
    return text_format_to_qt_char_format(ch.format());
}

/// Converts QTextCharFormat to tmvc text_format
inline text_format qt_char_format_to_text_format(const QTextCharFormat & fmt) {
    auto bold = fmt.fontWeight() >= QFont::Bold;
    std::optional<color> fg;
    std::optional<color> bg;
    if (fmt.foreground().style() != Qt::NoBrush && fmt.foreground().color().isValid()) {
        fg = qt_color_to_color(fmt.foreground().color());
    }
    if (fmt.background().style() != Qt::NoBrush && fmt.background().color().isValid()) {
        bg = qt_color_to_color(fmt.background().color());
    }
    return text_format{fg,
                       bg,
                       bold,
                       fmt.fontItalic(),
                       fmt.fontUnderline(),
                       fmt.fontStrikeOut()};
}

/// Converts QTextCharFormat to formatted character with specified character
template <std_character Char>
inline basic_formatted_char<Char> qt_char_to_formatted_char(Char ch,
                                                            const QTextCharFormat & fmt) {
    return basic_formatted_char<Char>{ch, qt_char_format_to_text_format(fmt)};
}

/// Inserts formatted characters at specified QTextCursor position
template <std_character Char>
inline void insert_formatted_chars(QTextCursor & cursor,
                                   const std::vector<basic_formatted_char<Char>> & chars) {
    if (chars.empty()) {
        return;
    }

    cursor.beginEditBlock();
    for (const auto & ch : chars) {
        std::basic_string<Char> str;
        str.push_back(ch.character());
        cursor.insertText(std_string_to_qstring(str),
                          text_format_to_qt_char_format(ch.format()));
    }
    cursor.endEditBlock();
}

/// Reads formatted characters from document at specified cursor
template <std_character Char>
inline std::vector<basic_formatted_char<Char>> read_formatted_chars(QTextCursor cursor) {
    std::vector<basic_formatted_char<Char>> out;
    if (!cursor.hasSelection()) {
        return out;
    }

    auto start = cursor.selectionStart();
    auto end = cursor.selectionEnd();
    cursor.setPosition(start);

    while (cursor.position() < end) {
        cursor.setPosition(cursor.position() + 1, QTextCursor::KeepAnchor);
        auto qstr = cursor.selectedText();
        auto chars = qstring_to_chars<Char>(qstr);
        if (!chars.empty()) {
            const auto ch = *chars.begin();
            out.emplace_back(ch, qt_char_format_to_text_format(cursor.charFormat()));
        }
        cursor.setPosition(cursor.position());
        start = cursor.position();
    }

    return out;
}


}
