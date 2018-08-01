// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/streaming/detail/Dispatcher.h"
#include "carla/streaming/Stream.h"

#include <boost/asio/io_service.hpp>

namespace carla {
namespace streaming {
namespace low_level {

  /// A low-level streaming server. Each new stream has a token associated, this
  /// token can be used by a client to subscribe to the stream. This server
  /// requires an external io_service running.
  template <typename T>
  class Server {
  public:

    using underlying_server = T;

    using endpoint = typename underlying_server::endpoint;
    using protocol_type = typename underlying_server::protocol_type;

    explicit Server(boost::asio::io_service &io_service, const endpoint &ep)
      : _server(io_service, ep),
        _dispatcher(ep) {
      _server.Listen([this](auto session){
        _dispatcher.RegisterSession(session);
      });
    }

    explicit Server(boost::asio::io_service &io_service, uint16_t port)
      : Server(io_service, endpoint(protocol_type::v4(), port)) {}

    explicit Server(boost::asio::io_service &io_service, const std::string &address, uint16_t port)
      : Server(io_service, endpoint(boost::asio::ip::address::from_string(address), port)) {}

    void set_timeout(time_duration timeout) {
      _server.set_timeout(timeout);
    }

    Stream MakeStream() {
      return _dispatcher.MakeStream();
    }

  private:

    underlying_server _server;

    detail::Dispatcher _dispatcher;
  };

} // namespace low_level
} // namespace streaming
} // namespace carla
