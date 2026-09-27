// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma
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
#include <string_view>
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
    std::future<SessionInfo> WriteToOne(std::weak_ptr<Primary> server, MultiGPUCommand id, Buffer &&buffer);
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

#ifdef LIBCARLA_WITH_GTEST
    // Test-only access to the session bookkeeping, bypassing the real
    // Listener accept flow so the promise-fulfillment logic can be
    // unit-tested without a live TCP connection.
    void TestConnectSession(std::shared_ptr<Primary> session) {
      ConnectSession(std::move(session));
    }
    void TestDisconnectSession(std::shared_ptr<Primary> session) {
      DisconnectSession(std::move(session));
    }
    // Test-only: invokes the same response-handling path SetCallbacks()
    // wires to the Listener, so the resync-on-episode-ready contract can be
    // exercised without a live secondary.
    void TestHandleResponse(std::shared_ptr<Primary> session, Buffer buffer) {
      HandleResponse(std::move(session), std::move(buffer));
    }
#endif // LIBCARLA_WITH_GTEST

  private:
    void ConnectSession(std::shared_ptr<Primary> session);
    void DisconnectSession(std::shared_ptr<Primary> session);
    void ClearSessions();

    /// Re-arms the new-connection resync if @a buffer is the secondary's
    /// episode-ready marker (checked first, so it can never be misdelivered
    /// to a pending promise on the same session); otherwise resolves the
    /// pending promise for @a session with @a buffer, or logs and drops the
    /// data if no request is currently pending on it.
    void HandleResponse(std::shared_ptr<Primary> session, Buffer buffer);

    /// Fails a pending request instead of leaving its future unresolved.
    /// A session can vanish (dead weak_ptr, disconnect, empty router) between
    /// the moment a promise is created and the moment a response would have
    /// arrived; without this, the caller's std::future::get() blocks forever.
    static void RejectPromise(
        std::shared_ptr<std::promise<SessionInfo>> promise,
        std::string_view reason);

    /// Rejects and clears every currently-pending promise. Caller must
    /// already hold _mutex.
    void RejectAllPending(std::string_view reason);

    // mutex and thread pool must be at the beginning to be destroyed last
    std::mutex                              _mutex;
    ThreadPool                              _pool;
    boost::asio::ip::tcp::endpoint          _endpoint;
    std::vector<std::shared_ptr<Primary>>   _sessions;
    std::shared_ptr<Listener>               _listener;
    uint32_t                                _next;
    std::unordered_map<Primary *, std::shared_ptr<std::promise<SessionInfo>>> _promises;
    PrimaryCommands                         _commander;
    std::function<void(void)>               _callback;
  };

} // namespace multigpu
} // namespace carla
