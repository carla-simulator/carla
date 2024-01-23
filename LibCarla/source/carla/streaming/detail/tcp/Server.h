// Copyright (c) 2024 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/NonCopyable.h"
#include "carla/Time.h"
#include "carla/streaming/detail/tcp/ServerSession.h"

#include <boost/asio/io_context.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/post.hpp>

#include <atomic>

namespace carla {
namespace streaming {
namespace detail {
namespace tcp {

  /// @warning This server cannot be destructed before its @a io_context is
  /// stopped.
  class Server : private NonCopyable {
  public:

    using endpoint = boost::asio::ip::tcp::endpoint;
    using protocol_type = endpoint::protocol_type;

    explicit Server(boost::asio::io_context &io_context, endpoint ep);

    endpoint GetLocalEndpoint() const {
      return _acceptor.local_endpoint();
    }

    /// Set session time-out. Applies only to newly created sessions. By default
    /// the time-out is set to 10 seconds.
    void SetTimeout(time_duration timeout) {
      _timeout = timeout;
    }

    /// Start listening for connections. On each new connection, @a
    /// on_session_opened is called, and @a on_session_closed when the session
    /// is closed.
    template <typename FunctorT1, typename FunctorT2>
    void Listen(FunctorT1 on_session_opened, FunctorT2 on_session_closed) {
      boost::asio::post(_io_context, [=, this]() {
        OpenSession(
            _timeout,
            std::move(on_session_opened),
            std::move(on_session_closed));
      });
    }

    void SetSynchronousMode(bool is_synchro) {
      _synchronous = is_synchro;
    }

    bool IsSynchronousMode() const {
      return _synchronous;
    }

  private:

    void OpenSession(
        time_duration timeout,
        ServerSession::callback_function_type on_session_opened,
        ServerSession::callback_function_type on_session_closed);

    boost::asio::io_context &_io_context;

    boost::asio::ip::tcp::acceptor _acceptor;

    std::atomic<time_duration> _timeout;

    bool _synchronous;
  };

} // namespace tcp
} // namespace detail
} // namespace streaming
} // namespace carla
