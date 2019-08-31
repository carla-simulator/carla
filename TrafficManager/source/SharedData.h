#pragma once

#include <chrono>
#include <memory>

#include "carla/client/Client.h"
#include "carla/client/Actor.h"

#include "InMemoryMap.h"
#include "CreateGrid.h"
#include "TrafficDistributor.h"

namespace traffic_manager {

  typedef std::chrono::time_point<
      std::chrono::_V2::system_clock,
      std::chrono::nanoseconds
      > TimeInstance;

  struct StateEntry {
    float deviation;
    float velocity;
    TimeInstance time_instance;
    float deviation_integral;
    float velocity_integral;
  };

  class SharedData {
    /// This class is used to share common data among threads.

  public:

    carla::client::Client *client;
    carla::client::DebugHelper *debug;
    std::shared_ptr<InMemoryMap> local_map;
    TrafficDistributor traffic_distributor;
    CreateGrid Grid;

    std::vector<carla::SharedPtr<carla::client::Actor>> registered_actors;
    std::map<int, StateEntry> state_map;

    SharedData();
    ~SharedData();

    void Destroy() {
    for (auto actor: registered_actors) {
      if (
          actor != nullptr
          and
          actor->IsAlive()) {
          actor->Destroy();
        }
      }
    }
  };

}
