#include "Carla.h"
#include "CarlaCommunication.h"
#include <iostream>

namespace carla {
namespace server {
  // -- Static methods ---------------------------------------------------------

  template <typename ERROR_CODE>
  static void logTCPError(const std::string &text, const ERROR_CODE &errorCode) {
    std::cerr << "CarlaConnection - TCP Server: " << text << ": " << errorCode.message() << std::endl;
  }


  // This is the thread that sends a string over the TCP socket.
  static void serverWorkerThread(TCPServer &server, const std::string &message) {
    TCPServer::error_code error;
    //message = message.size.c_ + message;

    //if (!server.Connected()) server.AcceptSocket();

      server.writeString(message, error);

    if (error)
      logTCPError("Failed to send", error);

  }


  //TODO:
  // Sortida amb google protocol
  // This is the thread that listens for string over the TCP socket.
  static std::string clientWorkerThread(TCPServer &server) {

    //if (!server.Connected()) server.AcceptSocket();

    std::string message;
    TCPServer::error_code error;

    server.readString(message, error);
    if (error && (error != boost::asio::error::eof)) { // eof is expected.
      logTCPError("Failed to read", error);
      return std::string();
    }

    return message;
  }


  // This is the thread that listens & sends a string over the TCP world socket.
  static std::string worldReceiveThread(TCPServer &server) {
    std::string message;
    TCPServer::error_code error;

    //if (!server.Connected()) server.AcceptSocket();

    server.readString(message, error);
    if (error && (error != boost::asio::error::eof)) { // eof is expected.
      logTCPError("Failed to read world", error);
      return std::string();
    }
    return message;

  }

  static void worldSendThread(TCPServer &server, const std::string &message) {
    TCPServer::error_code error;
    //message = message.size + message;

    //if (!server.Connected()) server.AcceptSocket();

    server.writeString(message, error);
    if (error)
      logTCPError("Failed to send world", error);

  }

  static void Connect(TCPServer &server) {
    if (!server.Connected()) server.AcceptSocket();
  }

  CarlaCommunication::CarlaCommunication(int worldPort, int writePort, int readPort) :
    _world(worldPort),
    _server(writePort),
    _client(readPort),
    _worldThread{
        [this]() { return worldReceiveThread(this->_world); },
        [this](const std::string &msg) { worldSendThread(this->_world, msg); },
        [this]() { Connect(this->_world); }},
    _serverThread{
        [this](const std::string &str) { serverWorkerThread(this->_server, str); },
        [this]() { Connect(this->_server); }},
    _clientThread{
        [this]() { return clientWorkerThread(this->_client); },
        [this]() { Connect(this->_client); }}
    {

    /*std::cout << "WorldPort: " << worldPort << std::endl;
    std::cout << "writePort: " << writePort << std::endl;
    std::cout << "readPort: " << readPort << std::endl;*/

  }

  void CarlaCommunication::sendReward(const Reward &reward) {
    std::string message;
    bool error = !reward.SerializeToString(&message);
    if (!error) {
      _serverThread.push(message);
      std::cout << "Send Reward" << std::endl;
    }
  }

  bool CarlaCommunication::tryReadControl(std::string &control) {
    return _clientThread.tryPop(control);
  }

  void CarlaCommunication::sendWorld(const World &world) {
    std::string message;
    bool error = !world.SerializeToString(&message);
    _worldThread.push(message);
  }

  void CarlaCommunication::sendScene(const Scene &scene) {
    std::string message;
    bool error = !scene.SerializeToString(&message);
    _worldThread.push(message);
  }

  void CarlaCommunication::sendReset(const EpisodeReady &ready) {
    std::string message;
    bool error = !ready.SerializeToString(&message);
    if (!error) {
      //std::cout << "Send End Reset" << std::endl;
      _worldThread.push(message);
    }
    else {
      std::cout << " >> SEND RESET ERROR <<" << std::endl;
    }
  }

  bool CarlaCommunication::tryReadWorldInfo(std::string &info) {
    return _worldThread.tryPop(info);
  }

}
}
