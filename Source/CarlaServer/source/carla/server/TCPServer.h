// CARLA, Copyright (C) 2017 Computer Vision Center (CVC)

#pragma once

#include <string>

#include <boost/asio.hpp>

namespace carla {
namespace server {

  /// Synchronous TCP server.
  ///
  /// A new socket is created for every connection (every write and read).
  class CARLA_API TCPServer : private NonCopyable {
  public:

    using error_code = boost::system::error_code;

    explicit TCPServer(int port);

    ~TCPServer();

    void writeString(const std::string &message, error_code &error);

    void readString(std::string &message, error_code &error);

  private:

    boost::asio::io_service _service;

    boost::asio::ip::tcp::acceptor _acceptor;
  };

} // namespace server
} // namespace carla
