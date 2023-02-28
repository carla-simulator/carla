// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/streaming/detail/MultiStreamState.h"
#include "carla/streaming/detail/Stream.h"
#include "carla/streaming/detail/StreamState.h"

namespace carla {
namespace streaming {

  /// A stream represents an unidirectional channel for sending data from server
  /// to client. A **single** client can subscribe to this stream using the
  /// stream token. If no client is subscribed, the data flushed down the stream
  /// is discarded.
  using Stream = detail::Stream<detail::StreamState>;

  /// A stream represents an unidirectional channel for sending data from server
  /// to client. Multiple clients can subscribe to this stream using the stream
  /// token. If no client is subscribed, the data flushed down the stream is
  /// discarded.
  ///
  /// @warning MultiStream is quite slower than Stream.
  using MultiStream = detail::Stream<detail::MultiStreamState>;

} // namespace streaming
} // namespace carla
