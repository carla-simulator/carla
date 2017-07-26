#pragma once

class Reward;
class Scene;
class World;

namespace carla {

  struct Reward_Values;
  struct Scene_Values;

namespace server {

  class CarlaCommunication;

  class Protocol {
  public:

	  explicit Protocol(CarlaCommunication *communication);

	  ~Protocol();

	  void LoadReward(Reward &reward, const Reward_Values &values);

	  void LoadScene(Scene &scene, const Scene_Values &values);

	  void LoadWorld(World &world, const int modes, const int scenes);

  private:

	  carla::server::CarlaCommunication *_communication;
  };

}
}
