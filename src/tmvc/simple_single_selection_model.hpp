// Copyright (c) 2025, Alexandr Esilevich
//
// Distributed under the Boost Software License.
// See accompanying file LICENSE for license information.
//

/// \file simple_single_selection_model.hpp
/// Contains definition of the simple_single_selection_model.

#pragma once

#include "position.hpp"
#include "range.hpp"
#include "selection_model.hpp"
#include "signals.hpp"
#include "text_model.hpp"


namespace tmvc {


/// Stores state of selection in a text model.
/// Allows only single selection (with anchor position and current position).
template <text_model TextModel>
class simple_single_selection_model {
public:
    /// Constructs text selection model with specified reference to text model
    simple_single_selection_model(const TextModel & txt):
    text_{txt} {
        selections_.reserve(1);

        after_inserted_2_con_ = text_.after_inserted_2.connect([this](const range & r) {
            on_after_inserted(r);
        });

        after_erased_2_con_ = text_.after_erased_2.connect([this](const range & r) {
            on_after_erased(r);
        });
    }


    /// Returns current text position
    const position & pos() const {
        return pos_;
    }


    /// Returns current selection anchor posotion
    const position & anchor_pos() const {
        return anchor_pos_;
    }


    /// Sets current text position and selection anchor position
    void set_pos_and_anchor(const position & a_pos, const position & pos) {
        assert(pos_is_valid(text_, pos) && "text position is not valid for text model");
        assert(pos_is_valid(text_, a_pos) && "anchor position is not valid for text model");

        if (pos == pos_ && a_pos == anchor_pos_) {
            return;
        }

        pos_ = pos;
        anchor_pos_ = a_pos;

        selections_.clear();
        if (pos_ < anchor_pos_) {
            selections_.push_back(range{pos_, anchor_pos_});
        } else if (pos_ > anchor_pos_) {
            selections_.push_back(range{anchor_pos_, pos_});
        }

        changed();
    }


    /// Sets anchor position
    void set_anchor_pos(const position & a_pos) {
        set_pos_and_anchor(a_pos, pos());
    }


    /// Sets current text position and keeps selection anchor position
    void set_pos_keep_anchor(const position & pos) {
        set_pos_and_anchor(anchor_pos(), pos);
    }


    /// Sets current text position and selection anchor position to same values
    void set_pos_move_anchor(const position & pos) {
        set_pos_and_anchor(pos, pos);
    }


    /// Sets move position after insertion flag. If flag is set then selection model
    /// moves position after inserted text if current position is equal to insert position.
    void set_move_pos_after_insert(bool val) {
        move_pos_after_insert_ = val;
    }


    /// Returns range of cursors
    auto cursors() const {
        return std::ranges::views::single(pos());
    }


    /// Returns range of selections
    const auto & selections() const {
        return selections_;
    }


    /// The signal which is emiited when current position of anchor position is changed
    mutable signal<void ()> changed;

private:
    /// Handler which is called after characters are inserted in text model
    void on_after_inserted(const range & r) {
        // changing current position in document if it was after or equal
        // to insert postion

        auto new_pos = pos();
        auto new_anchor_pos = anchor_pos(); 

        if (r.start < new_pos || (r.start == new_pos && move_pos_after_insert_)) {
            new_pos = adjust_pos_after_insert(new_pos, r, move_pos_after_insert_);

            if (new_anchor_pos < r.start) {
                new_anchor_pos = new_pos;
            } else {
                // same as for pos_
                new_anchor_pos = adjust_pos_after_insert(new_anchor_pos, r);
            }
        } else {
            if (new_anchor_pos > r.start) {
                new_anchor_pos = new_pos;
            }
        }

        set_pos_and_anchor(new_anchor_pos, new_pos);
    }


    /// Handler which is called before characters are removed from text model
    void on_after_erased(const range & r) {
        auto new_pos = pos();
        auto new_anchor_pos = anchor_pos();

        // we need change current position according to its location
        // relative to delete range
        if (new_pos > r.end) {
            // current position is located after delete range
            new_pos = adjust_pos_after_erase(new_pos, r);

            if (new_anchor_pos < r.end) {
                new_anchor_pos = new_pos;
            } else {
                new_anchor_pos = adjust_pos_after_erase(new_anchor_pos, r);
            }
        } else if (r.start < new_pos) {
            // current position is located inside delete range
            new_pos = r.start;
            new_anchor_pos = r.start;
        } else {
            // current position is located before delete range and
            // should not be changed, but we have to change anchor pos
            // if it is located not before delete range
            if (new_anchor_pos > r.start) {
                new_anchor_pos = new_pos;
            }
        }

        set_pos_and_anchor(new_anchor_pos, new_pos);
    }


    const TextModel & text_;                ///< Reference to text model
    position pos_;                          ///< Current text position
    position anchor_pos_;                   ///< Selection anchor position
    bool move_pos_after_insert_ = true;     ///< Move position after insertion flag

    /// Vector of selections. Contains zero or single item. Used in the selections()
    /// member function to emulate multi selection model.
    std::vector<range> selections_;

    scoped_signal_connection after_inserted_2_con_;
    scoped_signal_connection after_erased_2_con_;
};


}
