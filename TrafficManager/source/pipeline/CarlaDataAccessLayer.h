#pragma once

#include <array>
#include <vector>

#include "carla/client/Map.h"
#include "carla/client/Waypoint.h"
#include "carla/Memory.h"

namespace traffic_manager {

  class CarlaDataAccessLayer {
    /// This class is responsible for retrieving data from server

  private:

    carla::SharedPtr<carla::client::Map> world_map;

  public:

    CarlaDataAccessLayer(carla::SharedPtr<carla::client::Map> world_map);
    ~CarlaDataAccessLayer();

    std::vector<
        std::pair<
        carla::SharedPtr<carla::client::Waypoint>,
        carla::SharedPtr<carla::client::Waypoint
        >
        >
        > GetTopology() const; /// Retrieves list of topology segments from the

    /// simulator

  };

}
