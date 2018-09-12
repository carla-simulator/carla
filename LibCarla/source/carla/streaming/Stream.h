// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/Buffer.h"
#include "carla/Debug.h"
#include "carla/streaming/Token.h"
#include "carla/streaming/detail/StreamState.h"

#include <memory>

namespace carla {
namespace streaming {

namespace detail {

  class Dispatcher;

} // namespace detail

  class Stream {
  public:

    Stream() = delete;

    Stream(const Stream &) = default;
    Stream(Stream &&) = default;

    Stream &operator=(const Stream &) = default;
    Stream &operator=(Stream &&) = default;

    Token token() const {
      return _shared_state->token();
    }

    /// Flush @a buffers down the stream. No copies are made.
    template <typename... Buffers>
    void Write(Buffers... buffers) {
      _shared_state->Write(std::move(buffers)...);
    }

    /// Make a copy of @a data and flush it down the stream.
    template <typename T>
    Stream &operator<<(const T &data) {
      Write(Buffer(data));
      return *this;
    }

  private:

    friend class detail::Dispatcher;

    Stream(std::shared_ptr<detail::StreamState> state)
      : _shared_state(std::move(state)) {
      DEBUG_ASSERT(_shared_state != nullptr);
    }

    std::shared_ptr<detail::StreamState> _shared_state;
  };

} // namespace streaming
} // namespace carla
