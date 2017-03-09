#pragma once

#include "carla\Protocol\carlaProtocol\carla_protocol.pb.h"

//#include "carla\server\CarlaServer.h"
#include <string>

namespace carla {
namespace server {
  class CarlaServer;
  struct Reward_Values;
  struct Scene_Values; 

  class Protocol {

  public:

	  Protocol(CarlaServer *server);
	  ~Protocol();

	  Reward LoadReward(const Reward_Values &values);

	  Scene LoadScene(const Scene_Values &values);

	  World LoadWorld();


  private:

	  carla::server::CarlaServer *_server;

  };

}
}



