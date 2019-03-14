// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/road/RoadTypes.h"

#include <cstdint>

namespace carla {
namespace road {
namespace element {

  struct Waypoint {

    RoadId road_id = 0u;

    LaneId lane_id = 0;

    float s = 0.0f;
  };

} // namespace element
} // namespace road
} // namespace carla
