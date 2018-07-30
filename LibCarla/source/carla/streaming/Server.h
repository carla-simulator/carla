// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/ThreadGroup.h"
#include "carla/streaming/low_level/Server.h"
#include "carla/streaming/detail/tcp/Server.h"

#include <boost/asio/io_service.hpp>

namespace carla {
namespace streaming {

  /// A streaming server. Each new stream has a token associated, this token can
  /// be used by a client to subscribe to the stream.
  class Server {
    using underlying_server = low_level::Server<detail::tcp::Server>;
  public:

    explicit Server(uint16_t port)
      : _server(_io_service, port) {}

    explicit Server(const std::string &address, uint16_t port)
      : _server(_io_service, address, port) {}

    ~Server() {
      Stop();
    }

    void set_timeout(time_duration timeout) {
      _server.set_timeout(timeout);
    }

    Stream MakeStream() {
      return _server.MakeStream();
    }

    void Run() {
      _io_service.run();
    }

    void AsyncRun(size_t worker_threads) {
      _workers.CreateThreads(worker_threads, [this](){ Run(); });
    }

    void Stop() {
      _io_service.stop();
      _workers.JoinAll();
    }

  private:

    boost::asio::io_service _io_service;

    underlying_server _server;

    ThreadGroup _workers;
  };

} // namespace streaming
} // namespace carla
