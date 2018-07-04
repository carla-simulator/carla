// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/streaming/low_level/tcp/ServerSession.h"

#include <boost/asio/io_service.hpp>
#include <boost/asio/ip/tcp.hpp>

#include <atomic>
#include <memory>
#include <string>

namespace carla {
namespace streaming {
namespace low_level {
namespace tcp {

  class Server : private boost::noncopyable {
  public:

    using endpoint = boost::asio::ip::tcp::endpoint;
    using protocol_type = endpoint::protocol_type;
    using duration_type = ServerSession::duration_type;

    explicit Server(boost::asio::io_service &io_service, endpoint ep)
      : _acceptor(io_service, std::move(ep)),
        _timeout(duration_type::seconds(10u)) {}

    /// Set session time-out. Applies only to newly created sessions.
    void set_timeout(duration_type timeout) {
      _timeout = timeout;
    }

    template <typename Functor>
    void Listen(Functor callback) {
      log_info("starting streaming server at port", _acceptor.local_endpoint().port());
      _acceptor.get_io_service().post([=]() { OpenSession(_timeout, callback); });
    }

  private:

    template <typename Functor>
    void OpenSession(duration_type timeout, Functor callback) {
      using boost::system::error_code;

      auto session = std::make_shared<ServerSession>(_acceptor.get_io_service(), timeout);

      auto handle_query = [=](const error_code &ec) {
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

    boost::asio::ip::tcp::acceptor _acceptor;

    std::atomic<timeout_type> _timeout;
  };

} // namespace tcp
} // namespace low_level
} // namespace streaming
} // namespace carla
