// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/streaming/detail/HashableClient.h"
#include "carla/streaming/detail/Token.h"
#include "carla/streaming/detail/tcp/Client.h"

#include <boost/asio/io_service.hpp>

#include <unordered_set>

namespace carla {
namespace streaming {
namespace low_level {

  /// A client able to subscribe to multiple streams. Accepts an external
  /// io_service.
  ///
  /// @warning The client should not be destroyed before the @a io_service is
  /// stopped.
  template <typename T>
  class Client {
  public:

    using underlying_client = detail::HashableClient<T>;

    using token_type = carla::streaming::detail::token_type;

    template <typename Functor>
    void Subscribe(
        boost::asio::io_service &io_service,
        const token_type &token,
        Functor &&callback) {
      if (!token.protocol_is_tcp()) { /// @todo
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
