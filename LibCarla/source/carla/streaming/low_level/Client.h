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
      _clients.emplace(
          io_service,
          token,
          std::forward<Functor>(callback));
    }

  private:

    boost::asio::ip::address _fallback_address;

    std::unordered_set<underlying_client> _clients;
  };

} // namespace low_level
} // namespace streaming
} // namespace carla
