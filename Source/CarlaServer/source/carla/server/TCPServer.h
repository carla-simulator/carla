// CARLA, Copyright (C) 2017 Computer Vision Center (CVC)

#pragma once

#include <carla/NonCopyable.h>

#include <string>
#include <mutex>
#include <boost/asio.hpp>

namespace carla {
namespace server {

  /// { TCP server.
  ///
  /// A new socket is created for every connection (every write and read).
  class TCPServer : private NonCopyable {
  public:

    using error_code = boost::system::error_code;

    explicit TCPServer(int port);

    ~TCPServer();

    void writeString(const std::string &message, error_code &error);

    void readString(std::string &message, error_code &error);

	void AcceptSocket();

	bool Connected();

	const int _port;

  private:

    boost::asio::io_service _service;

    boost::asio::ip::tcp::acceptor _acceptor;

	boost::asio::ip::tcp::socket _socket;

	bool _connected;
  };

} // namespace server
} // namespace carla
