// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/NonCopyable.h"
#include "carla/streaming/detail/Session.h"
#include "carla/streaming/detail/Token.h"

#include <atomic>
#include <memory>

namespace carla {

  class BufferPool;

namespace streaming {
namespace detail {

  /// Handles the synchronization of the shared session.
  class SessionHolder : private NonCopyable {
  public:

    void set_session(std::shared_ptr<Session> session) {
      std::atomic_store_explicit(&_session, session, std::memory_order_relaxed);
    }

  protected:

    std::shared_ptr<Session> get_session() const {
      return std::atomic_load_explicit(&_session, std::memory_order_relaxed);
    }

  private:

    std::shared_ptr<Session> _session;
  };

  /// Shared state among all the copies of a stream. Provides access to the
  /// underlying server session if active.
  class StreamState : public SessionHolder {
  public:

    explicit StreamState(const token_type &token);

    ~StreamState();

    const token_type &token() const {
      return _token;
    }

    Buffer MakeBuffer();

    template <typename... Buffers>
    void Write(Buffers... buffers) {
      auto session = get_session();
      if (session != nullptr) {
        session->Write(std::move(buffers)...);
      }
    }

  private:

    const token_type _token;

    const std::shared_ptr<BufferPool> _buffer_pool;
  };

} // namespace detail
} // namespace streaming
} // namespace carla
