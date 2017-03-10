// CARLA, Copyright (C) 2017 Computer Vision Center (CVC)

#include "Carla.h"
#include "CarlaServer.h"

#include <iostream>
#include <memory>

namespace carla {

namespace server {

  // -- CarlaServer ------------------------------------------------------------

  CarlaServer::CarlaServer(int worldPort, int writePort, int readPort, int modesCount, int scenesCount) :
	  _communication(std::make_unique<CarlaCommunication>(worldPort, writePort, readPort)), _proto(std::make_unique<Protocol>(this)),
	  _modes(modesCount),
	  _scenes(scenesCount){

  }

  CarlaServer::~CarlaServer() {}

  void CarlaServer::sendReward(const Reward_Values &values) {
	  Reward reward = _proto->LoadReward(values);
	  _communication->sendReward(reward);
  }

  void CarlaServer::sendSceneValues(const Scene_Values &values) {

	  std::cout << "Send Scene Values" << std::endl;
	  Scene scene = _proto->LoadScene(values);
	  _communication->sendScene(scene);
  }

  void CarlaServer::sendEndReset() {
	  EpisodeReady eReady;
	  eReady.set_ready(true);
	  _communication->sendReset(eReady);
  }

  void CarlaServer::sendWorld() {
	  std::cout << "Send World" << std::endl;
	  World world = _proto->LoadWorld();
	  _communication->sendWorld(world);
  }

  bool CarlaServer::tryReadControl(float &steer, float &gas) {
	std::string controlMessage;
    bool error = !_communication->tryReadControl(controlMessage);
	Control control;
	if (!error) {
		error &= !control.ParseFromString(controlMessage);
	}
	steer = control.steer();
	gas = control.gas();

	if (error) {
		steer = 0.0f;
		gas = 0.0f;
	}
	else {
		steer = control.steer();
		gas = control.gas();
		std::cout << "Steer: " << steer << " Gas: " << gas << std::endl;
	}

	return !error;
  }

  bool CarlaServer::tryReadSceneInit(int &mode, int &scene) {
	  std::string initMessage;
	  bool error = !_communication->tryReadWorldInfo(initMessage);
	  SceneInit sceneInit;

	  if (!error) {
		  error &= !sceneInit.ParseFromString(initMessage);
	  }
	  
	  if (error) {
		  mode = -1; 
		  scene = -1;
	  }
	  else {
		  mode = sceneInit.mode();
		  scene = sceneInit.scene();
		  std::cout << "Mode: " << mode << " Scene: " << scene << std::endl;
	  }

	  return !error;
  }

  bool CarlaServer::tryReadEpisodeStart(float &start_index, float &end_index) {
	  std::string startData;
	  bool error = !_communication->tryReadWorldInfo(startData);
	  EpisodeStart episodeStart;
	  error &= !episodeStart.ParseFromString(startData);

	  if (error) {
		  start_index = 0.0f;
		  end_index = 0.0f;
	  }
	  else {
		  start_index = episodeStart.start_index();
		  end_index = episodeStart.end_index();
		  std::cout << "Start: " << start_index << " End: " << end_index << std::endl;
	  }

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
