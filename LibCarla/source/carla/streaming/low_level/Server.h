// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/streaming/low_level/Dispatcher.h"
#include "carla/streaming/Stream.h"

#include <boost/asio/io_service.hpp>

namespace carla {
namespace streaming {
namespace low_level {

  /// Wrapper around low level servers.
  template <typename T>
  class Server {
  public:

    using underlying_server = T;

    using duration_type = typename underlying_server::duration_type;
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

    void set_timeout(duration_type timeout) {
      _server.set_timeout(timeout);
    }

    Stream MakeStream() {
      return _dispatcher.MakeStream();
    }

  private:

    underlying_server _server;

    Dispatcher _dispatcher;
  };

} // namespace low_level
} // namespace streaming
} // namespace carla
