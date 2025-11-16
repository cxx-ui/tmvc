// Copyright (c) 2025, Alexandr Esilevich
//
// Distributed under the Boost Software License.
// See accompanying file LICENSE for license information.
//

/// \file preserving_viewport_position_model.hpp
/// Contains definition of the preserving_viewport_position_model class.

#pragma once

#include "selection_model.hpp"
#include "simple_viewport_position_model.hpp"
#include "text_model.hpp"
#include <cstdint>


namespace tmvc {


/// Viewport position model that preserves relative viewport position
/// after changes in text model, and moves viewport position after changes in
/// selection model to make current position visible in viewport.
template <text_model TextModel, selection_model SelectionModel>
class preserving_viewport_position_model: public simple_viewport_position_model {
public:
    /// Type of text model
    using text_model_t = TextModel;

    /// Type of selection model
    using selection_model_t = SelectionModel;

    /// Constructs viewport position model with specified references
    /// to text model and selection model, and current position
    preserving_viewport_position_model(text_model_t & txt, selection_model_t & sel, uint64_t p = 0):
    simple_viewport_position_model{p},
    text_{txt}, selection_{sel} {
        // listening for text model changes

        // passing at_front flag when connecting to the before_erased signal to
        // catch it before selection model changes text position
        before_erased_con_ = text_.before_erased.connect([this](auto && range) {
            on_before_erased(range);
        }, boost::signals2::at_front);

        after_erased_con_ = text_.after_erased_2.connect([this](auto && ...) {
            on_after_erased();
        });

        before_inserted_con_ = text_.before_inserted.connect([this](auto && range, auto && ...) {
            on_before_inserted(range);
        });

        after_inserted_con_ = text_.after_inserted.connect([this](auto && ...) {
            on_after_inserted();
        });

        // listening for selection model changes
        selection_changed_con_ = selection_.changed.connect([this] {
            on_selection_changed();
        });
    }

    /// Returns max viewport position
    uint64_t max_pos() const { return text_.lines_size() - 1; }

    /// Sets number of visible lines in viewport. Should be called by views when
    /// they resized.
    void set_visible_lines_count(uint64_t cnt) { visible_lines_count_ = cnt; }

private:
    /// Called before characters erased from text model
    void on_before_erased(const range & range) {
        if (selection_.pos() > range.end && selection_.pos().line >= pos()) {
            preserve_relative_vertical_pos_offset_ = selection_.pos().line - pos();
        }
    }

    /// Called after characters erased from text model
    void on_after_erased() {
        preserve_relative_vertical_pos_offset_ = UINT64_MAX;
    }

    /// Called before characters inserted into text model
    void on_before_inserted(const range & range) {
        if (selection_.pos() > range.end && selection_.pos().line >= pos()) {
            preserve_relative_vertical_pos_offset_ = selection_.pos().line - pos();
        }
    }

    /// Called after characters inserted into text model
    void on_after_inserted() {
        preserve_relative_vertical_pos_offset_ = UINT64_MAX;
    }

    /// Called when selection model changed
    void on_selection_changed() {
        // moving vertical viewport position to make sure selection position is visible
        if (preserve_relative_vertical_pos_offset_ != UINT64_MAX) {
            // trying preserve viewport position relative to selection position

            auto new_pos = selection_.pos().line;
            if (new_pos >= preserve_relative_vertical_pos_offset_) {
                new_pos -= preserve_relative_vertical_pos_offset_;
            } else {
                new_pos = 0;
            }

            if (new_pos > max_pos()) {
                new_pos = max_pos();
            }

            set_pos(new_pos);
        } else {
            if (selection_.pos().line < pos()) {
                // moving backward
                set_pos(selection_.pos().line);
            } else {
                auto vis_lcount = visible_lines_count_;
                if (vis_lcount != 0 &&
                    selection_.pos().line >= vis_lcount &&
                    selection_.pos().line - vis_lcount + 1 > pos()) {
                    // moving forward
                    set_pos(selection_.pos().line - vis_lcount + 1);
                }
            }
        }

    }

    text_model_t & text_;           ///< Reference to text model
    selection_model_t & selection_; ///< Reference to selection model

    /// Number of visible lines in viewport
    uint64_t visible_lines_count_ = 0;

    /// Offset in lines from viewport position to selection position that should be
    /// preserved when selection position is changed in selection model.
    /// Set to valid value when removing or inserting characters before current selection position.
    uint64_t preserve_relative_vertical_pos_offset_ = UINT64_MAX;

    // connections to text model signals
    scoped_signal_connection before_erased_con_;
    scoped_signal_connection after_erased_con_;
    scoped_signal_connection before_inserted_con_;
    scoped_signal_connection after_inserted_con_;

    /// Connection to selection model changed signal
    scoped_signal_connection selection_changed_con_;
};


}
