// Copyright (c) 2025, Alexandr Esilevich
//
// Distributed under the Boost Software License.
// See accompanying file LICENSE for license information.
//

/// \file viewport_model.hpp
/// Contains definition of the basic_viewport_model class and associated type aliases.

#pragma once

#include "../../impl/viewport_model.hpp"
#include <QFont>


namespace tmvc::qt::impl {


/// Viewport model implementation for the Qt library
template <
    text_model TextModel,
    selection_model SelectionModel,
    viewport_position_model ViewportPositionModel
>
class viewport_model: public tmvc::impl::viewport_model<TextModel, SelectionModel, ViewportPositionModel> {
public:
    /// Type of text model
    using text_model_t = TextModel;

    /// Type of selection model
    using selection_model_t = SelectionModel;

    /// Type of viewport position model
    using viewport_position_model_t = ViewportPositionModel;

    /// Type of character
    using char_t = typename text_model_t::char_t;

    /// Constructs viewport model with specified references to models and Qt text font
    viewport_model(const TextModel & text_mdl,
                   selection_model_t & sel_mdl,
                   viewport_position_model_t & v_pos,
                   const QFont & fnt):
        tmvc::impl::viewport_model<TextModel, SelectionModel, ViewportPositionModel>{text_mdl, sel_mdl, v_pos},
        text_mdl_{text_mdl},
        fnt_{fnt} {}

    /// Destroys model
    ~viewport_model() override = default;

    /// Returns font line height
    float line_height() const override;

    /// Returns width of substring of line with specified index.
    float line_width(uint64_t line_idx, size_t n_first_chars) const override;

    /// Returns position in line with specified index at specified horizontal offset.
    uint64_t line_position(uint64_t line_idx, float offset, bool overwrite_mode) const override;

    /// Sets font
    void set_font(const QFont & fnt) {
        fnt_ = fnt;
        this->layout_updated();
    }

private:
    const TextModel & text_mdl_;    ///< Reference to model
    QFont fnt_;                     ///< Text font
};


}


#include "viewport_model.ipp"
