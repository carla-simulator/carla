// CARLA, Copyright (C) 2017 Computer Vision Center (CVC)

#include "Carla.h"
#include "Server.h"

#include <iostream>
#include <memory>

namespace carla {
namespace server {

  // -- CarlaServer ------------------------------------------------------------

  Server::Server(int worldPort, int writePort, int readPort) :
    _communication(std::make_unique<CarlaCommunication>(worldPort, writePort, readPort)),
     _proto(std::make_unique<Protocol>(this)){}

  Server::~Server() {
  }

  void Server::sendReward(const Reward_Values &values) {
    Reward reward = _proto->LoadReward(values);
    _communication->sendReward(reward);
  }

  void Server::sendSceneValues(const Scene_Values &values) {
    Scene scene = _proto->LoadScene(values);
    _communication->sendScene(scene);
  }

  void Server::sendEndReset() {
    EpisodeReady eReady;
    eReady.set_ready(true);
    _communication->sendReset(eReady);
  }

  void Server::sendWorld(const int modes, const int scenes) {
    World world = _proto->LoadWorld(modes, scenes);
    _communication->sendWorld(world);
  }

  bool Server::tryReadControl(float &steer, float &gas) {
  std::string controlMessage;
    bool success = _communication->tryReadControl(controlMessage);
  Control control;
  if (success) {
    success &= control.ParseFromString(controlMessage);
  }
  steer = control.steer();
  gas = control.gas();

  if (!success) {
    steer = 0.0f;
    gas = 0.0f;
  }
  else {
    steer = control.steer();
    gas = control.gas();
    //std::cout << "Steer: " << steer << " Gas: " << gas << std::endl;
  }

  return success;
  }

  bool Server::tryReadSceneInit(int &mode, int &scene) {
    std::string initMessage;
    bool success = _communication->tryReadWorldInfo(initMessage);
    SceneInit sceneInit;

    if (success) {
      success &= sceneInit.ParseFromString(initMessage);
    }

    if (!success) {
      mode = -1;
      scene = -1;
    }
    else {
      mode = sceneInit.mode();
      scene = sceneInit.scene();
      //std::cout << "Mode: " << mode << " Scene: " << scene << std::endl;
    }

    return success;
  }

  bool Server::tryReadEpisodeStart(size_t &start_index, size_t &end_index) {
    std::string startData;
    bool success = _communication->tryReadWorldInfo(startData);
    EpisodeStart episodeStart;
    success &= episodeStart.ParseFromString(startData);

    if (!success) {
      start_index = 0.0;
      end_index = 0.0;
    }
    else {
      start_index = episodeStart.start_index();
      end_index = episodeStart.end_index();
      //std::cout << "Start: " << start_index << " End: " << end_index << std::endl;
    }

    return success;
  }

  void Server::setMode(Mode mode) {
    _mode = mode;
  }

  Mode Server::GetMode() const {
    return _mode;
  }

  void Server::SetScene(int scene) {
    _scene = scene;
  }

  int Server::GetScene() const {
    return _scene;
  }

  void Server::SetReset(bool reset) {
    _reset = reset;
  }

  bool Server::Reset() const {
    return _reset;
  }

} // namespace server
} // namespace carla
