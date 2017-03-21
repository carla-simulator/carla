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
  class TCPServer {
  public:

    using error_code = boost::system::error_code;

    explicit TCPServer(int port);

    ~TCPServer();

    void writeString(const std::string &message, error_code &error);

    bool readString(std::string &message, error_code &error);

	void AcceptSocket();

	bool Connected();

  void close();

	const int port;

  private:
  
   boost::asio::io_service _service;


	 boost::asio::ip::tcp::socket _socket;

   std::atomic_bool _connected;
  };

} // namespace server
} // namespace carla
