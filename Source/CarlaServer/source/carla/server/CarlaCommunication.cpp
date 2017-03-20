#include "CarlaCommunication.h"
#include "lodepng.h"

#include "carla_protocol.pb.h"

#include <iostream>

namespace carla {
namespace server {

  // -- Static methods ---------------------------------------------------------

  std::mutex _generalMutex;


  template<typename ERROR_CODE>
  static void logTCPError(const std::string &text, const ERROR_CODE &errorCode) {
    std::cerr << "CarlaConnection - TCP Server: " << text << ": " << errorCode.message() << std::endl;
  }

  // This is the thread that sends a string over the TCP socket.
  static void serverWorkerThread(TCPServer &server, thread::AsyncReaderJobQueue<Reward> &thr, const Reward &rwd) {

    if (!thr.getRestart()){
      std::string message;
      bool correctSerialize = rwd.SerializeToString(&message);

      TCPServer::error_code error;

      if (correctSerialize) {
        server.writeString(message, error);
        if (error) {
          logTCPError("Failed to send", error); 
        }

        if (!server.Connected() && !thr.getRestart()) {
          thr.reconnect();
        }

      } else {
        logTCPError("Falied to serialize", error);
      }

     
    }
  }

  //TODO:
  // Sortida amb google protocol
  // This is the thread that listens for string over the TCP socket.
  static std::string clientWorkerThread(TCPServer &server, thread::AsyncWriterJobQueue<std::string> &thr) {

    //if (!server.Connected()) server.AcceptSocket();

    std::string message;
    bool success = false;

    do{

      if (!thr.getRestart()){
        TCPServer::error_code error;

        success = server.readString(message, error);

        if (error && (error != boost::asio::error::eof)) { // eof is expected.
          logTCPError("Failed to read", error);
          return std::string();
        }

        if (!server.Connected() && !thr.getRestart()) { 
          thr.reconnect();
          break;
        }
      }

    } while (!success);

    return message;
  }

  // This is the thread that listens & sends a string over the TCP world socket.
  static std::string worldReceiveThread(TCPServer &server, thread::AsyncReadWriteJobQueue<std::string, std::string> &thr) {
    //std::lock_guard<std::mutex> lock(server.getMutex());
    std::string message;
    bool success = false;

    do{

      if (!thr.getRestart()){
        TCPServer::error_code error;

        success = server.readString(message, error);
        if (error && (error != boost::asio::error::eof)) { // eof is expected.
          logTCPError("Failed to read world", error);
          return std::string();
        }
       
        if (!server.Connected() && !thr.getRestart()) {
          thr.reconnect();
          break;
        }
      }
      
    }while (!success);

    return message;

  }

  static void worldSendThread(TCPServer &server, thread::AsyncReadWriteJobQueue<std::string, std::string> &thr, const std::string &message) {

    if (!thr.getRestart()){
      TCPServer::error_code error;

      server.writeString(message, error);
      if (error) {
        logTCPError("Failed to send world", error);
      }

      if (!server.Connected() && !thr.getRestart()) {
        thr.reconnect();
      }
    }
  }

  static void Connect(TCPServer &server) {
      std::cout << "Waiting... port: " << server.port << std::endl;
      server.AcceptSocket(); 
  }

  static void ReconnectAll(CarlaCommunication &communication){

    std::lock_guard<std::mutex> lock(_generalMutex);



    if (!communication.NeedRestart()){

      std::cout << " ---- RECONNECT ALL ...." << std::endl;

      if (!communication.getWorldThread().getRestart()){
        std::cout << " ---- RESTART WORLD ...." << std::endl;
        communication.restartWorld();
        communication.getWorldThread().restart();
      }

      if (!communication.getServerThread().getRestart()){
        std::cout << " ---- RESTART SERVER ...." << std::endl;
        communication.restartServer();
        communication.getServerThread().restart();
      }

      if (!communication.getClientThread().getRestart()){
        std::cout << " ---- RESTART CLIENT ...." << std::endl;
        communication.restartClient();
        communication.getClientThread().restart();
      }

      communication.Restart();

    }
  }

  CarlaCommunication::CarlaCommunication(int worldPort, int writePort, int readPort) :
    _serverPort(writePort),
    _clientPort(readPort),
    _worldPort(worldPort),
    _world(worldPort),
    _server(writePort),
    _client(readPort),
    _needRestart(false),
    _worldThread {
    [this]() { return worldReceiveThread(this->_world, this->_worldThread); },
    [this](const std::string & msg) { worldSendThread(this->_world, this->_worldThread, msg); },
    [this]() { Connect(this->_world); },
    [this]() { ReconnectAll(*this);}
  },
  _serverThread {
    [this](const Reward &rwd) { serverWorkerThread(this->_server, this->_serverThread, rwd); },
    [this]() { Connect(this->_server); },
    [this]() { ReconnectAll(*this);}
  },
  _clientThread {
    [this]() { return clientWorkerThread(this->_client, this->_clientThread); },
    [this]() { Connect(this->_client); },
    [this]() { ReconnectAll(*this);}
  }
  {

    /*std::cout << "WorldPort: " << worldPort << std::endl;
    std::cout << "writePort: " << writePort << std::endl;
    std::cout << "readPort: " << readPort << std::endl;*/

  }

  void CarlaCommunication::sendReward(const Reward &reward) {
    _serverThread.push(reward);
    std::cout << "Send Reward" << std::endl;
  }

  bool CarlaCommunication::tryReadControl(std::string &control) {
    return _clientThread.tryPop(control);
  }

  void CarlaCommunication::sendWorld(const World &world) {

    _needRestart = false;

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
  }

  bool CarlaCommunication::tryReadWorldInfo(std::string &info) {
    return _worldThread.tryPop(info);
  }

  void CarlaCommunication::restartServer(){
    _server.close();
    //_server =  TCPServer(_serverPort);
  }

  void CarlaCommunication::restartWorld(){
    _world.close();
    //_world = TCPServer(_worldPort);
  }

  void CarlaCommunication::restartClient(){
    _client.close();
    //_client = TCPServer(_clientPort); 
  }

  thread::AsyncReaderJobQueue<Reward>& CarlaCommunication::getServerThread(){
    return _serverThread;
  }

  thread::AsyncWriterJobQueue<std::string>& CarlaCommunication::getClientThread(){
    return _clientThread;
  }
  
  thread::AsyncReadWriteJobQueue<std::string, std::string>& CarlaCommunication::getWorldThread(){
    return _worldThread;
  }

  bool CarlaCommunication::worldConnected(){
    return _world.Connected();
  }

  bool CarlaCommunication::clientConnected(){
    return _client.Connected();
  }
    
  bool CarlaCommunication::serverConnected(){
    return _server.Connected();
  }

  bool CarlaCommunication::NeedRestart(){
    return _needRestart;
  }

  void CarlaCommunication::Restart(){
    _needRestart = true;
  }
}
}
