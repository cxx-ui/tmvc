// Copyright (c) 2025, Alexandr Esilevich
//
// Distributed under the Boost Software License.
// See accompanying file LICENSE for license information.
//

/// \file signals.hpp
/// Cotnains typedefs for signals

#pragma once

#include <boost/signals2/signal.hpp>


namespace tmvc {


template <typename Signature>
using signal = boost::signals2::signal<Signature>;

using scoped_signal_connection = boost::signals2::scoped_connection;


}
