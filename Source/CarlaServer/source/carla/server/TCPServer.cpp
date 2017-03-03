// CARLA, Copyright (C) 2017 Computer Vision Center (CVC)

#include "Carla.h"
#include "TCPServer.h"

namespace carla {
namespace server {

  using boost::asio::ip::tcp;

  TCPServer::TCPServer(int port) :
    _service(),
    _acceptor(_service, tcp::endpoint(tcp::v4(), port)) {}

  TCPServer::~TCPServer() {}

  void TCPServer::writeString(const std::string &message, error_code &error) {
    tcp::socket socket(_service);
    _acceptor.accept(socket);

    boost::asio::write(socket, boost::asio::buffer(message), error);
  }

  void TCPServer::readString(std::string &message, error_code &error) {
    tcp::socket socket(_service);
    _acceptor.accept(socket);

    for (;; ) {
      std::array<char, 128> buf;

      size_t len = socket.read_some(boost::asio::buffer(buf), error);

      if (error == boost::asio::error::eof) {
        break; // Connection closed cleanly by peer.
      } else if (error) {
        return;
      }

      // @todo find a better way.
      for (size_t i = 0u; i < len; ++i) {
        message += buf[i];
      }
    }
  }

} // namespace server
} // namespace carla
