#include "CarlaDataAccessLayer.h"

namespace traffic_manager {

  CarlaDataAccessLayer::CarlaDataAccessLayer(carla::SharedPtr<carla::client::Map> _world_map) {
    world_map = _world_map;
  }

  CarlaDataAccessLayer::~CarlaDataAccessLayer() {}

  using WaypointPtr = carla::SharedPtr<carla::client::Waypoint>;
  std::vector<std::pair<WaypointPtr, WaypointPtr>> CarlaDataAccessLayer::GetTopology() const {
    return world_map->GetTopology();
  }
}
