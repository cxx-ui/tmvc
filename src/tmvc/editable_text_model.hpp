// Copyright (c) 2025, Alexandr Esilevich
//
// Distributed under the Boost Software License.
// See accompanying file LICENSE for license information.
//

/// \file editable_text_model.hpp
/// Contains definition of the editable_text_model concept.

#pragma once

#include "text_model.hpp"


namespace tmvc {


/// Represents editable text model that can be used with text_edit_controller.
template <typename TextModel>
concept editable_text_model = text_model<TextModel> && requires(TextModel & mdl) {
    // Inserts characters at specified position. Returns position range for inserted characters.
    { mdl.insert(std::declval<position>(), std::declval<std::vector<typename TextModel::char_t>>()) }
        -> std::convertible_to<range>;

    /// Deletes characters from specified range.
    mdl.erase(std::declval<range>());

    /// Replaces characters at specified position. All replaced characters must
    /// be in the same line. The newline character will not add new line and
    /// will be inserted into text as is.
    mdl.replace(std::declval<position>(), std::declval<std::vector<typename TextModel::char_t>>());
};


/// Replaces entire text in text model
template <editable_text_model TextModel>
void assign(TextModel & mdl, const text_model_string<TextModel> & t) {
    clear(mdl);
    mdl.insert({0, 0}, t);
}

/// Removes all text in text mode
void clear(editable_text_model auto & mdl) {
    mdl.erase({begin_pos(mdl), end_pos(mdl)});
}


}
