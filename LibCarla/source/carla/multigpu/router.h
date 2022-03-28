// Copyright (c) 2022 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

// #include "carla/Logging.h"
#include "carla/streaming/detail/tcp/Message.h"
#include "carla/ThreadPool.h"
#include "carla/multigpu/primary.h"
#include "carla/multigpu/commands.h"

#include <boost/asio/io_context.hpp>
#include <boost/asio/ip/tcp.hpp>

#include <mutex>
#include <vector>
#include <sstream>
#include <unordered_map>

namespace carla {
namespace multigpu {

  // class Primary;
  class Listener;

  struct SessionInfo {
    std::shared_ptr<Primary>  session;
    carla::Buffer             buffer;
  };

  struct CommandHeader {
    MultiGPUCommand id;
    uint32_t size;
  };

  class Router : public std::enable_shared_from_this<Router> {
  public:

    Router(void);
    explicit Router(uint16_t port);
    ~Router();

    void Write(MultiGPUCommand id, Buffer &&buffer);
    std::future<SessionInfo> WriteToNext(MultiGPUCommand id, Buffer &&buffer);

    void SetCallbacks();
    void AsyncRun(size_t worker_threads);

    boost::asio::ip::tcp::endpoint GetLocalEndpoint() const;

    bool IsMultiGPU() {
      return (!_sessions.empty());
    }

  private:
    void ConnectSession(std::shared_ptr<Primary> session);
    void DisconnectSession(std::shared_ptr<Primary> session);
    void ClearSessions();
    
    // mutex and thread pool must be at the beginning to be destroyed last
    std::mutex                              _mutex;
    ThreadPool                              _pool;
    boost::asio::ip::tcp::endpoint          _endpoint;
    std::vector<std::shared_ptr<Primary>>   _sessions;
    std::shared_ptr<Listener>               _listener;
    int                                     _next;
    std::unordered_map<Primary *, std::shared_ptr<std::promise<SessionInfo>>>   _promises;
  };

} // namespace multigpu
} // namespace carla
