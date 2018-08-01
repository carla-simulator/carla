// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/Logging.h"
#include "carla/ThreadGroup.h"
#include "carla/streaming/Token.h"
#include "carla/streaming/detail/tcp/Client.h"
#include "carla/streaming/low_level/Client.h"

#include <boost/asio/io_service.hpp>

namespace carla {
namespace streaming {

  using stream_token = detail::token_type;

  /// A client able to subscribe to multiple streams.
  class Client {
  public:

    Client() : _io_service(), _work_to_do(_io_service) {}

    ~Client() {
      Stop();
    }

    template <typename Functor>
    void Subscribe(const Token &token, Functor &&callback) {
      _client.Subscribe(_io_service, token, std::forward<Functor>(callback));
    }

    void Run() {
      _io_service.run();
    }

    void AsyncRun(size_t worker_threads) {
      _workers.CreateThreads(worker_threads, [this]() { Run(); });
    }

    void Stop() {
      _io_service.stop();
      _workers.JoinAll();
    }

  private:

    using underlying_client = low_level::Client<detail::tcp::Client>;

    boost::asio::io_service _io_service;

    boost::asio::io_service::work _work_to_do;

    ThreadGroup _workers;

    underlying_client _client;
  };

} // namespace streaming
} // namespace carla
