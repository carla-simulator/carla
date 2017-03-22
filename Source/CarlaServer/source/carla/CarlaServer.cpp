#include "CarlaServer.h"

#include <carla/server/Server.h>

namespace carla {

  class CarlaServer::Pimpl : public carla::server::Server {
  public:
    Pimpl(uint32_t worldPort, uint32_t writePort, uint32_t readPort) :
      carla::server::Server(worldPort, writePort, readPort) {}
  };

  CarlaServer::CarlaServer(uint32_t writePort, uint32_t readPort, uint32_t worldPort) :
    _pimpl(std::make_unique<Pimpl>(worldPort, writePort, readPort)) {}

  CarlaServer::~CarlaServer() {}

  void CarlaServer::init(uint32_t LevelCount) {
    _pimpl->sendWorld(static_cast<uint32_t>(Mode::NUMBER_OF_MODES), LevelCount);
  }

  bool CarlaServer::tryReadSceneInit(Mode &mode, uint32_t &scene) {
    return _pimpl->tryReadSceneInit(mode, scene);
  }

  bool CarlaServer::tryReadEpisodeStart(uint32_t &startIndex, uint32_t &endIndex) {
    return _pimpl->tryReadEpisodeStart(startIndex, endIndex);
  }

  bool CarlaServer::tryReadControl(float &steer, float &throttle) {
    return _pimpl->tryReadControl(steer, throttle);
  }

  bool CarlaServer::newEpisodeRequested() {
    return _pimpl->tryReadRequestNewEpisode();
  }

  bool CarlaServer::sendReward(const Reward_Values &values) {
    if (needsRestart()) return false;
    _pimpl->sendReward(values);
    return true;
  }

  bool CarlaServer::sendSceneValues(const Scene_Values &values) {
    if (needsRestart()) return false;
    _pimpl->sendSceneValues(values);
    return true;
  }

  bool CarlaServer::sendEndReset() {
    if (needsRestart()) return false;
    _pimpl->sendEndReset();
    return true;
  }

  /*bool CarlaServer::worldConnected(){
    return _pimpl->worldConnected();
  }

  bool CarlaServer::clientConnected(){
    return _pimpl->clientConnected();
  }

  bool CarlaServer::serverConnected(){
    return _pimpl->serverConnected();
  }

  bool CarlaServer::needRestart() {
    return _pimpl->needRestart();
  }*/

  bool CarlaServer::needsRestart(){
    return _pimpl->needsRestart() || (!_pimpl->worldConnected() || !_pimpl->clientConnected() || !_pimpl->serverConnected());
  }

} // namespace carla
