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
    auto cursor = edit->textCursor();

    // moving cursor to beginning of line to reset selection
    cursor.movePosition(QTextCursor::StartOfBlock, QTextCursor::MoveAnchor, 1);

    // moving up or down to specific anchor line
    auto mdlAnchorLine = static_cast<int>(anchor_pos.line);
    auto widgetLine = cursor.blockNumber();
    assert(mdlAnchorLine < edit->document()->blockCount() && "invalid position line number");
    if (widgetLine < mdlAnchorLine) {
        cursor.movePosition(QTextCursor::NextBlock,
                            QTextCursor::MoveAnchor,
                            static_cast<int>(mdlAnchorLine - widgetLine));
    } else {
        cursor.movePosition(QTextCursor::PreviousBlock,
                            QTextCursor::MoveAnchor,
                            static_cast<int>(widgetLine - mdlAnchorLine));
    }

    // then moving cursor to specific anchor column
    auto mdlAnchorCol = static_cast<int>(anchor_pos.column);
    assert(mdlAnchorCol < cursor.block().length() && "invalid position column number");
    cursor.movePosition(QTextCursor::NextCharacter, QTextCursor::MoveAnchor, mdlAnchorCol);

    // now moving to current line with keeping anchor
    auto mdlLine = static_cast<int>(pos.line);
    if (mdlAnchorLine < mdlLine) {
        cursor.movePosition(QTextCursor::NextBlock,
                            QTextCursor::KeepAnchor,
                            static_cast<int>(mdlLine - mdlAnchorLine));
    } else {
        cursor.movePosition(QTextCursor::PreviousBlock,
                            QTextCursor::KeepAnchor,
                            static_cast<int>(mdlAnchorLine - mdlLine));
    }

    // moving cursor to beginning of line with keeping anchor
    cursor.movePosition(QTextCursor::StartOfBlock, QTextCursor::KeepAnchor, 1);

    // finally moving cursor to specific anchor column
    auto mdlCol = static_cast<int>(pos.column);
    cursor.movePosition(QTextCursor::NextCharacter, QTextCursor::KeepAnchor, mdlCol);

    return cursor;
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
