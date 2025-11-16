// Copyright (c) 2025, Alexandr Esilevich
//
// Distributed under the Boost Software License.
// See accompanying file LICENSE for license information.
//

/// \file viewport_model.ipp
/// Contains implementation of the viewport_model class.

#include <QFontMetricsF>
#include <QTextLayout>


namespace tmvc::qt::impl {


template <text_model TextModel, selection_model SelectionModel, viewport_position_model ViewportPositionModel>
float viewport_model<TextModel, SelectionModel, ViewportPositionModel>::line_height() const {
    QFontMetricsF font_metrics{fnt_};
    return font_metrics.lineSpacing() + 1.0f;
}


template <text_model TextModel, selection_model SelectionModel, viewport_position_model ViewportPositionModel>
float viewport_model<TextModel, SelectionModel, ViewportPositionModel>::line_width(uint64_t line_idx, size_t n_first_chars) const {
    auto str = line_str(text_mdl_, line_idx);
    assert(n_first_chars <= str.size() && "invalid length of substring of line");

    str = str.substr(0, n_first_chars);

    QFontMetrics font_metrics{fnt_};
    QString qstr;
    if constexpr(std::is_same_v<char_t, wchar_t>) {
        qstr = QString::fromStdWString(str);
    } else {
        qstr = QString::fromStdString(str);
    }

    return font_metrics.horizontalAdvance(qstr);
}


template <text_model TextModel, selection_model SelectionModel, viewport_position_model ViewportPositionModel>
uint64_t viewport_model<TextModel, SelectionModel, ViewportPositionModel>::line_position(uint64_t line_idx, float offset, bool overwrite_mode) const {
    auto str = line_str(text_mdl_, line_idx);

    if (str.empty() || offset < 0.0f) {
        return 0;
    }

    // setting up layout parameters
    QTextLayout layout;
    layout.setFont(fnt_);
    layout.setText(QString::fromStdWString(str));
    QTextOption opt;
    opt.setWrapMode(QTextOption::NoWrap);
    layout.setTextOption(opt);

    // laying out string
    layout.beginLayout();
    auto line_layout = layout.createLine();
    assert(line_layout.isValid() && "single text layout line is not valid");
    line_layout.setPosition(QPointF{0, 0});
    layout.endLayout();

    // determining position in string
    auto cursor_mode = overwrite_mode ? QTextLine::CursorOnCharacter : QTextLine::CursorBetweenCharacters;
    auto pos_i = layout.lineAt(0).xToCursor(offset, cursor_mode);
    assert(pos_i >= 0 && pos_i <= str.size() && "invalid position returned from text layout");

    return static_cast<uint64_t>(pos_i);
}


}
