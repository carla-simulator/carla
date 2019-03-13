// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include <cstdint>

namespace carla {
namespace road {
namespace element {

  struct Waypoint {

    /// @todo Don't define here.
    using RoadId = uint32_t;
    using LaneId = int32_t;
    using RoadDistance = float;

    RoadId road_id = 0u;

    LaneId lane_id = 0;

    RoadDistance s = 0.0f;
  };

} // namespace element
} // namespace road
} // namespace carla
