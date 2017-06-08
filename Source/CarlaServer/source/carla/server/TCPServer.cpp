// CARLA, Copyright (C) 2017 Computer Vision Center (CVC)

#include "TCPServer.h"

#include <fstream>
#include <iostream>

#include <carla/Logging.h>

namespace carla {
namespace server {

  using boost::asio::ip::tcp;

  static std::string GetBytes(int n) {
    std::string bytes;

    bytes = (n >> 24) & 0xFF;
    bytes += (n >> 16) & 0xFF;
    bytes += (n >> 8) & 0xFF;
    bytes += n & 0xFF;

    return bytes;
  }

  TCPServer::TCPServer(int port) :
    port(port),
    _service(),
    _socket(_service),
    _connected(false) {}

  TCPServer::~TCPServer() {
    close();
  }

  void TCPServer::AcceptSocket() {
    try {
      boost::asio::ip::tcp::acceptor acceptor(_service, tcp::endpoint(tcp::v4(), port));
      acceptor.accept(_socket);
      _service.run();
      log_info("Connected port", port);
      _connected = true;
    } catch (const boost::system::system_error &e) {
      log_error("Socket System error: ", e.what());
    }
  }

  bool TCPServer::Connected() {
    return _connected;
  }

  void TCPServer::writeString(const std::string &message, error_code &error) {
    const int messageSize = static_cast<int>(message.length());
    std::string outMessage(GetBytes(messageSize) + message);
    boost::asio::write(_socket, boost::asio::buffer(outMessage), error);

    if (error) {
      log_info("DESCONECTED port", port);
      _connected = false;
    }
  }

  bool TCPServer::readString(std::string &message, error_code &error) {
    bool end = false, readedBytes = false;
    int readedSize = 0, sizeToRead = -1;

    log_debug("Try to read");
    do {

      std::array<unsigned char, 128> buf;

      size_t len = _socket.read_some(boost::asio::buffer(buf), error);

      if (error) {
        log_info("DESCONECTED port", port);
        _connected = false;
      } else if (!error) {
        // @todo find a better way.
        for (size_t i = 0; i < len && !end; ++i) {
          if (!readedBytes) {
            i = 3;
            readedBytes = true;
          } else {
            std::cout << std::dec;
            message += buf[i];
            ++readedSize;
            if (readedSize >= sizeToRead) { end = true;}
          }

        }
      }

    } while ((!readedBytes || sizeToRead > readedSize) && _connected);

    log_debug("End read");
    return true;
  }

  void TCPServer::close() {
    _connected = false;
    // flush the socket buffer
    std::string message;
    TCPServer::error_code error;
    readString(message, error);
    _service.stop();
    _socket.close();
  }

} // namespace server
} // namespace carla
