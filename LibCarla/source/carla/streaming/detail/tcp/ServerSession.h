// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/NonCopyable.h"
#include "carla/Time.h"
#include "carla/TypeTraits.h"
#include "carla/streaming/detail/Types.h"
#include "carla/streaming/detail/tcp/Message.h"

#include <boost/asio/deadline_timer.hpp>
#include <boost/asio/io_service.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/strand.hpp>

#include <functional>
#include <memory>

namespace carla {
namespace streaming {
namespace detail {
namespace tcp {

  /// A TCP server session. When a session opens, it reads from the socket a
  /// stream id object and passes itself to the callback functor. The session
  /// closes itself after @a timeout of inactivity is met.
  class ServerSession
    : public std::enable_shared_from_this<ServerSession>,
      private NonCopyable {
  public:

    using socket_type = boost::asio::ip::tcp::socket;
    using callback_function_type = std::function<void(std::shared_ptr<ServerSession>)>;

    explicit ServerSession(boost::asio::io_service &io_service, time_duration timeout);

    ~ServerSession();

    /// Starts the session and calls @a callback after successfully reading the
    /// stream id.
    void Open(callback_function_type callback);

    /// @warning This function should only be called after the session is
    /// opened. It is safe to call this function from within the @a callback.
    stream_id_type get_stream_id() const {
      return _stream_id;
    }

    /// Writes some data to the socket.
    template <typename... Buffers>
    void Write(Buffers... buffers) {
      static_assert(
          are_same<Buffer, Buffers...>::value,
          "This function only accepts arguments of type Buffer.");
      Write(std::make_shared<const Message>(std::move(buffers)...));
    }

    /// Posts a job to close this session.
    void Close();

  private:

    void Write(std::shared_ptr<const Message> message);

    void StartTimer();

    friend class Server;

    const size_t _session_id;

    stream_id_type _stream_id = 0u;

    socket_type _socket;

    time_duration _timeout;

    boost::asio::deadline_timer _deadline;

    boost::asio::io_service::strand _strand;

    bool _is_writing = false;
  };

} // namespace tcp
} // namespace detail
} // namespace streaming
} // namespace carla
