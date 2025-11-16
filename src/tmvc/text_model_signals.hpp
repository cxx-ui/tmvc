// Copyright (c) 2025, Alexandr Esilevich
//
// Distributed under the Boost Software License.
// See accompanying file LICENSE for license information.
//

/// \file text_model_signals.hpp
/// Contains definition of the text_model_signals class.

#pragma once

#include "range.hpp"
#include "signals.hpp"


namespace tmvc {


/// Contains definition of signals fields for text model
class text_model_signals {
public:
    /// The signal is emitted before characters are inserted in range
    mutable signal<void (const range &)> before_inserted;

    /// The signal is emitted after characters are inserted in range
    mutable signal<void (const range &)> after_inserted;

    /// Additional signal that is emitted after the after_inserted signal. It is used
    /// for implementing advanced logic when users need to execute some actions
    /// after all clients processed the after_inserted signal.
    mutable signal<void (const range &)> after_inserted_2;

    /// The signal is emiited before characters are removed in specified range
    mutable signal<void (const range &)> before_erased;

    /// The signal is emiited after characters are removed in specified range
    mutable signal<void (const range &)> after_erased;

    /// Additional signal that is emitted after the after_erased signal. It is used
    /// for implementing advanced logic when users need to execute some actions
    /// after all clients processed the after_inserted signal.
    mutable signal<void (const range &)> after_erased_2;

    /// The signal is emitted before characters are replaced in specified range
    mutable signal<void (const range &)> before_replaced;

    /// The signal is emitted after characters are replaced in specified range
    mutable signal<void (const range &)> after_replaced;
};


}
