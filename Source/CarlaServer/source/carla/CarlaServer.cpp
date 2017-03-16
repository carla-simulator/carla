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

  void CarlaServer::sendReward(const Reward_Values &values) {
    _pimpl->sendReward(values);
  }

  void CarlaServer::sendSceneValues(const Scene_Values &values) {
    _pimpl->sendSceneValues(values);
  }

  void CarlaServer::sendEndReset() {
    _pimpl->sendEndReset();
  }

} // namespace carla
