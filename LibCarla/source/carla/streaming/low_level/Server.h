// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/streaming/detail/Dispatcher.h"
#include "carla/streaming/detail/Types.h"
#include "carla/streaming/Stream.h"

#include <boost/asio/io_context.hpp>

namespace carla {
namespace streaming {
namespace low_level {

  /// A low-level streaming server. Each new stream has a token associated, this
  /// token can be used by a client to subscribe to the stream. This server
  /// requires an external io_context running.
  ///
  /// @warning This server cannot be destructed before its @a io_context is
  /// stopped.
  template <typename T>
  class Server {
  public:

    using underlying_server = T;

    using protocol_type = typename underlying_server::protocol_type;

    using token_type = carla::streaming::detail::token_type;

    using stream_id = carla::streaming::detail::stream_id_type;

    template <typename InternalEPType, typename ExternalEPType>
    explicit Server(
        boost::asio::io_context &io_context,
        detail::EndPoint<protocol_type, InternalEPType> internal_ep,
        detail::EndPoint<protocol_type, ExternalEPType> external_ep)
      : _server(io_context, std::move(internal_ep)),
        _dispatcher(std::move(external_ep)) {
      StartServer();
    }

    template <typename InternalEPType>
    explicit Server(
        boost::asio::io_context &io_context,
        detail::EndPoint<protocol_type, InternalEPType> internal_ep)
      : _server(io_context, std::move(internal_ep)),
        _dispatcher(make_endpoint<protocol_type>(_server.GetLocalEndpoint().port())) {
      StartServer();
    }

    template <typename... EPArgs>
    explicit Server(boost::asio::io_context &io_context, EPArgs &&... args)
      : Server(io_context, make_endpoint<protocol_type>(std::forward<EPArgs>(args)...)) {}

    typename underlying_server::endpoint GetLocalEndpoint() const {
      return _server.GetLocalEndpoint();
    }

    void SetTimeout(time_duration timeout) {
      _server.SetTimeout(timeout);
    }

    Stream MakeStream() {
      return _dispatcher.MakeStream();
    }

    void CloseStream(carla::streaming::detail::stream_id_type id) {
      return _dispatcher.CloseStream(id);
    }

    void SetSynchronousMode(bool is_synchro) {
      _server.SetSynchronousMode(is_synchro);
    }

    token_type GetToken(stream_id sensor_id) {
      return _dispatcher.GetToken(sensor_id);
    }

    void EnableForROS(stream_id sensor_id) {
      _dispatcher.EnableForROS(sensor_id);
    }

    void DisableForROS(stream_id sensor_id) {
      _dispatcher.DisableForROS(sensor_id);
    }

    bool IsEnabledForROS(stream_id sensor_id) {
      return _dispatcher.IsEnabledForROS(sensor_id);
    }

  private:

    void StartServer() {
      auto on_session_opened = [this](auto session) {
        if (!_dispatcher.RegisterSession(session)) {
          session->Close();
        }
      };
      auto on_session_closed = [this](auto session) {
        log_debug("on_session_closed called");
        _dispatcher.DeregisterSession(session);
      };
      _server.Listen(on_session_opened, on_session_closed);
    }

    underlying_server _server;

    detail::Dispatcher _dispatcher;
  };

} // namespace low_level
} // namespace streaming
} // namespace carla
