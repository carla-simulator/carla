#include "CarlaCommunication.h"

#include "carla_protocol.pb.h"
#include "../CarlaServer.h"

#include <carla/Logging.h>

#include <iostream>

namespace carla {
namespace server {

  // -- Static methods ---------------------------------------------------------

  template<typename ERROR_CODE>
  static void logTCPError(const std::string &text, const ERROR_CODE &errorCode) {
    log_error("CarlaConnection - TCP Server:", text, ':', errorCode.message());
  }


  // This is the thread that sends a string over the TCP socket.
  static void serverWorkerThread(
      TCPServer &server,
      thread::AsyncReaderJobQueue<Reward_Values> &thr,
      const std::unique_ptr<Protocol> &proto,
      const Reward_Values &rwd
      ) {

    {
      using namespace std;



      if (!thr.getRestart()) {
      std::string message;


      clock_t start = clock();

      Reward reward;
      proto->LoadReward(reward, rwd);


      clock_t end = clock();

      double elapsed_secs = double(end- start) / CLOCKS_PER_SEC;

      log_info("Send time:", elapsed_secs, "(s)");

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

  }

  // This is the thread that listens for string over the TCP socket.
  static std::unique_ptr<std::string> clientWorkerThread(TCPServer &server, thread::AsyncWriterJobQueue<std::string> &thr) {
    auto message = std::make_unique<std::string>();
    bool success = false;

      if (!thr.getRestart()) {
        TCPServer::error_code error;

        success = server.readString(*message, error);

        if (error && (error != boost::asio::error::eof)) { // eof is expected.
          logTCPError("Failed to read", error);
          return nullptr;
        }

        if (!server.Connected()) {
          thr.reconnect();
          return nullptr;
        }
      }

    if (!success){
      return nullptr;
    }
    else{
      return message;
    }
  }

  // This is the thread that listens  a string over the TCP world socket.
  static std::unique_ptr<std::string> worldReceiveThread(TCPServer &server, thread::AsyncReadWriteJobQueue<std::string, std::string> &thr) {
    auto message = std::make_unique<std::string>();
    bool success = false;

      if (!thr.getRestart()) {
        TCPServer::error_code error;

        success = server.readString(*message, error);
        if (error && (error != boost::asio::error::eof)) { // eof is expected.
          logTCPError("Failed to read world", error);
          return nullptr;
        }

        if (!server.Connected()) {
          thr.reconnect();
          return nullptr;
        }
      }

    if (!success) {
      return nullptr;
    }
    else {
      return message;
    }
  }

  // This is the thread that sends a string over the TCP world socket.
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
    log_info("Waiting... port:", server.port);
    server.AcceptSocket();

    communication.checkRestart();
  }

  static void ReconnectAll(CarlaCommunication &communication) {
    /// @todo This blocks every CarlaCommunication with a single mutex.
    static std::mutex MUTEX;
    std::lock_guard<std::mutex> lock(MUTEX);

    if (!communication.needsRestart()) {

      log_debug("---- RECONNECT ALL ....");

      if (!communication.getWorldThread().getRestart()) {
        log_debug("---- RESTART WORLD ....");
        communication.restartWorld();
        communication.getWorldThread().restart();
      }

      if (!communication.getServerThread().getRestart()) {
        log_debug("---- RESTART SERVER ....");
        communication.restartServer();
        communication.getServerThread().restart();
      }

      if (!communication.getClientThread().getRestart()) {
        log_debug("---- RESTART CLIENT ....");
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


  CarlaCommunication::~CarlaCommunication(){
    _worldThread.done = true;
    _serverThread.done = true;
    _clientThread.done = true;
  }

  void CarlaCommunication::sendReward(std::unique_ptr<Reward_Values> values) {
    _serverThread.push(std::move(values));
  }

  bool CarlaCommunication::tryReadControl(Control_Values &control_values) {
    control_values.steer = 0.0f;
    control_values.gas = 0.0f;
    control_values.brake = 0.0f;
    control_values.hand_brake = false;
    control_values.reverse = false;

    auto message = _clientThread.tryPop();
    if (message == nullptr) { return false; }

    Control control;
    if (!control.ParseFromString(*message)) { return false; }

    control_values.steer = control.steer();
    control_values.gas = control.gas();
    control_values.brake = control.brake();
    control_values.hand_brake = control.hand_brake();
    control_values.reverse = control.reverse();

    return true;
  }
/*
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
*/
  void CarlaCommunication::sendScene(const Scene_Values &values) {

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

/*
  bool CarlaCommunication::tryReadSceneInit(uint32_t &scene) {
    scene = 0u;

    std::unique_ptr<std::string> info = _worldThread.tryPop();
    if (info == nullptr) { return false; }

    SceneInit sceneInit;

    if (!sceneInit.ParseFromString(*info)) { return false; }

    scene = sceneInit.scene();
    return true;
  }
*/

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

  bool CarlaCommunication::tryReadRequestNewEpisode(std::string &init_file) {
    std::unique_ptr<std::string> request = _worldThread.tryPop();

    if (request == nullptr) {
      return false;
    }

    RequestNewEpisode reqEpisode;

    if (!reqEpisode.ParseFromString(*request)) {

      _worldThread.undoPop(std::move(request));

      return false;
    } else {

      init_file = reqEpisode.init_file();

      log_debug("Received init file:\n", init_file);

      return true;
    }
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
