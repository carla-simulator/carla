// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/Buffer.h"
#include "carla/Debug.h"
#include "carla/streaming/Token.h"

#include <memory>

namespace carla {
namespace streaming {
namespace detail {

  class Dispatcher;

  template <typename StreamStateT>
  class Stream {
  public:

    Stream() = delete;

    Stream(const Stream &) = default;
    Stream(Stream &&) = default;

    Stream &operator=(const Stream &) = default;
    Stream &operator=(Stream &&) = default;

    /// Token associated with this stream. This token can be used by a client to
    /// subscribe to this stream.
    Token token() const {
      return _shared_state->token();
    }

    /// Pull a buffer from the buffer pool associated to this stream. Discarded
    /// buffers are re-used to avoid memory allocations.
    ///
    /// @note Re-using buffers is optimized for the use case in which all the
    /// messages sent through the stream are big and have (approximately) the
    /// same size.
    Buffer MakeBuffer() {
      auto state = _shared_state;
      return state->MakeBuffer();
    }

    /// Flush @a buffers down the stream. No copies are made.
    template <typename... Buffers>
    void Write(Buffers &&... buffers) {
      _shared_state->Write(std::move(buffers)...);
    }

    /// Make a copy of @a data and flush it down the stream.
    template <typename T>
    Stream &operator<<(const T &data) {
      auto buffer = MakeBuffer();
      buffer.copy_from(data);
      Write(std::move(buffer));
      return *this;
    }

    bool AreClientsListening()
    {
      return _shared_state ? _shared_state->AreClientsListening() : false;
    }

  private:

    friend class detail::Dispatcher;

    Stream(std::shared_ptr<StreamStateT> state)
      : _shared_state(std::move(state)) {
      DEBUG_ASSERT(_shared_state != nullptr);
    }

    std::shared_ptr<StreamStateT> _shared_state;
  };

} // namespace detail
} // namespace streaming
} // namespace carla
