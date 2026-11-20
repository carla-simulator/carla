// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/rpc/Metadata.h"

#include <rpc/client.h>
#include <rpc/rpc_error.h>

#include <algorithm>
#include <chrono>
#include <future>
#include <mutex>
#include <vector>

namespace carla {
namespace rpc {

  /// An RPC client that keeps a connection's calls in order on the server's
  /// game thread.
  ///
  /// The server decodes a connection's messages on several threads at once,
  /// so a fire-and-forget call could reach the game thread after a call made
  /// after it. The server answers a fire-and-forget call once its task is
  /// queued for the game thread, whose queue runs in order, so `call` (and
  /// `Fence`) first waits for those answers: a call that waits for its result
  /// always runs after every fire-and-forget call made before it.
  class Client {
  public:

    template <typename... Args>
    explicit Client(Args &&... args)
      : _client(std::forward<Args>(args)...) {}

    void set_timeout(int64_t value) {
      _client.set_timeout(value);
    }

    auto get_timeout() const {
      return _client.get_timeout();
    }

    template <typename... Args>
    auto call(const std::string &function, Args &&... args) {
      Fence();
      return _client.call(function, Metadata::MakeSync(), std::forward<Args>(args)...);
    }

    template <typename... Args>
    void async_call(const std::string &function, Args &&... args) {
      auto future = _client.async_call(function, Metadata::MakeAsync(), std::forward<Args>(args)...);
      std::lock_guard<std::mutex> lock(_pending_mutex);
      if (_pending.size() >= kPruneThreshold) {
        _pending.erase(
            std::remove_if(_pending.begin(), _pending.end(), [](const Pending &pending) {
              return pending.wait_for(std::chrono::seconds(0)) == std::future_status::ready;
            }),
            _pending.end());
      }
      _pending.emplace_back(std::move(future));
    }

    /// Waits until the server has queued every fire-and-forget call made so
    /// far. Their errors are not reported, as before. Throws ::rpc::timeout
    /// when the server does not answer within the client's timeout.
    void Fence() {
      std::vector<Pending> waiting;
      {
        std::lock_guard<std::mutex> lock(_pending_mutex);
        if (_pending.empty()) {
          return;
        }
        waiting.swap(_pending);
      }
      const auto timeout = _client.get_timeout();
      const auto deadline = std::chrono::steady_clock::now() +
          std::chrono::milliseconds(timeout ? *timeout : 0);
      for (size_t i = 0u; i < waiting.size(); ++i) {
        Pending &pending = waiting[i];
        if (timeout && (pending.wait_until(deadline) != std::future_status::ready)) {
          // Keep what is still on its way for the next fence.
          std::lock_guard<std::mutex> lock(_pending_mutex);
          _pending.insert(
              _pending.begin(),
              std::make_move_iterator(waiting.begin() + static_cast<std::ptrdiff_t>(i)),
              std::make_move_iterator(waiting.end()));
          throw ::rpc::timeout("the server did not queue earlier calls within the timeout");
        }
        try {
          pending.get();
        } catch (...) {
          // A fire-and-forget call's error was never reported.
        }
      }
    }

  private:

    using Pending = std::future<clmdep_msgpack::object_handle>;

    /// Fire-and-forget calls are dropped once answered when this many wait.
    static constexpr size_t kPruneThreshold = 256u;

    ::rpc::client _client;

    std::mutex _pending_mutex;

    std::vector<Pending> _pending;
  };

} // namespace rpc
} // namespace carla
