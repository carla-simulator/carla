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

namespace carla {

  class BufferPool;

namespace streaming {
namespace detail {

  /// Shared state among all the copies of a stream. Provides access to the
  /// underlying server session(s) if active.
  class StreamStateBase : private NonCopyable {
  public:

    explicit StreamStateBase(const token_type &token);

    virtual ~StreamStateBase();

    const token_type &token() const {
      return _token;
    }

    Buffer MakeBuffer();

    virtual void ConnectSession(std::shared_ptr<Session> session) = 0;

    virtual void DisconnectSession(std::shared_ptr<Session> session) = 0;

    virtual void ClearSessions() = 0;

  private:

    const token_type _token;

    const std::shared_ptr<BufferPool> _buffer_pool;
  };

} // namespace detail
} // namespace streaming
} // namespace carla
