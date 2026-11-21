// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/Buffer.h"

#include <functional>
#include <type_traits>
#include <utility>

namespace carla {
namespace streaming {

  /// What a stream callback asks of its connection after a message.
  enum class CallbackResult {
    /// Read the next message.
    Continue,
    /// The callback cannot use what follows on this connection (it lost the
    /// state the next messages build on): reconnect, so that the server
    /// starts a new session from a self-contained message.
    Resync
  };

  using StreamCallback = std::function<CallbackResult (Buffer)>;

  /// Wraps a callback that returns nothing (sensors) into a StreamCallback.
  template <typename Functor>
  StreamCallback MakeStreamCallback(Functor &&callback) {
    using result_type = std::invoke_result_t<std::decay_t<Functor> &, Buffer>;
    if constexpr (std::is_same_v<result_type, CallbackResult>) {
      return StreamCallback(std::forward<Functor>(callback));
    } else {
      return [callback = std::forward<Functor>(callback)](Buffer buffer) mutable {
        callback(std::move(buffer));
        return CallbackResult::Continue;
      };
    }
  }

} // namespace streaming
} // namespace carla
