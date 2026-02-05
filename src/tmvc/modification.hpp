// Copyright (c) 2025, Alexandr Esilevich
//
// Distributed under the Boost Software License.
// See accompanying file LICENSE for license information.
//

/// \file modification.hpp
/// Contains definition of the modification_history class and related classes.

#pragma once

#include "characters_range.hpp"
#include "edit_controller.hpp"
#include "range.hpp"
#include "selection_controller.hpp"
#include "editable_text_model.hpp"
#include <list>
#include <memory>
#include <ranges>


namespace tmvc {


/// Represents user text modification action that can be undone or redone
template <typename Char>
class modification {
public:
    /// Default virtual destructor
    virtual ~modification() = default;
};


/// Base class for all single modifications. Contains information about modification range.
template <typename Char>
class range_modification: public modification<Char> {
public:
    /// Constructs modification with specified range
    range_modification(const range & r):
        range_{r} {}

    /// Returns modification range
    auto & modification_range() const { return range_; }

private:
    range range_;               ///< Modification range
};


/// Characters insert modification
template <typename Char>
class insert_modification: public range_modification<Char> {
public:
    /// Constructs insert modification with specified insert range
    /// and range of characters
    insert_modification(const range & r, characters_range<Char> auto && chars):
    range_modification<Char>{r} {
        auto c_chars = chars | std::ranges::views::common;
        chars_.assign(c_chars.begin(), c_chars.end());
    }

    /// Returns inserted characters
    auto & chars() const { return chars_; }

private:
    std::vector<Char> chars_;           ///< Inserted characters
};


/// Characters erase modification
template <typename Char>
class erase_modification: public range_modification<Char> {
public:
    /// Constructs delete action with specified delete range
    /// and range of characters
    erase_modification(const range & r, characters_range<Char> auto && chars):
    range_modification<Char>{r} {
        auto c_chars = chars | std::ranges::views::common;
        chars_.assign(c_chars.begin(), c_chars.end());
    }

    /// Returns erased characters
    auto & chars() const { return chars_; }

private:
    std::vector<Char> chars_;           ///< Erased characters
};


/// Characters replace modification
template <typename Char>
class replace_modification: public range_modification<Char> {
public:
    /// Constructs replace modification with specified range of replacement,
    /// and old and new ranges of characters
    replace_modification(const range & r,
                         characters_range<Char> auto && old_chars,
                         characters_range<Char> auto && new_chars):
    range_modification<Char>{r} {
        auto old_c_chars = old_chars | std::ranges::views::common;
        old_chars_.assign(old_c_chars.begin(), old_c_chars.end());

        auto new_c_chars = new_chars | std::ranges::views::common;
        new_chars_.assign(new_c_chars.begin(), new_c_chars.end());
    }

    /// Returns reference to characters before replacement
    auto & old_chars() const { return old_chars_; }

    /// Returns reference to characters after replacement
    auto & new_chars() const { return new_chars_; }

private:
    std::vector<Char> old_chars_;       ///< Characters before replacement
    std::vector<Char> new_chars_;       ///< Characters after replacement
};


/// Group of modifications that should be undoed/redoed as signle action
template <typename Char>
class modification_group: public modification<Char> {
public:
    /// Adds modification at the end of group
    void add(std::unique_ptr<modification<Char>> && mod) {
        mods_.push_back(std::move(mod));
    }

    /// Returns range of const modifications in the group
    auto modifications() const {
        auto fn = [](auto && uptr) {
            return static_cast<const modification<Char> *>(uptr.get());
        };
        return mods_ | std::ranges::views::transform(fn);
    }

private:
    /// Vector of modifications
    std::vector<std::unique_ptr<modification<Char>>> mods_;
};


/// Represents text document modification history. Stores list of text modifications
/// that can be undone or redone.
template <typename Char>
class modification_history {
public:
    /// Type of character
    using char_t = Char;

    /// Type of modification
    using modification_t = modification<char_t>;

    /// Type of unique pointer to modification
    using modification_ptr = std::unique_ptr<modification_t>;


    /// Constructs empty modification history
    modification_history() = default;


    /// Returns current undo modification
    const modification_t & current_undo() const {
        assert(can_undo() && "can't do undo at current state");
        return *undo_mods_.back();
    }


    /// Returns current redo modification
    const modification_t & current_redo() const {
        assert(can_redo() && "can't do redo at current state");
        return *redo_mods_.back();
    }


    /// Moves current undo modification to redo list
    void undo() {
        assert(can_undo() && "can't do undo at current state");

        // saving previous modification flags
        bool redo_was_empty = redo_mods_.empty();
        bool old_changed = changed();

        // extracting action from undo list
        auto action = std::move(undo_mods_.back());
        undo_mods_.pop_back();

        // adding action to redo list
        redo_mods_.push_back(std::move(action));

        // decrementing undo/redo count
        if (undo_redo_count_ != INT_MAX) {
            --undo_redo_count_;
        }


        // emitting signals for attributes change

        if (undo_mods_.empty()) {
            can_undo_changed();
        }

        if (redo_was_empty) {
            can_redo_changed();
        }

        if (old_changed != changed()) {
            changed_changed();
        }
    }


    /// Moves current redo modification to undo list
    void redo() {
        assert(can_redo() && "can't do redo at current state");

        // saving original values for modification attributes
        bool undo_was_empty = undo_mods_.empty();
        bool old_changed = changed();

        // extraction modification from redo list
        auto action = std::move(redo_mods_.back());
        redo_mods_.pop_back();

        // adding modification into undo list
        undo_mods_.push_back(std::move(action));

        // incrementing undo/redo count
        if (undo_redo_count_ != INT_MAX) {
            ++undo_redo_count_;
        }


        // emitting signals for changed attributes

        if (undo_was_empty) {
            can_undo_changed();
        }

        if (redo_mods_.empty()) {
            can_redo_changed();
        }

        if (old_changed != changed()) {
            changed_changed();
        }
    }


    /// Returns true if undo can be performed
    bool can_undo() const {
        return !undo_mods_.empty();
    }


    /// Returns true if redo can be performed
    bool can_redo() const {
        return !redo_mods_.empty();
    }


    /// Returns true if document was changed after last clearing changed flag
    bool changed() const {
        return undo_redo_count_ != 0;
    }


    /// Clears changed flag
    void clear_changed() {
        undo_redo_count_ = 0;
    }


    /// Completely clears modification history
    void clear() {
        undo_mods_.clear();
        redo_mods_.clear();
        clear_changed();

        can_undo_changed();
        can_redo_changed();
    }


    /// Adds modification and clears list of redo actions
    void add(modification_ptr && mod) {
    bool undo_changed = undo_mods_.empty();
    bool ch_changed = !changed();

    // adding undo action into list
    undo_mods_.push_back(std::move(mod));

    // checking for undo limit
    if (undo_mods_.size() > 1000) {
        undo_mods_.pop_front();
        assert(undo_mods_.size() == 1000 && "number of undo actions should be equal to limit");
    }

    // clearing redo actions
    bool redo_changed = !redo_mods_.empty();
        redo_mods_.clear();

        if (undo_redo_count_ == INT_MAX) {
            // doing nothing, original document version was removed from undo / redo chain
        } else if (undo_redo_count_ < 0) {
            // original document version was in redo action list and it's lost now
            undo_redo_count_ = INT_MAX;
        } else {
            ++undo_redo_count_;
        }

        // emitting signals

        if (undo_changed) {
            can_undo_changed();
        }

        if (redo_changed) {
            can_redo_changed();
        }

        if (ch_changed) {
            changed_changed();
        }
    }


    /// The signal is emitted after can_undo return values changed
    mutable signal<void ()> can_undo_changed;

    /// The signal is emitted after can_redo return values changed
    mutable signal<void ()> can_redo_changed;

    /// The signal is emitted after changed flag changes
    mutable signal<void ()> changed_changed;


private:
    /// List of modifications that can be undone
    std::list<modification_ptr> undo_mods_;

    /// List of modifications that can be redone
    std::list<modification_ptr> redo_mods_;

    /// Number of actions done after last clearing of changed flag
    /// Negative value means that undo was done after last clearing of changed flag
    /// INT_MAX values means that where is no original document version in undo/redo chain
    int undo_redo_count_ = 0;
};


/// Represents text modification transaction. Collects all modifications and
/// puts them in group before adding to modification history in destructor
template <editable_text_model TextModel>
class transaction {
public:
    /// Type of character
    using char_t = typename TextModel::char_t;

    /// Type of modification
    using modification_t = modification<char_t>;

    /// Type of unique pointer to modification
    using modification_ptr = std::unique_ptr<modification_t>;

    /// Type of modification history
    using modification_history_t = modification_history<char_t>;

    // Non copyable
    transaction(const transaction &) = delete;
    transaction & operator=(const transaction &) = delete;

    /// Constructs transaction for specified history
    transaction(TextModel & txt, modification_history_t & hist):
        text_{txt}, history_{hist}, group_{std::make_unique<modification_group<char_t>>()} {}

    /// Finishes transaction and adds grouped transaction into history
    ~transaction() {
        if (!std::ranges::empty(group_->modifications())) {
            history_.add(std::move(group_));
        }
    }


    /// Adds modification into transaction
    void add(modification_ptr && mod) {
        group_->add(std::move(mod));
    }


    /// Inserts characters into text model and adds modification into transaction.
    /// Returns range of inserted characters.
    range insert_characters(const position & pos, characters_range<char_t> auto && chars) {
        if (std::ranges::empty(chars)) {
            return {pos, pos};
        }

        auto ins_range = text_.insert(pos, chars);
        add(std::make_unique<insert_modification<char_t>>(ins_range, chars));
        return ins_range;
    }


    /// Erases characters from text model and adds modification into transcation
    void erase_characters(const range & r) {
        if (r.empty()) {
            return;
        }

        auto chars = characters_vector(text_, r);
        text_.erase(r);
        add(std::make_unique<erase_modification<char_t>>(r, chars));
    }

private:
    /// Reference to text model
    TextModel & text_;

    /// Reference to modification history
    modification_history_t & history_;

    /// Group for this transaction
    std::unique_ptr<modification_group<char_t>> group_;
};


}
