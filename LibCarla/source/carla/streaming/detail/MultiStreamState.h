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
      _session(nullptr),
      _writeFunction(&MultiStreamState::WriteSingle)
      {};

    template <typename... Buffers>
    void Write(Buffers &&... buffers) {
      auto message = Session::MakeMessage(std::move(buffers)...);
      (this->*_writeFunction)(std::move(message));
    }

  private:

    void ConnectSession(std::shared_ptr<Session> session) final {
      DEBUG_ASSERT(session != nullptr);
      std::lock_guard<std::mutex> lock(_mutex);
      _sessions.emplace_back(std::move(session));
      log_debug("Connecting multistream sessions:", _sessions.size());
      if (_sessions.size() == 1) {
        _session.store(_sessions[0]);
        _writeFunction = &MultiStreamState::WriteSingle;
      }
      else if (_sessions.size() > 1) {
        _writeFunction = &MultiStreamState::WriteMulti;
      }
      (_writeFunction == &MultiStreamState::WriteSingle) ? log_debug("Pointer to single stream") : log_debug("Pointer to multi stream");
    }

    void DisconnectSession(std::shared_ptr<Session> session) final {
      DEBUG_ASSERT(session != nullptr);
      std::lock_guard<std::mutex> lock(_mutex);
      if (_sessions.size() == 0) return;
      if (_sessions.size() == 1) {
        DEBUG_ASSERT(session == _session.load());
        _session.store(nullptr);
        _sessions.clear();
        _writeFunction = &MultiStreamState::WriteSingle;
      } else {
        _sessions.erase(
            std::remove(_sessions.begin(), _sessions.end(), session),
            _sessions.end());
            _session.store(_sessions[0]);
        
        // choose multi or single stream write function
        if (_sessions.size() > 1)
          _writeFunction = &MultiStreamState::WriteMulti;
        else
          _writeFunction = &MultiStreamState::WriteSingle;
      }
      log_debug("Disconnecting multistream sessions:", _sessions.size());
      (_writeFunction == &MultiStreamState::WriteSingle) ? log_debug("Pointer to single stream") : log_debug("Pointer to multi stream");
    }

    void ClearSessions() final {
      std::lock_guard<std::mutex> lock(_mutex);
      _sessions.clear();
      _session.store(nullptr);
      _writeFunction = &MultiStreamState::WriteSingle;
      log_debug("Disconnecting all multistream sessions");
      (_writeFunction == &MultiStreamState::WriteSingle) ? log_debug("Pointer to single stream") : log_debug("Pointer to multi stream");
    }

    void WriteSingle(std::shared_ptr<const tcp::Message> message) {
      auto session = _session.load();
      if (session != nullptr) {
        log_debug("Multistream single write to stream id", session->get_stream_id());
        session->Write(std::move(message));
      }
    }

    void WriteMulti(std::shared_ptr<const tcp::Message> message) {
      std::lock_guard<std::mutex> lock(_mutex);
      for (auto &session : _sessions) {
        if (session != nullptr) {
          log_debug("Multistream multi write to stream id", session->get_stream_id());
          session->Write(message);
        }
      }
    }

    std::mutex _mutex;

    // if there is only one session, then we use atomic
    AtomicSharedPtr<Session> _session;
    // if there are more than one session, we use vector of sessions with mutex
    std::vector<std::shared_ptr<Session>> _sessions;
    // write function pointer (by default write single stream)
    void (MultiStreamState::*_writeFunction)(std::shared_ptr<const tcp::Message>);
  };

} // namespace detail
} // namespace streaming
} // namespace carla
