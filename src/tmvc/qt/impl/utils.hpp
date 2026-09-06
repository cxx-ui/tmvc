// Copyright (c) 2025, Alexandr Esilevich
//
// Distributed under the Boost Software License.
// See accompanying file LICENSE for license information.
//

/// \file utils.hpp
/// Contains declaration of utility functions related to Qt text views.

#pragma once

#include <cassert>
#include <QPlainTextEdit>
#include <QTextBlock>
#include <QTextCursor>
#include <QTextEdit>


namespace tmvc::qt::impl {


/// Returns Qt text cursor from QPlainTextEdit for specified selection
template <typename QtTextEdit>
inline QTextCursor get_qt_cursor_for_selection(QtTextEdit * edit,
                                               const position & anchor_pos,
                                               const position & pos) {
    auto * doc = edit->document();

    // getting document block by line number for anchor
    auto anchor_block = doc->findBlockByNumber(static_cast<int>(anchor_pos.line));
    assert(anchor_block.isValid() && "invalid anchor position line number");
    assert(static_cast<int>(anchor_pos.column) < anchor_block.length() &&
           "invalid anchor position column number");

    // getting document block by line number for poition
    auto block = doc->findBlockByNumber(static_cast<int>(pos.line));
    assert(block.isValid() && "invalid position line number");
    assert(static_cast<int>(pos.column) < block.length() && "invalid position column number");

    // creating cursor for selection
    QTextCursor cursor{doc};
    cursor.setPosition(anchor_block.position() + static_cast<int>(anchor_pos.column));
    cursor.setPosition(block.position() + static_cast<int>(pos.column), QTextCursor::KeepAnchor);

    return cursor;
}


/// Calculates current position from cursor position
inline position get_position_from_cursor(const QTextCursor & cursor) {
    auto pos_line = cursor.blockNumber();
    auto pos_col = cursor.positionInBlock();
    return position{static_cast<size_t>(pos_line),
                    static_cast<size_t>(pos_col)};
}


/// Calculates selection anchor and current position from cursor position
/// in QPlainTextEdit or QTextEdit
template <typename QtTextEdit>
inline std::tuple<position, position> get_selection_from_text_edit(QtTextEdit * edit) {
    auto cursor = edit->textCursor();

    // getting anchor line and column
    auto anchorBlock = edit->document()->findBlock(cursor.anchor());
    auto anchorLine = anchorBlock.blockNumber();
    assert(anchorBlock.position() <= cursor.anchor() && "invalid block position");
    auto anchorCol = cursor.anchor() - anchorBlock.position();
    position anchor_pos{static_cast<size_t>(anchorLine),
                        static_cast<size_t>(anchorCol)};

    // getting position line and column
    auto posLine = cursor.blockNumber();
    auto posCol = cursor.positionInBlock();
    position pos{static_cast<size_t>(posLine),
                 static_cast<size_t>(posCol)};

    return {anchor_pos, pos};
}


}
