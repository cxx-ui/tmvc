// Copyright (c) 2025, Alexandr Esilevich
//
// Distributed under the Boost Software License.
// See accompanying file LICENSE for license information.
//

/// \file viewport_model.hpp
/// Contains definition of the basic_viewport_model class and associated type aliases.

#pragma once

#include "../signals.hpp"
#include "../position.hpp"
#include "../range.hpp"
#include "../viewport_position_model.hpp"
#include <cmath>


namespace tmvc::impl {


/// Represents abstract viewport for text editor or viewer.
/// Contains logic for managing current position after changes in selection model.
/// Should be subclassed for concrete view implementation and used only with single
/// instance of view.
template <
    text_model TextModel,
    selection_model SelectionModel,
    viewport_position_model ViewportPositionModel
>
class viewport_model {
public:
    /// Type of text model
    using text_model_t = TextModel;

    /// Type of selection model
    using selection_model_t = SelectionModel;

    /// Type of viewport position model
    using viewport_position_model_t = ViewportPositionModel;

    /// Constructs viewport model with specified references to text and text selection models, and text margin
    viewport_model(const TextModel & txt, selection_model_t & sel, viewport_position_model_t & v_pos):
    text_mdl_{txt}, sel_mdl_{sel}, viewport_pos_{v_pos} {
        pos_changed_con_ = sel_mdl_.changed.connect([this] {
            on_sel_pos_changed();
        });

        after_erased_con_ = text_mdl_.after_erased_2.connect([this](auto && ...) {
            layout_updated();
        });

        after_inserted_con_ = text_mdl_.after_inserted.connect([this](auto && ...) {
            layout_updated();
        });

        after_replaced_con_ = text_mdl_.after_replaced.connect([this](auto && ...) {
            layout_updated();
        });

        // listening to viewport position model changes
        viewport_pos_changed_con_ = viewport_pos_.changed.connect([this] {
            layout_updated();
        });
    }


    /// Virtual destructor
    virtual ~viewport_model() = default;


    /// Returns font line height. Default implementation returns 1.
    virtual float line_height() const { return 1.0; }


    /// Returns width of substring of line with specified index.
    /// Default implementation return length of substring.
    virtual float line_width(uint64_t line_idx, size_t n_first_chars) const {
        assert(n_first_chars <= text_mdl_.line_size(line_idx) && "invalid number of characters in line substring");
        return static_cast<float>(n_first_chars);
    }


    /// Returns position in line with specified index at specified horizontal offset.
    /// Default implementation returns rounded offset, taking into account line length.
    virtual uint64_t line_position(uint64_t line_idx, float offset, bool overwrite_mode) const {
        if (offset < 0.0f) {
            return 0;
        }

        uint64_t pos = static_cast<uint64_t>(std::roundf(offset));
        assert(pos >= 0 && "invalid rounded offset");

        auto line_sz = text_mdl_.line_size(line_idx);
        if (pos > line_sz) {
            pos = line_sz;
        }

        return pos;
    }

    /// Returns viewport width
    float width() const { return width_; }

    /// Returns viewport height
    float height() const { return height_; }

    /// Returns max viewport height
    float max_height() const { return max_height_; }

    /// Sets viewport size
    void set_size(float w, float h, float max_h) {
        // setting size
        width_ = w;
        height_ = h;
        max_height_ = max_h;

        // setting max visible lines in viewport position model
        viewport_pos_.set_visible_lines_count(visible_lines_count());

        // updating layout
        layout_updated();
    }


    /// Returns text left margin
    float left_margin() const {
        return left_margin_;
    }


    /// Sets text left margin
    void set_left_margin(float m) {
        left_margin_ = m;
        layout_updated();
    }


    /// Returns text right margin
    float right_margin() const {
        return right_margin_;
    }


    /// Sets text right margin
    void set_right_margin(float m) {
        right_margin_ = m;
        layout_updated();
    }


    /// Returns text top margin
    float top_margin() const {
        return top_margin_;
    }


    /// Sets text top margin
    void set_top_margin(float m) {
        top_margin_ = m;
        layout_updated();
    }


    /// Returns text width (width of viewport minus margins)
    float text_width() const {
        float tw = width() - left_margin_ - right_margin_;
        if (tw < 0.0f) {
            tw = 0.0f;
        }

        return tw; 
    }


    /// Returns text height (height of vewiport minus top)
    float text_height() const {
        float th = height() - top_margin_;
        if (th < 0.0f) {
            th = 0.0f;
        }

        return th;
    }


    /// Returns number of first visible line in viewport (vertical position).
    /// Reads this value from viewport position model.
    uint64_t vertical_pos() const {
        return viewport_pos_.pos();
    }


    /// Returns horizontal offset of viewport (horizontal position)
    float horizontal_pos() const {
        return horizontal_pos_;
    }


    /// Sets horizontal offset of viewport (horizontal position)
    void set_horizontal_pos(float offset) {
        if (horizontal_pos_ == offset) {
            return;
        }

        horizontal_pos_ = offset;
        layout_updated();
    }


    /// Calculates maximum viewport vertical positon
    uint64_t max_vertical_pos() const {
        auto lines_cnt = visible_lines_count();

        if (lines_cnt < text_mdl_.lines_size()) {
            return text_mdl_.lines_size() - lines_cnt;
        } else {
            return 0;
        }
    }


    /// Calculates maximum viewport horizontal position. Uses line_width virtual function
    /// to calculate maximum length of lines displayed in viewport.
    float max_horizontal_pos() const {
        auto t_width = text_width();
        if (t_width == 0.0f) {
            return 0.0f;
        }

        float max_line_width = 0.0f;

        for (uint64_t line_idx = vertical_pos(), end_idx = max_visible_line_index();
            line_idx <= end_idx; ++line_idx) {
            auto l_width = line_width(line_idx, text_mdl_.line_size(line_idx));
            if (max_line_width < l_width) {
                max_line_width = l_width;
            }
        }

        if (max_line_width > t_width) {
            return max_line_width - t_width;
        } else {
            return 0.0f;
        }
    }


    /// Calculates and returns number of fully visible lines of text in viewport
    uint64_t visible_lines_count() const {
        return static_cast<uint64_t>(text_height() / line_height());
    }


    /// Calculates and returns maximum visible lines count using maximum height for calculation
    uint64_t max_visible_lines_count() const {
        float text_height = max_height() - top_margin_;
        if (text_height < 0.0f) {
            text_height = 0.0f;
        }

        auto res_f = text_height / line_height();
        auto res = static_cast<uint64_t>(res_f);
        if (res_f - static_cast<float>(res) != 0.0f) {
            ++res;
        }

        return res;
    }


    /// Calculates and returns last index of maximum visible line taking into account number of lines in model
    uint64_t max_visible_line_index() const {
        if (max_visible_lines_count() == 0) {
            return vertical_pos();
        }

        assert(vertical_pos() < text_mdl_.lines_size() && "invalid vertical pos");
        if (text_mdl_.lines_size() - vertical_pos() < max_visible_lines_count()) {
            return text_mdl_.lines_size() - 1;
        }

        return vertical_pos() + (max_visible_lines_count() - 1);
    }


    /// Returns text position for specified coordinates inside viewport
    position text_pos(float x, float y, bool overwrite_mode) const {
        // calculating line index

        float text_top_offset = (vertical_pos() == 0) ? top_margin() : 0.0;
        float vertical_offset =  y - text_top_offset;

        uint64_t line_idx = vertical_pos();
        if (vertical_offset >= 0.0f) {
            uint64_t line_offset = static_cast<uint64_t>(vertical_offset / line_height());
            if (line_offset > text_mdl_.lines_size() || text_mdl_.lines_size() - line_offset <= vertical_pos()) {
                // position is the end position of text model
                return end_pos(text_mdl_);
            } else {
                line_idx += line_offset;
            }
        } else {
            uint64_t line_offset = static_cast<uint64_t>(std::ceil(-vertical_offset / line_height()));
            if (line_idx < line_offset) {
                // position if the beginning of text model
                return begin_pos(text_mdl_);
            } else {
                line_idx -= line_offset;
            }
        }

        assert(line_idx < text_mdl_.lines_size() && "invalid line index");

        // calculating position in line
        auto column = line_position(line_idx, x - left_margin() + horizontal_pos(), overwrite_mode);
        return {line_idx, column};
    }


    /// Performs page up action. Moves text position one page backward.
    void do_page_up(bool keep_anchor) {
        auto nlines = visible_lines_count();

        // calculating new viewport position
        auto new_vertical_pos = vertical_pos();
        if (new_vertical_pos < nlines) {
            new_vertical_pos = 0;
        } else {
            new_vertical_pos -= nlines;
        }

        // setting new viewport position
        viewport_pos_.set_pos(new_vertical_pos);
    }


    /// Performs page down action. Moves text position one page forward.
    void do_page_down(bool keep_anchor) {
        auto nlines = visible_lines_count();

        // calculating new viewport position
        auto new_vertical_pos = vertical_pos();
        if (max_vertical_pos() < nlines || max_vertical_pos() - nlines < new_vertical_pos) {
            new_vertical_pos = max_vertical_pos();
        } else {
            new_vertical_pos += nlines;
        }

        // setting new viewport position
        viewport_pos_.set_pos(new_vertical_pos);
    }


    /// The signal is emitted when text layout is updated and should be recalculated by view
    mutable signal<void ()> layout_updated;

private:
    /// Called when current position is changed in seleciton model
    void on_sel_pos_changed() {
        // moving horizontal viewport position to make sure selection position is visible and
        // centered horizontally in viewport

        auto cursors_range = sel_mdl_.cursors();
        if (std::ranges::empty(cursors_range)) {
            return;
        }

        auto pos = *std::ranges::begin(cursors_range);
        auto hpos = line_width(pos.line, pos.column);

        if (hpos < horizontal_pos() || hpos > horizontal_pos() + text_width()) {
            auto new_pos = hpos - text_width() / 2;
            if (new_pos < 0.0f) {
                new_pos = 0.0f;
            } else if (new_pos > max_horizontal_pos()) {
                new_pos = max_horizontal_pos();
            }

            set_horizontal_pos(new_pos);
        }
    }


    const TextModel & text_mdl_;                ///< Reference to text model
    selection_model_t & sel_mdl_;               ///< Reference to text selection model
    viewport_position_model_t & viewport_pos_;  ///< Reference to viewport position model

    float width_ = 0.0f;            ///< Viewport width in implementation defined units (pixels, characters)
    float height_ = 0.0f;           ///< Viewport height in implementation defined units (pixels, characters)
    
    /// Height that should be used for laying out text in viewport. Clients should set this value
    /// to maximum potential size of viewport ignoring currently visible additional elements such as
    /// horizontal toolbar.
    float max_height_ = 0.0f;

    float left_margin_ = 0.0f;      ///< Text left margin in viewport
    float right_margin_ = 0.0f;     ///< Text right margin in viewport
    float top_margin_ = 0.0f;       ///< Text top margin in viewport

    uint64_t vertical_pos_ = 0;     ///< Index of first visible line in viewport
    float horizontal_pos_ = 0.0f;   ///< Horizontal offset of viewport

    /// True if we are performing page up or page down action now
    bool is_performing_page_action_ = false;

    /// Connection to selection position change signal
    scoped_signal_connection pos_changed_con_;

    scoped_signal_connection after_erased_con_;
    scoped_signal_connection after_inserted_con_;
    scoped_signal_connection after_replaced_con_;

    scoped_signal_connection viewport_pos_changed_con_;
};


}
