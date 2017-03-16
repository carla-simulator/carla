#pragma once

#include "carla_protocol.pb.h"

#include <string>

namespace carla {

  struct Reward_Values;
  struct Scene_Values;

namespace server {

  class Server;

  class Protocol {

  public:

	  Protocol(Server *server);
	  ~Protocol();

	  Reward LoadReward(const Reward_Values &values);

	  Scene LoadScene(const Scene_Values &values);

	  World LoadWorld(const int modes, const int scenes);

  private:

	  carla::server::Server *_server;

  };

}
}



