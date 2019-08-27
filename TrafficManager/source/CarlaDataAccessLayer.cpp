// Implementation for CarlaDataAccessLayer

#include "CarlaDataAccessLayer.h"

namespace traffic_manager {

  CarlaDataAccessLayer::CarlaDataAccessLayer(carla::SharedPtr<carla::client::Map> world_map) {
    this->world_map = world_map;
  }

  CarlaDataAccessLayer::~CarlaDataAccessLayer() {}

  std::vector<
      std::pair<
      carla::SharedPtr<carla::client::Waypoint>,
      carla::SharedPtr<carla::client::Waypoint>
      >
      > CarlaDataAccessLayer::getTopology() const {
    return world_map->GetTopology();
  }
}
