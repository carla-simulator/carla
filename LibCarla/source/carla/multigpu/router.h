// Copyright (c) 2022 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

// #include "carla/Logging.h"
#include "carla/streaming/detail/tcp/Message.h"
#include "carla/ThreadPool.h"

#include <boost/asio/io_context.hpp>
#include <boost/asio/ip/tcp.hpp>

#include <mutex>
#include <vector>
#include <sstream>

namespace carla {
namespace multigpu {

  class Primary;
  class Listener;

  class Router {
  public:

    Router(void);
    ~Router();
    explicit Router(uint16_t port);

    template <typename... Buffers>
    void Write(Buffers &&... buffers) {
      auto message = Primary::MakeMessage(std::move(buffers)...);
      log_info("Writting to multi-gpu servers: ", message->size());
      
      // write to multiple servers
      std::lock_guard<std::mutex> lock(_mutex);
      for (auto &s : _sessions) {
        if (s != nullptr) {
          s->Write(message);
        }
      }
    }

    template <typename... Buffers>
    void WriteToNext(Buffers &&... buffers) {
      auto message = Primary::MakeMessage(std::move(buffers)...);
      log_info("Writting to next multi-gpu server: ", message->size());
      
      // write to the next server only
      std::lock_guard<std::mutex> lock(_mutex);
      if (_next >= _sessions.size()) {
        _next = 0;
      }
      if (_next < _sessions.size()) {
        auto s = _sessions[_next];
        if (s != nullptr) {
          s->Write(message);
        }
      }
      ++_next;
    }

    void AsyncRun(size_t worker_threads);

    boost::asio::ip::tcp::endpoint GetLocalEndpoint() const;

    bool IsMultiGPU() {
      return (!_sessions.empty());
    }

  private:
    void ConnectSession(std::shared_ptr<Primary> session);

    void DisconnectSession(std::shared_ptr<Primary> session);

    void ClearSessions();

    
    std::mutex                              _mutex;
    ThreadPool                              _pool;
    boost::asio::ip::tcp::endpoint          _endpoint;
    std::vector<std::shared_ptr<Primary>>   _sessions;
    std::shared_ptr<Listener>               _listener;
    int                                     _next;
  };

} // namespace multigpu
} // namespace carla
