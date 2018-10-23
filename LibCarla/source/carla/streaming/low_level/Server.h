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
  ///
  /// @warning This server cannot be destructed before its @a io_service is
  /// stopped.
  template <typename T>
  class Server {
  public:

    using underlying_server = T;

    using protocol_type = typename underlying_server::protocol_type;

    template <typename InternalEPType, typename ExternalEPType>
    explicit Server(
        boost::asio::io_service &io_service,
        detail::EndPoint<protocol_type, InternalEPType> internal_ep,
        detail::EndPoint<protocol_type, ExternalEPType> external_ep)
      : _server(io_service, std::move(internal_ep)),
        _dispatcher(std::move(external_ep)) {
      _server.Listen([this](auto session) {
        _dispatcher.RegisterSession(session);
      });
    }

    template <typename InternalEPType>
    explicit Server(
        boost::asio::io_service &io_service,
        detail::EndPoint<protocol_type, InternalEPType> internal_ep)
      : Server(io_service, internal_ep, make_endpoint<protocol_type>(internal_ep.port())) {}

    template <typename... EPArgs>
    explicit Server(boost::asio::io_service &io_service, EPArgs &&... args)
      : Server(io_service, make_endpoint<protocol_type>(std::forward<EPArgs>(args)...)) {}

    void SetTimeout(time_duration timeout) {
      _server.SetTimeout(timeout);
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
