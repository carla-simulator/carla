// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "carla/streaming/detail/tcp/Server.h"

#include "carla/Logging.h"

#include <memory>

namespace carla {
namespace streaming {
namespace detail {
namespace tcp {

  Server::Server(boost::asio::io_service &io_service, endpoint ep)
    : _acceptor(io_service, std::move(ep)),
      _timeout(time_duration::seconds(10u)) {}

  void Server::OpenSession(
      const time_duration timeout,
      ServerSession::callback_function_type callback) {
    using boost::system::error_code;

    auto session = std::make_shared<ServerSession>(_acceptor.get_io_service(), timeout);

    auto handle_query = [callback, session](const error_code &ec) {
      if (!ec) {
        session->Open(callback);
      } else {
        log_error("tcp accept error:", ec.message());
      }
    };

    _acceptor.async_accept(session->_socket, [=](error_code ec) {
      // Handle query and open a new session immediately.
      _acceptor.get_io_service().post([=]() { handle_query(ec); });
      OpenSession(timeout, callback);
    });
  }

} // namespace tcp
} // namespace detail
} // namespace streaming
} // namespace carla
