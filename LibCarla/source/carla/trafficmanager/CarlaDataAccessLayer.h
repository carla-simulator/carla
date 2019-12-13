// Copyright (c) 2019 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include <vector>

#include "carla/client/Map.h"
#include "carla/client/Waypoint.h"
#include "carla/Memory.h"

namespace carla {
namespace traffic_manager {

  namespace cc = carla::client;

  /// This class is responsible for retrieving data from the server.
  class CarlaDataAccessLayer {

  private:

    /// Pointer to Carla's map object.
    carla::SharedPtr<cc::Map> world_map;

  public:

    CarlaDataAccessLayer(carla::SharedPtr<cc::Map> world_map);

    /// This method retrieves a list of topology segments from the simulator.
    using WaypointPtr = carla::SharedPtr<cc::Waypoint>;
    std::vector<std::pair<WaypointPtr, WaypointPtr>> GetTopology() const;

  };

} // namespace traffic_manager
} // namespace carla
