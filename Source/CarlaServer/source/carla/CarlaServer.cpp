#include "CarlaServer.h"

#include <carla/server/CarlaCommunication.h>

namespace carla {

  Image::Image() {}

  Image::~Image() {}

  Reward_Values::Reward_Values() {}

  Reward_Values::~Reward_Values() {}

  Scene_Values::Scene_Values() {}

  Scene_Values::~Scene_Values() {}

  class CarlaServer::Pimpl {
  public:

    template<typename... Args>
    Pimpl(Args&&... args) : communication(std::forward<Args>(args)...) {}

    carla::server::CarlaCommunication communication;
  };

  CarlaServer::CarlaServer(uint32_t writePort, uint32_t readPort, uint32_t worldPort) :
    _pimpl(std::make_unique<Pimpl>(worldPort, writePort, readPort)) {}

  CarlaServer::~CarlaServer() {}

/*
  bool CarlaServer::init(uint32_t levelCount) {
    if (!worldConnected() && !clientConnected() && !serverConnected())
      return false;
    _pimpl->communication.sendWorld(levelCount);
    return true;
  }

  bool CarlaServer::tryReadSceneInit(uint32_t &scene, bool &readed) {
    if (!worldConnected())
      return false;
    readed = _pimpl->communication.tryReadSceneInit(scene);
    return true;
  }
*/

  bool CarlaServer::tryReadEpisodeStart(uint32_t &startIndex, uint32_t &endIndex, bool &readed) {
    if (!worldConnected())
      return false;
    readed = _pimpl->communication.tryReadEpisodeStart(startIndex, endIndex);
    return true;
  }

  bool CarlaServer::tryReadControl(float &steer, float &throttle, bool &readed) {
    if (!clientConnected())
      return false;
    readed = _pimpl->communication.tryReadControl(steer, throttle);
    return true;
  }

  bool CarlaServer::newEpisodeRequested(std::string &init_file, bool &readed) {
    if (!worldConnected())
      return false;
    
    readed = _pimpl->communication.tryReadRequestNewEpisode(init_file);

    return true;
  }

  bool CarlaServer::sendReward(Reward_Values *values) {
    std::unique_ptr<Reward_Values> ptr(values);
    if (!serverConnected())
      return false;
    _pimpl->communication.sendReward(std::move(ptr));
    return true;
  }

  bool CarlaServer::sendSceneValues(const Scene_Values &values) {
    if (!worldConnected())
      return false;
    _pimpl->communication.sendScene(values);
    return true;
  }

  bool CarlaServer::sendEndReset() {
    if (!worldConnected())
      return false;
    _pimpl->communication.sendReset();
    return true;
  }

  bool CarlaServer::worldConnected(){
    return _pimpl->communication.worldConnected() && !_pimpl->communication.needsRestart();
  }

  bool CarlaServer::clientConnected(){
    return _pimpl->communication.clientConnected() && !_pimpl->communication.needsRestart();
  }

  bool CarlaServer::serverConnected(){
    return _pimpl->communication.serverConnected() && !_pimpl->communication.needsRestart();
  }

  bool CarlaServer::needsRestart(){
    return _pimpl->communication.needsRestart();
  }

} // namespace carla
