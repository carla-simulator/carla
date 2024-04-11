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
  public:

    using StreamStateBase::StreamStateBase;

    MultiStreamState(const token_type &token) :
      StreamStateBase(token),
      _session(nullptr)
      {};

    template <typename... Buffers>
    void Write(Buffers... buffers) {
      // try write single stream
      auto session = _session.load();
      if (session != nullptr) {
        auto message = Session::MakeMessage(buffers...);
        session->Write(std::move(message));
        log_debug("sensor ", session->get_stream_id()," data sent");
        // Return here, _session is only valid if we have a
        // single session.
        return;
      }

      // try write multiple stream
      std::lock_guard<std::mutex> lock(_mutex);
      if (_sessions.size() > 0) {
        auto message = Session::MakeMessage(buffers...);
        for (auto &s : _sessions) {
          if (s != nullptr) {
            s->Write(message);
            log_debug("sensor ", s->get_stream_id()," data sent ");
         }
        }
      }
    }

    void ForceActive() {
      _force_active = true;
    }

    void EnableForROS() {
      _enabled_for_ros = true;
    }

    void DisableForROS() {
      _enabled_for_ros = false;
    }

    bool IsEnabledForROS() {
      return _enabled_for_ros;
    }

    bool AreClientsListening() {
      return (_sessions.size() > 0 || _force_active || _enabled_for_ros);
    }

    void ConnectSession(std::shared_ptr<Session> session) final {
      DEBUG_ASSERT(session != nullptr);
      std::lock_guard<std::mutex> lock(_mutex);
      _sessions.emplace_back(std::move(session));
      log_debug("Connecting multistream sessions:", _sessions.size());
      if (_sessions.size() == 1) {
        _session.store(_sessions[0]);
      }
      else if (_sessions.size() > 1) {
        _session.store(nullptr);
      }
    }

    void DisconnectSession(std::shared_ptr<Session> session) final {
      DEBUG_ASSERT(session != nullptr);
      std::lock_guard<std::mutex> lock(_mutex);
      log_debug("Calling DisconnectSession for ", session->get_stream_id());
      if (_sessions.size() == 0) return;
      if (_sessions.size() == 1) {
        DEBUG_ASSERT(session == _session.load());
        _session.store(nullptr);
        _sessions.clear();
        _force_active = false;
        log_debug("Last session disconnected");
      } else {
        _sessions.erase(
            std::remove(_sessions.begin(), _sessions.end(), session),
            _sessions.end());

        // set single session if only one
        if (_sessions.size() == 1)
          _session.store(_sessions[0]);
        else
          _session.store(nullptr);
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
      _session.store(nullptr);
      log_debug("Disconnecting all multistream sessions");
    }

  private:

    std::mutex _mutex;

    // if there is only one session, then we use atomic
    AtomicSharedPtr<Session> _session;
    // if there are more than one session, we use vector of sessions with mutex
    std::vector<std::shared_ptr<Session>> _sessions;
    bool _force_active {false};
    bool _enabled_for_ros {false};
  };

} // namespace detail
} // namespace streaming
} // namespace carla
