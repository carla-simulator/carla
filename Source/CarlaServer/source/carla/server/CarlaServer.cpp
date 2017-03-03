// CARLA, Copyright (C) 2017 Computer Vision Center (CVC)

#include "Carla.h"
#include "CarlaServer.h"

#include <carla/server/TCPServer.h>
#include <carla/thread/AsyncReaderJobQueue.h>
#include <carla/thread/AsyncWriterJobQueue.h>

#include <iostream>
#include <memory>

namespace carla {
namespace server {

  template <typename ERROR_CODE>
  static void logTCPError(const std::string &text, const ERROR_CODE &errorCode) {
    std::cerr << "CarlaServer - TCP Server: " << text << ": " << errorCode.message() << std::endl;
  }

  // -- Static methods ---------------------------------------------------------

  // This is the thread that sends a string over the TCP socket.
  static void serverWorkerThread(TCPServer &server, const std::string &message) {
    TCPServer::error_code error;
    server.writeString(message, error);
    if (error)
      logTCPError("Failed to send", error);
  }

  // This is the thread that listens for string over the TCP socket.
  static std::string clientWorkerThread(TCPServer &server) {
    std::string message;
    TCPServer::error_code error;
    server.readString(message, error);
    if (error && (error != boost::asio::error::eof)) { // eof is expected.
      logTCPError("Failed to read", error);
      return std::string();
    }
    return message;
  }

  // -- CarlaServer::Pimpl -----------------------------------------------------

  class CarlaServer::Pimpl : private NonCopyable {
  public:

    explicit Pimpl(int writePort, int readPort) :
      _server(writePort),
      _client(readPort),
      _serverThread([this](const std::string &str){ serverWorkerThread(this->_server, str); }),
      _clientThread([this](){ return clientWorkerThread(this->_client); }) {}

    void writeString(const std::string &message) {
      _serverThread.push(message);
    }

    bool tryReadString(std::string &message) {
      return _clientThread.tryPop(message);
    }

  private:

    TCPServer _server;

    TCPServer _client;

    thread::AsyncReaderJobQueue<std::string> _serverThread;

    thread::AsyncWriterJobQueue<std::string> _clientThread;
  };

  // -- CarlaServer ------------------------------------------------------------

  CarlaServer::CarlaServer(int writePort, int readPort) :
    _pimpl(std::make_unique<Pimpl>(writePort, readPort)) {}

  CarlaServer::~CarlaServer() {}

  void CarlaServer::writeString(const std::string &message) {
    _pimpl->writeString(message);
  }

  bool CarlaServer::tryReadString(std::string &message) {
    return _pimpl->tryReadString(message);
  }

} // namespace server
} // namespace carla
