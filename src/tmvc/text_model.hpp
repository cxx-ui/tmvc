// Copyright (c) 2025, Alexandr Esilevich
//
// Distributed under the Boost Software License.
// See accompanying file LICENSE for license information.
//

/// \file text_model.hpp
/// Contains definition of text model concepts and related functions.

#pragma once

#include "position.hpp"
#include "range.hpp"
#include "signals.hpp"
#include "std_character.hpp"
#include "text_data.hpp"
#include "text_model_signals.hpp"
#include <cctype>
#include <cwctype>
#include <iterator>
#include <ranges>
#include <string>


namespace tmvc {


/// Type of range signals used in text model
using text_model_range_signal = signal<void (const range &)>;


/// Represents read only text model
template <typename TextModel>
concept text_model = text_data<TextModel> && requires(const TextModel & mdl) {
    // The signal is emitted before characters are inserted in range
    { mdl.before_inserted } -> std::convertible_to<text_model_range_signal &>;

    // The signal is emitted after characters are inserted in range
    { mdl.after_inserted } -> std::convertible_to<text_model_range_signal &>;

    /// Additional signal that is emitted after the after_inserted signal. It is used
    /// for implementing advanced logic when users need to execute some actions
    /// after all clients processed the after_inserted signal.
    { mdl.after_inserted_2 } -> std::convertible_to<text_model_range_signal &>;

    /// The signal is emitted before characters are removed in specified range
    { mdl.before_erased } -> std::convertible_to<text_model_range_signal &>;

    /// The signal is emitted after characters are removed in specified range
    { mdl.after_erased } -> std::convertible_to<text_model_range_signal &>;

    /// Additional signal that is emitted after the after_erased signal. It is used
    /// for implementing advanced logic when users need to execute some actions
    /// after all clients processed the after_inserted signal.
    { mdl.after_erased_2 } -> std::convertible_to<text_model_range_signal &>;

    /// The signal is emitted before characters are replaced in specified range
    { mdl.before_replaced } -> std::convertible_to<text_model_range_signal &>;

    /// The signal is emitted after characters are replaced in specified range
    { mdl.after_replaced } -> std::convertible_to<text_model_range_signal &>;
};


/// Text model with known maximum line size
template <typename TextModel>
concept text_model_with_max_line_size = text_model<TextModel> &&
                                        text_data_with_max_line_size<TextModel>;


/// Alias for type of std string for characters in text model
template <text_model TextModel>
requires std_character<typename TextModel::char_t>
using text_model_string = std::basic_string<typename TextModel::char_t>;


}
