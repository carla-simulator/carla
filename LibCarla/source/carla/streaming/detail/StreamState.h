// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/AtomicSharedPtr.h"
#include "carla/streaming/detail/StreamStateBase.h"

namespace carla {
namespace streaming {
namespace detail {

  /// A stream state that can hold only a single session.
  class StreamState final : public StreamStateBase {
  public:

    using StreamStateBase::StreamStateBase;

    template <typename... Buffers>
    void Write(Buffers &&... buffers) {
      auto session = _session.load();
      if (session != nullptr) {
        session->Write(std::move(buffers)...);
      }
    }

  private:

    void ConnectSession(std::shared_ptr<Session> session) final {
      DEBUG_ASSERT(session != nullptr);
      _session = std::move(session);
    }

    void DisconnectSession(std::shared_ptr<Session> DEBUG_ONLY(session)) final {
      DEBUG_ASSERT(session == _session.load());
      _session = nullptr;
    }

    void ClearSessions() final {
      _session = nullptr;
    }

    AtomicSharedPtr<Session> _session;
  };

} // namespace detail
} // namespace streaming
} // namespace carla
