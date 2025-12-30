// Copyright (c) 2025 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include <functional>
#include <memory>

#include "carla/TypeTraits.h"
#include "carla/NonCopyable.h"
#include "carla/streaming/detail/Message.h"
#include "carla/streaming/detail/Types.h"

namespace carla {
namespace streaming {
namespace detail {

  /// A base class of a server session. When a session opens, it reads from the socket a
  /// stream id object and passes itself to the callback functor. The session
  /// closes itself after @a timeout of inactivity is met.
  class Session:
    private NonCopyable  {

  public:
    Session(stream_id_type stream_id = 0) :_stream_id(stream_id) {}
    virtual ~Session() = default;

    template <typename... Buffers>
    static auto MakeMessage(Buffers... buffers) {
      static_assert(
          are_same<SharedBufferView, Buffers...>::value,
          "This function only accepts arguments of type BufferView.");
      return std::make_shared<const Message>(buffers...);
    
    }

    /// @warning This function should only be called after the session is
    /// opened. It is safe to call this function from within the @a callback.
    stream_id_type get_stream_id() const {
      return _stream_id;
    }

    /// Writes message to the socket.
    virtual void WriteMessage(std::shared_ptr<const Message> message) = 0;

    /// Writes some data to the socket.
    template <typename... Buffers>
    void Write(Buffers... buffers) {
      WriteMessage(MakeMessage(buffers...));
    }

    /// Post a job to close the session.
    virtual void Close() = 0;

    virtual void EnableForROS(actor_id_type actor_id) { (void) actor_id; }
    virtual void DisableForROS(actor_id_type actor_id) { (void) actor_id; }
    virtual bool IsEnabledForROS(actor_id_type actor_id) { (void) actor_id; return false; }

  protected:
    stream_id_type _stream_id = 0u;
  };

} // namespace detail
} // namespace streaming
} // namespace carla
