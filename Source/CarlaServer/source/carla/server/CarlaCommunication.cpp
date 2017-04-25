#include "CarlaCommunication.h"

#include "carla_protocol.pb.h"
#include "../CarlaServer.h"

#include <iostream>

namespace carla {
namespace server {

  // -- Static methods ---------------------------------------------------------

  template<typename ERROR_CODE>
  static void logTCPError(const std::string &text, const ERROR_CODE &errorCode) {
    std::cerr << "CarlaConnection - TCP Server: " << text << ": " << errorCode.message() << std::endl;
  }

  // This is the thread that sends a string over the TCP socket.
  static void serverWorkerThread(
      TCPServer &server,
      thread::AsyncReaderJobQueue<Reward_Values> &thr,
      const std::unique_ptr<Protocol> &proto,
      const Reward_Values &rwd
      ) {
    if (!thr.getRestart()) {
      std::string message;

      Reward reward;
      proto->LoadReward(reward, rwd);

      bool correctSerialize = reward.SerializeToString(&message);

      TCPServer::error_code error;

      if (correctSerialize) {
        server.writeString(message, error);
        //server.writeString("reward", error);

        if (error) { logTCPError("Failed to send", error); }

        if (!server.Connected()) {
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
  static std::unique_ptr<std::string> clientWorkerThread(TCPServer &server, thread::AsyncWriterJobQueue<std::string> &thr) {
    //if (!server.Connected()) server.AcceptSocket();

    auto message = std::make_unique<std::string>();
    bool success = false;

    do {

      if (!thr.getRestart()) {
        TCPServer::error_code error;

        success = server.readString(*message, error);

        if (error && (error != boost::asio::error::eof)) { // eof is expected.
          logTCPError("Failed to read", error);
          return nullptr;
        }

        if (!server.Connected()) {
          thr.reconnect();
          break;
        }
      }

    } while (!success);

    return message;
  }

  // This is the thread that listens & sends a string over the TCP world socket.
  static std::unique_ptr<std::string> worldReceiveThread(TCPServer &server, thread::AsyncReadWriteJobQueue<std::string, std::string> &thr) {
    auto message = std::make_unique<std::string>();
    bool success = false;

    do {

      if (!thr.getRestart()) {
        TCPServer::error_code error;

        success = server.readString(*message, error);
        if (error && (error != boost::asio::error::eof)) { // eof is expected.
          logTCPError("Failed to read world", error);
          return nullptr;
        }

        if (!server.Connected()) {
          thr.reconnect();
          break;
        }
      }

    } while (!success);

    return message;
  }

  static void worldSendThread(TCPServer &server, thread::AsyncReadWriteJobQueue<std::string, std::string> &thr, const std::string &message) {
    if (!thr.getRestart()) {
      TCPServer::error_code error;

      server.writeString(message, error);

      if (error) {
        logTCPError("Failed to send world", error);
      }

      if (!server.Connected()) {
        thr.reconnect();
      }
    }
  }

  static void Connect(TCPServer &server, CarlaCommunication &communication) {
    std::cout << "Waiting... port: " << server.port << std::endl;
    server.AcceptSocket();

    communication.checkRestart();
  }

  static void ReconnectAll(CarlaCommunication &communication) {
    /// @todo This blocks every CarlaCommunication with a single mutex.
    static std::mutex MUTEX;
    std::lock_guard<std::mutex> lock(MUTEX);

    if (!communication.needsRestart()) {

      std::cout << " ---- RECONNECT ALL ...." << std::endl;

      if (!communication.getWorldThread().getRestart()) {
        std::cout << " ---- RESTART WORLD ...." << std::endl;
        communication.restartWorld();
        communication.getWorldThread().restart();
      }

      if (!communication.getServerThread().getRestart()) {
        std::cout << " ---- RESTART SERVER ...." << std::endl;
        communication.restartServer();
        communication.getServerThread().restart();
      }

      if (!communication.getClientThread().getRestart()) {
        std::cout << " ---- RESTART CLIENT ...." << std::endl;
        communication.restartClient();
        communication.getClientThread().restart();
      }

      communication.restart();
    }
  }

  CarlaCommunication::CarlaCommunication(int worldPort, int writePort, int readPort) :
    _world(worldPort),
    _server(writePort),
    _client(readPort),
    _needsRestart(false),
    _proto(std::make_unique<Protocol>(this)),
    _worldThread{
      [this]() { return worldReceiveThread(this->_world, this->_worldThread); },
      [this](const std::string & msg) { worldSendThread(this->_world, this->_worldThread, msg); },
      [this]() { Connect(this->_world, *this); },
      [this]() { ReconnectAll(*this); }
    },
    _serverThread{
      [this](const Reward_Values &rwd) { serverWorkerThread(this->_server, this->_serverThread, this->_proto, rwd); },
      [this]() { Connect(this->_server, *this); },
      [this]() { ReconnectAll(*this); }
    },
    _clientThread{
      [this]() { return clientWorkerThread(this->_client, this->_clientThread); },
      [this]() { Connect(this->_client, *this); },
      [this]() { ReconnectAll(*this); }
    } {}

  void CarlaCommunication::sendReward(std::unique_ptr<Reward_Values> values) {
    _serverThread.push(std::move(values));
  }

  bool CarlaCommunication::tryReadControl(float &steer, float &gas) {
    steer = 0.0f;
    gas = 0.0f;

    auto message = _clientThread.tryPop();
    if (message == nullptr) { return false; }

    Control control;
    if (!control.ParseFromString(*message)) { return false; }

    steer = control.steer();
    gas = control.gas();

    return true;
  }

  void CarlaCommunication::sendWorld(const uint32_t scenes) {
    //ClearThreads
    _worldThread.clear();
    _clientThread.clear();
    _serverThread.clear();

    World world;
    constexpr int modes = 0; /// @todo #18
    _proto->LoadWorld(world, modes, scenes);

    auto message = std::make_unique<std::string>();
    if (world.SerializeToString(message.get())) {
      _worldThread.push(std::move(message));
    }
  }

  void CarlaCommunication::sendScene(const Scene_Values &values) {

    // Protobuf produces a segmentation fault in the destructor of the Scene
    // when called from Unreal Engine in Linux. As a workaround, we added this
    // cute memory leak.
    /// @todo #10 Fix the memory leak!

    Scene scene;
    _proto->LoadScene(scene, values);

    auto message = std::make_unique<std::string>();

    if (scene.SerializeToString(message.get())) {
      _worldThread.push(std::move(message));
    }

  }

  void CarlaCommunication::sendReset() {
    EpisodeReady eReady;
    eReady.set_ready(true);

    auto message = std::make_unique<std::string>();
    if (eReady.SerializeToString(message.get())) {
      _worldThread.push(std::move(message));
    }
  }

  bool CarlaCommunication::tryReadSceneInit(uint32_t &scene) {
    scene = 0u;

    std::unique_ptr<std::string> info = _worldThread.tryPop();
    if (info == nullptr) { return false; }

    SceneInit sceneInit;

    if (!sceneInit.ParseFromString(*info)) { return false; }

    scene = sceneInit.scene();
    return true;
  }

  bool CarlaCommunication::tryReadEpisodeStart(uint32_t &start_index, uint32_t &end_index) {
    start_index = 0;
    end_index = 0;

    std::unique_ptr<std::string> startData = _worldThread.tryPop();
    if (startData == nullptr) { return false; }

    EpisodeStart episodeStart;
    if (!episodeStart.ParseFromString(*startData)) { return false; }

    start_index = episodeStart.start_index();
    end_index = episodeStart.end_index();

    return true;
  }

  bool CarlaCommunication::tryReadRequestNewEpisode() {
    std::unique_ptr<std::string> request = _worldThread.tryPop();

    if (request == nullptr) { return false; }

    RequestNewEpisode reqEpisode;

    if (!reqEpisode.ParseFromString(*request)) {
      _worldThread.undoPop(std::move(request));
      return false;
    } else { return true; }
  }

  void CarlaCommunication::restartServer() {
    _server.close();
  }

  void CarlaCommunication::restartWorld() {
    _world.close();
  }

  void CarlaCommunication::restartClient() {
    _client.close();
  }

  void CarlaCommunication::checkRestart() {
    if (_needsRestart && _world.Connected() &&
        _client.Connected() && _server.Connected()) {
      _needsRestart = false;
    }
  }

  thread::AsyncReaderJobQueue<Reward_Values> &CarlaCommunication::getServerThread() {
    return _serverThread;
  }

  thread::AsyncWriterJobQueue<std::string> &CarlaCommunication::getClientThread() {
    return _clientThread;
  }

  thread::AsyncReadWriteJobQueue<std::string, std::string> &CarlaCommunication::getWorldThread() {
    return _worldThread;
  }

  bool CarlaCommunication::worldConnected() {
    return _world.Connected();
  }

  bool CarlaCommunication::clientConnected() {
    return _client.Connected();
  }

  bool CarlaCommunication::serverConnected() {
    return _server.Connected();
  }

  bool CarlaCommunication::needsRestart() {
    return _needsRestart;
  }

  void CarlaCommunication::restart() {
    _needsRestart = true;
  }

}
}
