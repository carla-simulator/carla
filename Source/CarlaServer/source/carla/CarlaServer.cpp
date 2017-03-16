#include "server/Server.h"

explicit CarlaServer::CarlaServer(uint32 writePort, uint32 readPort, uint32 worldPort):
_server(std::make_unique<Server>(worldPort, writePort, readPort)){}

CarlaServer::~CarlaServer(){}

void CarlaServer::init(uint32 LevelCount) {
  _server->sendWorld(Mode::NUMBER_OF_MODES, LevelCount);
}


bool tryReadSceneInit(Mode &mode, int &scene){
  _server->tryReadSceneInit(mode, scene);
}


bool tryReadEpisodeStart(uint32 &startIndex, uint32 &endIndex){
  _server->tryReadEpisodeStart(startIndex, endIndex);
}

bool tryReadControl(float &steer, float &throttle){
  return _server->tryReadControl(steer, throttle);
}


void sendReward(const Reward_Values &values){
  _server->sendReward(values);
}

    /// Send the values of the generated scene.
void sendSceneValues(const Scene_Values &values){
  _server->sendSceneValues(values);
}

void sendEndReset(){
  _server->sendEndReset();
}

