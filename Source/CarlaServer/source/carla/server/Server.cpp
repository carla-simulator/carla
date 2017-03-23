// CARLA, Copyright (C) 2017 Computer Vision Center (CVC)

#include "Server.h"

#include "CarlaCommunication.h"
#include "carla_protocol.pb.h"

#include <iostream>
#include <memory>

namespace carla {
namespace server {

  

  // -- CarlaServer ------------------------------------------------------------

  Server::Server(uint32_t worldPort, uint32_t writePort, uint32_t readPort) :
    _communication(std::make_unique<CarlaCommunication>(worldPort, writePort, readPort)){}

  Server::~Server() {
  }

  void Server::sendReward( std::unique_ptr<Reward_Values> values) {
    //Reward reward;
    //_proto->LoadReward(reward, values);
    //_communication->sendReward(reward);
    _communication->sendReward(std::move(values));
  }

  void Server::sendSceneValues(const Scene_Values &values) {
    //Scene scene;
    //_proto->LoadScene(scene, values);
    //_communication->sendScene(scene);

    /*std::cout << "POSSIBLE POSITIONS 1"<< std::endl;

    for (int i=0; i<values.possible_positions.size(); ++i){
      std::cout << "   x: " << values.possible_positions[i].x << " y: " << values.possible_positions[i].y << std::endl;
    }*/

    _communication->sendScene(values);
  }

  void Server::sendEndReset() {
    
    _communication->sendReset();
  }

  void Server::sendWorld(const uint32_t modes, const uint32_t scenes) {
    //World world;
    //_proto->LoadWorld(world, modes, scenes);
    //_communication->sendWorld(world);
    _communication->sendWorld(modes, scenes);
  }

  bool Server::tryReadControl(float &steer, float &gas) {
    return _communication->tryReadControl(steer, gas);
  }

  bool Server::tryReadSceneInit(Mode &mode, uint32_t &scene) {
    return _communication->tryReadSceneInit(mode, scene);
  }

  bool Server::tryReadEpisodeStart(uint32_t &start_index, uint32_t &end_index) {
    return _communication->tryReadEpisodeStart(start_index, end_index);
  }

  bool Server::tryReadRequestNewEpisode(){
    return _communication->tryReadRequestNewEpisode();
  }

  Mode Server::GetMode() const {
    return _communication->GetMode();
  }

  void Server::SetScene(int scene) {
    _scene = scene;
  }

  bool Server::worldConnected() const{
    return _communication->worldConnected();
  }

  bool Server::clientConnected() const{
    return _communication->clientConnected();
  }

  bool Server::serverConnected() const{
    return _communication->serverConnected();
  }

  bool Server::needsRestart() const {
    return _communication->NeedsRestart();
  }

} // namespace server
} // namespace carla
