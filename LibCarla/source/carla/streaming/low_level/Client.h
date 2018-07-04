// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/streaming/low_level/Token.h"

#include <boost/asio/io_service.hpp>

#include <unordered_set>

namespace carla {
namespace streaming {
namespace low_level {

  /// Wrapper around low level clients. You can subscribe to multiple streams.
  ///
  /// @warning The client should not be destroyed before the @a io_service is
  /// stopped.
  /// @pre T has to be hashable.
  template <typename T>
  class Client {
  public:

    using underlying_client = T;

    template <typename Functor>
    void Subscribe(
        boost::asio::io_service &io_service,
        const token_type &token,
        Functor &&callback) {
      if (!token.protocol_is_tcp()) {
        throw std::invalid_argument("invalid token, only TCP tokens supported");
      }
      _clients.emplace(
          io_service,
          token.to_tcp_endpoint(),
          token.get_stream_id(),
          std::forward<Functor>(callback));
    }

  private:

    std::unordered_set<underlying_client> _clients;
  };

} // namespace low_level
} // namespace streaming
} // namespace carla
