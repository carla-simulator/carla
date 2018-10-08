// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/streaming/detail/Token.h"
#include "carla/streaming/detail/tcp/Client.h"

#include <boost/asio/io_service.hpp>

#include <unordered_map>

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

    using underlying_client = T;
    using protocol_type = typename underlying_client::protocol_type;
    using token_type = carla::streaming::detail::token_type;

    explicit Client(boost::asio::ip::address fallback_address)
      : _fallback_address(std::move(fallback_address)) {}

    explicit Client(const std::string &fallback_address)
      : Client(carla::streaming::make_address(fallback_address)) {}

    explicit Client()
      : Client(carla::streaming::make_localhost_address()) {}

    template <typename Functor>
    void Subscribe(
        boost::asio::io_service &io_service,
        token_type token,
        Functor &&callback) {
      if (!token.has_address()) {
        token.set_address(_fallback_address);
      }
      _clients.emplace(std::piecewise_construct,
          std::forward_as_tuple(token.get_stream_id()),
          std::forward_as_tuple(io_service, token, std::forward<Functor>(callback)));
    }

    void UnSubscribe(token_type token) {
      _clients.erase(token.get_stream_id());
    }

  private:

    boost::asio::ip::address _fallback_address;

    std::unordered_map<detail::stream_id_type, underlying_client> _clients;
  };

} // namespace low_level
} // namespace streaming
} // namespace carla
