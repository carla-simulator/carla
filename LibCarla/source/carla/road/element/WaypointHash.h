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

  struct Waypoint;

  struct WaypointHash {

    using argument_type = Waypoint;

    using result_type = uint64_t;

    /// Generates an unique id for @a waypoint based on its road_id, lane_id,
    /// and "s" offset. The "s" offset is truncated to half centimetre
    /// precision.
    uint64_t operator()(const Waypoint &waypoint) const;
  };

} // namespace element
} // namespace road
} // namespace carla
