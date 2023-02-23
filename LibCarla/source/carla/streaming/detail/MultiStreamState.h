// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/AtomicSharedPtr.h"
#include "carla/Logging.h"
#include "carla/streaming/detail/StreamStateBase.h"
#include "carla/streaming/detail/tcp/Message.h"

#include <mutex>
#include <vector>
#include <atomic>

namespace carla {
namespace streaming {
namespace detail {

  /// A stream state that can hold any number of sessions.
  ///
  /// @todo Lacking some optimization.
  class MultiStreamState final : public StreamStateBase {

    template <typename F>
    void ForEachSession(F&& fn) { /*TODO*/ }

  public:

    using StreamStateBase::StreamStateBase;

    MultiStreamState(const token_type &token) : 
      StreamStateBase(token),
      _mutex(),
      _sessions(),
      _force_active()
    {
    }

    template <typename... Buffers>
    void Write(Buffers &&... buffers) {
      std::lock_guard<std::mutex> lock(_mutex);
      auto message = Session::MakeMessage(std::forward<Buffers>(buffers)...);
      for (auto &s : _sessions) {
        if (s != nullptr) {
          s->Write(message);
          log_debug("sensor ", s->get_stream_id()," data sent ");
       }
      }
    }

    void ForceActive() {
      std::lock_guard<std::mutex> lock(_mutex);
      _force_active = true;
    }

    bool AreClientsListening() {
      std::lock_guard<std::mutex> lock(_mutex);
      return (_sessions.size() > 0 || _force_active);
    }

    void ConnectSession(std::shared_ptr<Session> session) final {
      log_debug("Connecting multistream sessions:", _sessions.size());
      DEBUG_ASSERT(session != nullptr);
      std::lock_guard<std::mutex> lock(_mutex);
      _sessions.emplace_back(std::move(session));
    }

    void DisconnectSession(std::shared_ptr<Session> session) final {
      DEBUG_ASSERT(session != nullptr);
      std::lock_guard<std::mutex> lock(_mutex);
      log_debug("Calling DisconnectSession for ", session->get_stream_id());
      if (_sessions.size() == 0) return;
      if (_sessions.size() == 1) {
        _sessions.clear();
        _force_active = false;
        log_debug("Last session disconnected");
      } else {
        _sessions.erase(std::remove(_sessions.begin(), _sessions.end(), session), _sessions.end());
      }
      log_debug("Disconnecting multistream sessions:", _sessions.size());
    }

    void ClearSessions() final {
      std::lock_guard<std::mutex> lock(_mutex);
      for (auto &s : _sessions) {
        if (s != nullptr) {
          s->Close();
        }
      }
      _sessions.clear();
      _force_active = false;
      log_debug("Disconnecting all multistream sessions");
    }

  private:

    std::mutex _mutex;
    std::vector<std::shared_ptr<Session>> _sessions;
    bool _force_active = false;
  };

} // namespace detail
} // namespace streaming
} // namespace carla
