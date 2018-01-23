// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include <boost/asio/deadline_timer.hpp>
#include <boost/asio/io_service.hpp>
#include <boost/asio/ip/tcp.hpp>

#include "carla/NonCopyable.h"
#include "carla/server/ServerTraits.h"

namespace carla {
namespace server {

  /// Basic blocking TCP server with time-out. It is safe to call disconnect
  /// in a separate thread.
  class TCPServer : private NonCopyable {
  public:

    TCPServer();

    ~TCPServer();

    /// Posts a job to disconnect the server.
    void Disconnect();

    error_code Connect(uint32_t port, time_duration timeout);

    error_code Read(mutable_buffer buffer, time_duration timeout);

    error_code Write(const_buffer buffer, time_duration timeout);

  private:

    void CheckDeadline();

    boost::asio::io_service _service;

    boost::asio::ip::tcp::acceptor _acceptor;

    boost::asio::ip::tcp::socket _socket;

    boost::asio::deadline_timer _deadline;
  };

} // namespace server
} // namespace carla
