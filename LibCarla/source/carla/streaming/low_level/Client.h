// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/streaming/detail/Token.h"
#include "carla/streaming/detail/tcp/Client.h"

#include <boost/asio/io_context.hpp>

#include <memory>
#include <unordered_map>

namespace carla {
namespace streaming {
namespace low_level {

  /// A client able to subscribe to multiple streams. Accepts an external
  /// io_context.
  ///
  /// @warning The client should not be destroyed before the @a io_context is
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

    ~Client() {
      for (auto &pair : _clients) {
        pair.second->Stop();
      }
    }

    /// @warning cannot subscribe twice to the same stream (even if it's a
    /// MultiStream).
    template <typename Functor>
    void Subscribe(
        boost::asio::io_context &io_context,
        token_type token,
        Functor &&callback) {
      DEBUG_ASSERT_EQ(_clients.find(token.get_stream_id()), _clients.end());
      if (!token.has_address()) {
        token.set_address(_fallback_address);
      }
      auto client = std::make_shared<underlying_client>(
          io_context,
          token,
          std::forward<Functor>(callback));
      client->Connect();
      _clients.emplace(token.get_stream_id(), std::move(client));
    }

    void UnSubscribe(token_type token) {
      log_warning("calling sensor UnSubscribe()");
      auto it = _clients.find(token.get_stream_id());
      if (it != _clients.end()) {
        it->second->Stop();
        _clients.erase(it);
      }
    }

  private:

    boost::asio::ip::address _fallback_address;

    std::unordered_map<
        detail::stream_id_type,
        std::shared_ptr<underlying_client>> _clients;
  };

} // namespace low_level
} // namespace streaming
} // namespace carla
