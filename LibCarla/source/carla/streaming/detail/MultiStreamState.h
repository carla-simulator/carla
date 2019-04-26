// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/streaming/detail/StreamStateBase.h"

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

    template <typename... Buffers>
    void Write(Buffers &&... buffers) {
      auto message = Session::MakeMessage(std::move(buffers)...);
      std::lock_guard<std::mutex> lock(_mutex);
      for (auto &session : _sessions) {
        if (session != nullptr) {
          session->Write(message);
        }
      }
    }

  private:

    void ConnectSession(std::shared_ptr<Session> session) final {
      DEBUG_ASSERT(session != nullptr);
      std::lock_guard<std::mutex> lock(_mutex);
      _sessions.emplace_back(std::move(session));
    }

    void DisconnectSession(std::shared_ptr<Session> session) final {
      std::lock_guard<std::mutex> lock(_mutex);
      DEBUG_ASSERT(session != nullptr);
      _sessions.erase(
          std::remove(_sessions.begin(), _sessions.end(), session),
          _sessions.end());
    }

    void ClearSessions() final {
      std::lock_guard<std::mutex> lock(_mutex);
      _sessions.clear();
    }

    std::mutex _mutex;

    std::vector<std::shared_ptr<Session>> _sessions;
  };

} // namespace detail
} // namespace streaming
} // namespace carla
