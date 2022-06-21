// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/Logging.h"
#include "carla/ThreadPool.h"
#include "carla/streaming/Token.h"
#include "carla/streaming/detail/tcp/Client.h"
#include "carla/streaming/low_level/Client.h"

#include <boost/asio/io_context.hpp>

namespace carla {
namespace streaming {

  using stream_token = detail::token_type;

  /// A client able to subscribe to multiple streams.
  class Client {
    using underlying_client = low_level::Client<detail::tcp::Client>;
  public:

    Client() = default;

    explicit Client(const std::string &fallback_address)
      : _client(fallback_address) {}

    ~Client() {
      _service.Stop();
    }

    /// @warning cannot subscribe twice to the same stream (even if it's a
    /// MultiStream).
    template <typename Functor>
    void Subscribe(const Token &token, Functor &&callback) {
      _client.Subscribe(_service.io_context(), token, std::forward<Functor>(callback));
    }

    void UnSubscribe(const Token &token) {
      _client.UnSubscribe(token);
    }

    void Run() {
      _service.Run();
    }

    void AsyncRun(size_t worker_threads) {
      _service.AsyncRun(worker_threads);
    }

  private:

    // The order of these two arguments is very important.

    ThreadPool _service;

    underlying_client _client;
  };

} // namespace streaming
} // namespace carla
