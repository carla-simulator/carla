// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "carla/streaming/detail/tcp/Server.h"

#include <boost/asio/post.hpp>

#include "carla/Logging.h"

#include <memory>

namespace carla {
namespace streaming {
namespace detail {
namespace tcp {

  Server::Server(boost::asio::io_context &io_context, endpoint ep)
    : _io_context(io_context),
      _acceptor(_io_context, std::move(ep)),
      _timeout(time_duration::seconds(10u)),
      _synchronous(false) {}

  void Server::OpenSession(
      time_duration timeout,
      ServerSession::callback_function_type on_opened,
      ServerSession::callback_function_type on_closed) {
    using boost::system::error_code;

    auto session = std::make_shared<ServerSession>(_io_context, timeout, *this);

    auto handle_query = [on_opened, on_closed, session](const error_code &ec) {
      if (!ec) {
        session->Open(std::move(on_opened), std::move(on_closed));
      } else {
        log_error("tcp accept error:", ec.message());
      }
    };

    _acceptor.async_accept(session->_socket, [=](error_code ec) {
      // Handle query and open a new session immediately.
      boost::asio::post(_io_context, [=]() { handle_query(ec); });
      OpenSession(timeout, on_opened, on_closed);
    });
  }

} // namespace tcp
} // namespace detail
} // namespace streaming
} // namespace carla
