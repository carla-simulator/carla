#include "CarlaCommunication.h"
#include "lodepng.h"

#include "carla_protocol.pb.h"
#include "../CarlaServer.h"

#include <iostream>

namespace carla {
namespace server {

  // -- Static methods ---------------------------------------------------------

  std::mutex _generalMutex;

  static Mode getMode(int modeInt) {
    switch (modeInt) {
      case 0:   return Mode::MONO;
      case 1:   return Mode::STEREO;
      default:  return Mode::INVALID;
    }
  }


  template<typename ERROR_CODE>
  static void logTCPError(const std::string &text, const ERROR_CODE &errorCode) {
    std::cerr << "CarlaConnection - TCP Server: " << text << ": " << errorCode.message() << std::endl;
  }

  // This is the thread that sends a string over the TCP socket.
  static void serverWorkerThread(
    TCPServer &server, 
    thread::AsyncReaderJobQueue<Reward_Values> &thr,
    const std::unique_ptr<Protocol> &proto ,
    const Reward_Values &rwd
    ) {

    if (!thr.getRestart()){
      std::string message;

      Reward reward;
      proto->LoadReward(reward, rwd);

      bool correctSerialize = reward.SerializeToString(&message);

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
    _proto(std::make_unique<Protocol>(this)),
    _worldThread {
    [this]() { return worldReceiveThread(this->_world, this->_worldThread); },
    [this](const std::string & msg) { worldSendThread(this->_world, this->_worldThread, msg); },
    [this]() { Connect(this->_world); },
    [this]() { ReconnectAll(*this);}
  },
  _serverThread {
    [this](const Reward_Values &rwd) { serverWorkerThread(this->_server, this->_serverThread, this->_proto, rwd); },
    [this]() { Connect(this->_server); },
    [this]() { ReconnectAll(*this);}
  },
  _clientThread {
    [this]() { return clientWorkerThread(this->_client, this->_clientThread); },
    [this]() { Connect(this->_client); },
    [this]() { ReconnectAll(*this);}
  }
  {
    _mode = Mode::MONO;
    /*std::cout << "WorldPort: " << worldPort << std::endl;
    std::cout << "writePort: " << writePort << std::endl;
    std::cout << "readPort: " << readPort << std::endl;*/

  }

  void CarlaCommunication::sendReward(const Reward_Values &values) {
    _serverThread.push(values);
  }

  bool CarlaCommunication::tryReadControl(float &steer, float &gas) {

    steer = 0.0f;
    gas = 0.0f;
    std::string controlMessage;

    if (!_clientThread.tryPop(controlMessage)) return false;

    Control control;
    if (!control.ParseFromString(controlMessage)) return false;

    steer = control.steer();
    gas = control.gas();

    return true;

  }

  void CarlaCommunication::sendWorld(const uint32_t modes,const uint32_t scenes) {

    _needRestart = false;

    World world;
    _proto->LoadWorld(world, modes, scenes);

    std::string message;
    bool error = !world.SerializeToString(&message);

    _worldThread.push(message);
  }

  void CarlaCommunication::sendScene(const Scene_Values &values) {
    Scene scene;
    _proto -> LoadScene(scene, values);

    std::string message;
    bool error = !scene.SerializeToString(&message);
    _worldThread.push(message);
  }

  void CarlaCommunication::sendReset() {

    EpisodeReady eReady;
    eReady.set_ready(true);

    std::string message;
    if (eReady.SerializeToString(&message)) {
      _worldThread.push(message);
    }
  }

  bool CarlaCommunication::tryReadSceneInit(Mode &mode, uint32_t &scene) {

    mode = Mode::INVALID;
    scene = 0u;

    std::string info;
    if (!_worldThread.tryPop(info)) return false;

    SceneInit sceneInit;

    if (!sceneInit.ParseFromString(info)) return false;

    mode = getMode(sceneInit.mode());
    scene = sceneInit.scene();

    _mode = mode;

    return true;
  }

  bool CarlaCommunication::tryReadEpisodeStart(uint32_t &start_index, uint32_t &end_index){
    start_index = 0;
    end_index = 0;

    std::string startData;
    if (!_worldThread.tryPop(startData)) return false;

    EpisodeStart episodeStart;
    if(!episodeStart.ParseFromString(startData)) return false;

    start_index = episodeStart.start_index();
    end_index = episodeStart.end_index();

    return true;
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

  thread::AsyncReaderJobQueue<Reward_Values>& CarlaCommunication::getServerThread(){
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

  Mode CarlaCommunication::GetMode(){
    return _mode;
  }

  bool CarlaCommunication::NeedRestart(){
    return _needRestart;
  }

  void CarlaCommunication::Restart(){
    _needRestart = true;
  }
}
}
