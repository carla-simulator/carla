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
#include "carla/multigpu/primaryCommands.h"
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

  class Router : public std::enable_shared_from_this<Router> {
  public:

    Router(void);
    explicit Router(uint16_t port);
    ~Router();
    
    void Write(MultiGPUCommand id, Buffer &&buffer);
    std::future<SessionInfo> WriteToNext(MultiGPUCommand id, Buffer &&buffer);
    void Stop();

    void SetCallbacks();
    void SetNewConnectionCallback(std::function<void(void)>);

    void AsyncRun(size_t worker_threads);

    boost::asio::ip::tcp::endpoint GetLocalEndpoint() const;

    bool HasClientsConnected() {
      return (!_sessions.empty());
    }

    PrimaryCommands &GetCommander() {
      return _commander;
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
    uint32_t                                _next;
    std::unordered_map<Primary *, std::shared_ptr<std::promise<SessionInfo>>>   _promises;
    PrimaryCommands                         _commander;
    std::function<void(void)>               _callback;
  };

} // namespace multigpu
} // namespace carla
