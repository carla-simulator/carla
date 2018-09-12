// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/NonCopyable.h"
#include "carla/streaming/detail/Session.h"
#include "carla/streaming/detail/Token.h"

#include <memory>
#include <mutex>

namespace carla {
namespace streaming {
namespace detail {

  /// Handles the synchronization of the shared session.
  class SessionHolder {
  public:

    void set_session(std::shared_ptr<Session> session) {
      std::lock_guard<std::mutex> guard(_mutex);
      _session = std::move(session);
    }

  protected:

    std::shared_ptr<Session> get_session() const {
      std::lock_guard<std::mutex> guard(_mutex);
      return _session;
    }

  private:

    mutable std::mutex _mutex; /// @todo it can be atomic.

    std::shared_ptr<Session> _session;
  };

  /// Shared state among all the copies of a stream. Provides access to the
  /// underlying server session if active.
  class StreamState
    : public SessionHolder,
      private NonCopyable {
  public:

    explicit StreamState(const token_type &token) : _token(token) {}

    const token_type &token() const {
      return _token;
    }

    template <typename... Buffers>
    void Write(Buffers... buffers) {
      auto session = get_session();
      if (session != nullptr) {
        session->Write(std::move(buffers)...);
      }
    }

  private:

    const token_type _token;
  };

} // namespace detail
} // namespace streaming
} // namespace carla
