// Copyright (c) 2025 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/Time.h"
#include "carla/profiler/LifetimeProfiled.h"
#include "carla/streaming/detail/Session.h"

#if defined(__clang__)
#  pragma clang diagnostic push
#  pragma clang diagnostic ignored "-Wshadow"
#endif
#include <boost/asio/deadline_timer.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/strand.hpp>
#if defined(__clang__)
#  pragma clang diagnostic pop
#endif

namespace carla {
namespace streaming {
namespace detail {
namespace tcp {

  class Server;

  /// A TCP server session. When a session opens, it reads from the socket a
  /// stream id object and passes itself to the callback functor. The session
  /// closes itself after @a timeout of inactivity is met.
  class ServerSession
    : public std::enable_shared_from_this<ServerSession>,
      public Session,
      private profiler::LifetimeProfiled {
  public:

    using socket_type = boost::asio::ip::tcp::socket;
    using callback_function_type = std::function<void(std::shared_ptr<ServerSession>)>;

    explicit ServerSession(
        boost::asio::io_context &io_context,
        time_duration timeout,
        Server &server);

    /// Starts the session and calls @a on_opened after successfully reading the
    /// stream id, and @a on_closed once the session is closed.
    void Open(
        callback_function_type on_opened,
        callback_function_type on_closed);

    /// Writes message to the socket.
    virtual void WriteMessage(std::shared_ptr<const Message> message) override;

    /// Post a job to close the session.
    virtual void Close() override;

  private:

    void StartTimer();

    void CloseNow(boost::system::error_code ec = boost::system::error_code());

    friend class Server;

    Server &_server;

    const size_t _session_id;

    socket_type _socket;

    time_duration _timeout;

    boost::asio::deadline_timer _deadline;

    boost::asio::io_context::strand _strand;

    callback_function_type _on_closed;

    bool _is_writing = false;
  };

} // namespace tcp
} // namespace detail
} // namespace streaming
} // namespace carla
