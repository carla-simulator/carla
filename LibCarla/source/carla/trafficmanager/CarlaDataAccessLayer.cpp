// Copyright (c) 2019 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "CarlaDataAccessLayer.h"

namespace carla {
namespace traffic_manager {

  CarlaDataAccessLayer::CarlaDataAccessLayer(carla::SharedPtr<cc::Map> _world_map) {
    world_map = _world_map;
  }

  using WaypointPtr = carla::SharedPtr<cc::Waypoint>;
  std::vector<std::pair<WaypointPtr, WaypointPtr>> CarlaDataAccessLayer::GetTopology() const {
    return world_map->GetTopology();
  }
} // namespace traffic_manager
} // namespace carla
