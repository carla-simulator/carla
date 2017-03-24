#include "CarlaServer.h"

#include <carla/server/Server.h>

namespace carla {

  Reward_Values::~Reward_Values() {}

  Scene_Values::~Scene_Values() {}

  class CarlaServer::Pimpl : public carla::server::Server {
  public:
    Pimpl(uint32_t worldPort, uint32_t writePort, uint32_t readPort) :
      carla::server::Server(worldPort, writePort, readPort) {}
  };

  CarlaServer::CarlaServer(uint32_t writePort, uint32_t readPort, uint32_t worldPort) :
    _pimpl(std::make_unique<Pimpl>(worldPort, writePort, readPort)) {}

  CarlaServer::~CarlaServer() {}

  bool CarlaServer::init(uint32_t LevelCount) {
    if (!worldConnected() && !clientConnected() && !serverConnected()) return false;
    _pimpl->sendWorld(static_cast<uint32_t>(Mode::NUMBER_OF_MODES), LevelCount);
    return true;
  }

  bool CarlaServer::tryReadSceneInit(Mode &mode, uint32_t &scene, bool &readed) {
    if (!worldConnected()) return false;
    readed = _pimpl->tryReadSceneInit(mode, scene);
    return true;
  }

  bool CarlaServer::tryReadEpisodeStart(uint32_t &startIndex, uint32_t &endIndex, bool &readed) {
    if (!worldConnected()) return false;
    readed = _pimpl->tryReadEpisodeStart(startIndex, endIndex);
    return true;
  }

  bool CarlaServer::tryReadControl(float &steer, float &throttle, bool &readed) {
    if (!clientConnected()) return false;
    readed = _pimpl->tryReadControl(steer, throttle);
    return true;
  }

  bool CarlaServer::newEpisodeRequested(bool &newEpisode) {
    if (!worldConnected()) return false;
    newEpisode = _pimpl->tryReadRequestNewEpisode();
    return true;
  }

  bool CarlaServer::sendReward(Reward_Values *values) {
    std::unique_ptr<Reward_Values> ptr(values);
    if (!serverConnected()) return false;
    _pimpl->sendReward(std::move(ptr));
    return true;
  }

  bool CarlaServer::sendSceneValues(const Scene_Values &values) {
    if (!worldConnected()) return false;
    _pimpl->sendSceneValues(values);
    return true;
  }

  bool CarlaServer::sendEndReset() {
    if (!worldConnected()) return false;
    _pimpl->sendEndReset();
    return true;
  }

  bool CarlaServer::worldConnected(){
    return _pimpl->worldConnected() && !_pimpl->needsRestart();
  }

  bool CarlaServer::clientConnected(){
    return _pimpl->clientConnected() && !_pimpl->needsRestart();
  }

  bool CarlaServer::serverConnected(){
    return _pimpl->serverConnected() && !_pimpl->needsRestart();
  }

  bool CarlaServer::needsRestart(){
    return _pimpl->needsRestart();
  }

} // namespace carla
