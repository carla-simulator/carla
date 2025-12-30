// Copyright (c) 2025 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/ThreadPool.h"
#include "carla/streaming/detail/tcp/Server.h"
#include "carla/streaming/detail/Types.h"
#include "carla/streaming/low_level/Server.h"

#include <boost/asio/io_context.hpp>

namespace carla {
namespace streaming {

  /// A streaming server. Each new stream has a token associated, this token can
  /// be used by a client to subscribe to the stream.
  class Server {
    using underlying_server = low_level::Server<detail::tcp::Server>;
    using protocol_type = low_level::Server<detail::tcp::Server>::protocol_type;
    using token_type = carla::streaming::detail::token_type;
    using stream_id = carla::streaming::detail::stream_id_type;
  public:

    explicit Server(uint16_t port)
      : _server(_pool.io_context(), make_endpoint<protocol_type>(port)) {}

    explicit Server(const std::string &address, uint16_t port)
      : _server(_pool.io_context(), make_endpoint<protocol_type>(address, port)) {}

    explicit Server(
        const std::string &address, uint16_t port,
        const std::string &external_address, uint16_t external_port)
      : _server(
          _pool.io_context(),
          make_endpoint<protocol_type>(address, port),
          make_endpoint<protocol_type>(external_address, external_port)) {}

    ~Server() {
      _pool.Stop();
    }

    auto GetLocalEndpoint() const {
      return _server.GetLocalEndpoint();
    }

    void SetTimeout(time_duration timeout) {
      _server.SetTimeout(timeout);
    }

    Stream MakeStream() {
      return _server.MakeStream();
    }

    void CloseStream(carla::streaming::detail::stream_id_type id) {
      return _server.CloseStream(id);
    }

    void Run() {
      _pool.Run();
    }

    void AsyncRun(size_t worker_threads) {
      _pool.AsyncRun(worker_threads);
    }

    void SetSynchronousMode(bool is_synchro) {
      _server.SetSynchronousMode(is_synchro);
    }

    token_type GetToken(stream_id sensor_id) {
      return _server.GetToken(sensor_id);
    }

    void SetROS2TopicVisibilityDefaultEnabled(bool _topic_visibility_default_enabled) {
      _server.SetROS2TopicVisibilityDefaultEnabled(_topic_visibility_default_enabled);
    }

    void EnableForROS(detail::stream_actor_id_type stream_actor_id) {
      _server.EnableForROS(stream_actor_id);
    }

    void DisableForROS(detail::stream_actor_id_type stream_actor_id) {
      _server.DisableForROS(stream_actor_id);
    }

    bool IsEnabledForROS(detail::stream_actor_id_type stream_actor_id) {
      return _server.IsEnabledForROS(stream_actor_id);
    }
    
    std::shared_ptr<detail::Dispatcher> GetDispatcher() const {
      return _server.GetDispatcher();
    }

  private:

    // The order of these two arguments is very important.

    ThreadPool _pool;

    underlying_server _server;
  };

} // namespace streaming
} // namespace carla
