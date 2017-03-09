// CARLA, Copyright (C) 2017 Computer Vision Center (CVC)

#include "Carla.h"
#include "CarlaServer.h"

#include <iostream>
#include <memory>

namespace carla {

namespace server {

  // -- CarlaServer ------------------------------------------------------------

  CarlaServer::CarlaServer(int writePort, int readPort, int worldPort, int modesCount, int scenesCount) :
	  _communication(std::make_unique<CarlaCommunication>(writePort, readPort, worldPort)), _proto(std::make_unique<Protocol>(this)),
	  _modes(modesCount),
	  _scenes(scenesCount){}

  CarlaServer::~CarlaServer() {}

  void CarlaServer::sendReward(const Reward_Values &values) {
	  Reward reward = _proto->LoadReward(values);
	  _communication->sendReward(reward);
  }

  void CarlaServer::sendSceneValues(const Scene_Values &values) {
	  Scene scene = _proto->LoadScene(values);
	  _communication->sendScene(scene);
  }

  void CarlaServer::sendEndReset() {
	  EpisodeReady eReady;
	  eReady.set_ready(true);
	  _communication->sendReset(eReady);
  }

  void CarlaServer::sendWord() {
	  World world = _proto->LoadWorld();
	  _communication->sendWorld(world);
  }

  bool CarlaServer::tryReadControl(float &steer, float &gas) {
	std::string controlMessage;
    bool error = !_communication->tryReadControl(controlMessage);
	Control control;
	error &= !control.ParseFromString(controlMessage);
	steer = control.steer();
	gas = control.gas();

	return !error;
  }

  bool CarlaServer::tryReadSceneInit(int &mode, int &scene) {
	  std::string initMessage;
	  bool error = !_communication->tryReadWorldInfo(initMessage);
	  SceneInit sceneInit;
	  error &= !sceneInit.ParseFromString(initMessage);
	  mode = sceneInit.mode();
	  scene = sceneInit.scene();

	  return !error;
  }

  bool CarlaServer::tryReadEpisodeStart(float &start_index, float &end_index) {
	  std::string startData;
	  bool error = !_communication->tryReadWorldInfo(startData);
	  EpisodeStart episodeStart;
	  error &= !episodeStart.ParseFromString(startData);
	  start_index = episodeStart.start_index();
	  end_index = episodeStart.end_index();

	  return !error;
  }

  void CarlaServer::setMode(Mode mode) {
	  _mode = mode;
  }

  Mode CarlaServer::GetMode() const {
	  return _mode;
  }

  void CarlaServer::SetScene(int scene) {
	  _scene = scene;
  }

  int CarlaServer::GetScene() const {
	  return _scene;
  }

  int CarlaServer::GetModesCount() const {
	  return _modes;
  }

  int CarlaServer::GetScenesCount() const {
	  return _scenes;
  }

  void CarlaServer::SetReset(bool reset) {
	  _reset = reset;
  }

  bool CarlaServer::Reset() const {
	  return _reset;
  }

} // namespace server
} // namespace carla
