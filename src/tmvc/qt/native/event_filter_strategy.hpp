// Copyright (c) 2025, Alexandr Esilevich
//
// Distributed under the Boost Software License.
// See accompanying file LICENSE for license information.
//

/// \file event_filter_strategy.hpp
/// Contains definition of the event_filter_strategy class.

#pragma once

#include "core_view.hpp"
#include <memory>
#include <type_traits>


namespace tmvc::qt::native {


/// Plain text view strategy that installs event filter of specified type
template <typename EventFilter>
class event_filter_strategy {
public:
    /// Constructs strategy from arguments which are passed to event filter constructor
    template <typename ... Args>
    requires (std::constructible_from<EventFilter, Args...>)
    event_filter_strategy(Args && ... args):
        filter_(std::make_unique<EventFilter>(args...)) {}

    /// Initializes event filter and installs it into text view
    void initialize(plain_text_view_base * view) {
        filter_->initialize(view);
        view->installEventFilter(filter_.get());
    }

private:
    std::unique_ptr<EventFilter> filter_;       ///< Event filter instance
};


}
