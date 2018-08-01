// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/Debug.h"
#include "carla/streaming/Message.h"
#include "carla/streaming/Token.h"
#include "carla/streaming/detail/StreamState.h"

#include <boost/asio/buffer.hpp>

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

    template <typename ConstBufferSequence>
    void Write(ConstBufferSequence buffer) {
      _shared_state->Write(std::make_shared<Message>(buffer));
    }

    template <typename T>
    Stream &operator<<(const T &rhs) {
      Write(boost::asio::buffer(rhs));
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
