// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.
//
// This is a modification of boost example, blocking_tcp_client.cpp
//
// Copyright (c) 2003-2012 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include "carla/server/TCPServer.h"

#include <boost/asio/read.hpp>
#include <boost/asio/write.hpp>
#include <boost/lambda/bind.hpp>
#include <boost/lambda/lambda.hpp>
#include <boost/system/error_code.hpp>
#include <boost/system/system_error.hpp>

#include "carla/Logging.h"

using boost::lambda::_1;
using boost::lambda::var;
using namespace boost::asio::ip;

namespace carla {
namespace server {

  // ===========================================================================
  // -- Static local methods ---------------------------------------------------
  // ===========================================================================

  static inline int GetPort(const tcp::socket &socket) {
    return (socket.is_open() ? socket.local_endpoint().port() : 0);
  }

#define LOG_PREFIX "tcpserver", GetPort(_socket), ':'

  static void CloseConnection(tcp::acceptor &_acceptor, tcp::socket &_socket) {
    log_info(LOG_PREFIX, "disconnecting");
    if (_acceptor.is_open()) {
      _acceptor.close();
    }
    if (_socket.is_open()) {
      _socket.close();
    }
  }

  // ===========================================================================
  // -- TCPServer --------------------------------------------------------------
  // ===========================================================================

  TCPServer::TCPServer()
      : _service(),
        _acceptor(_service),
        _socket(_service),
        _deadline(_service) {
    // No deadline is required until the first socket operation is started. We
    // set the deadline to positive infinity so that the actor takes no action
    // until a specific deadline is set.
    _deadline.expires_at(boost::posix_time::pos_infin);
    // Start the persistent actor that checks for deadline expiry.
    CheckDeadline();
  }

  TCPServer::~TCPServer() {
    CloseConnection(_acceptor, _socket);
  }

  void TCPServer::Disconnect() {
    log_debug(LOG_PREFIX, "request close connection");
    _service.post([this](){ CloseConnection(_acceptor, _socket); });
  }

  error_code TCPServer::Connect(uint32_t port, time_duration timeout) {
    // Set the deadline, it will close the socket when expired.
    _deadline.expires_from_now(timeout);

    if (_acceptor.is_open()) {
      log_error(LOG_PREFIX, "already connected");
      return boost::asio::error::already_connected;
    }

    // Create an acceptor at the given port.
    try {
      _acceptor = tcp::acceptor(_service, tcp::endpoint(tcp::v4(), port));
    } catch (const boost::system::system_error &exception) {
      log_error(LOG_PREFIX, "unable to accept connection:", exception.what());
      return exception.code();
    }

    // Asio guarantees that its asynchronous operations will never fail with
    // would_block, so any other value in ec indicates completion.
    error_code ec = boost::asio::error::would_block;

    // Start the asynchronous operation.
    _acceptor.async_accept(_socket, var(ec) = _1);

    // Block until the asynchronous operation has completed.
    do {
      _service.run_one();
    } while (ec == boost::asio::error::would_block);

    // Determine whether a connection was successfully established.
    if (ec) {
      log_error(LOG_PREFIX, "connection failed:", ec.message());
      Disconnect(); // Will disconnect on the next run.
    } else {
      log_info(LOG_PREFIX, "connected");
    }
    return ec;
  }

  error_code TCPServer::Read(mutable_buffer buffer, time_duration timeout) {
    log_debug(LOG_PREFIX, "receiving to buffer of length", boost::asio::buffer_size(buffer));
    _deadline.expires_from_now(timeout);

    error_code ec = boost::asio::error::would_block;
    boost::asio::async_read(_socket, boost::asio::buffer(buffer), var(ec) = _1);

    do {
      _service.run_one();
    } while (ec == boost::asio::error::would_block);

    if (ec) {
      log_error(LOG_PREFIX, "error reading message:", ec.message());
    }
    return ec;
  }

  error_code TCPServer::Write(const_buffer buffer, time_duration timeout) {
    log_debug(LOG_PREFIX, "sending from buffer of length", boost::asio::buffer_size(buffer));
    _deadline.expires_from_now(timeout);

    error_code ec = boost::asio::error::would_block;
    boost::asio::async_write(_socket, boost::asio::buffer(buffer), var(ec) = _1);

    do {
      _service.run_one();
    } while (ec == boost::asio::error::would_block);

    if (ec) {
      log_error(LOG_PREFIX, "error writing message:", ec.message());
    }
    return ec;
  }

  void TCPServer::CheckDeadline() {
    if (_deadline.expires_at() <= boost::asio::deadline_timer::traits_type::now()) {
      log_info(LOG_PREFIX, "timed out");
      CloseConnection(_acceptor, _socket);
      _deadline.expires_at(boost::posix_time::pos_infin);
    }
    _deadline.async_wait(boost::lambda::bind(&TCPServer::CheckDeadline, this));
  }

#undef LOG_PREFIX

} // namespace server
} // namespace carla
