// Copyright (c) 2024 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/road/RoadTypes.h"

#include <cstdint>
#include <functional>

namespace carla {
namespace road {
namespace element {

  struct Waypoint {

    RoadId road_id = 0u;

    SectionId section_id = 0u;

    LaneId lane_id = 0;

    double s = 0.0;
  };

} // namespace element
} // namespace road
} // namespace carla

namespace std {

  template <>
  struct hash<carla::road::element::Waypoint> {

    using argument_type = carla::road::element::Waypoint;

    using result_type = uint64_t;

    /// Generates an unique id for @a waypoint based on its road_id, lane_id,
    /// section_id, and "s" offset. The "s" offset is truncated to half
    /// centimetre precision.
    result_type operator()(const argument_type &waypoint) const;

  };

} // namespace std

namespace carla {
namespace road {
namespace element {

  inline bool operator==(const Waypoint &lhs, const Waypoint &rhs) {
    auto hasher = std::hash<Waypoint>();
    return hasher(lhs) == hasher(rhs);
  }

  inline bool operator!=(const Waypoint &lhs, const Waypoint &rhs) {
    return !operator==(lhs, rhs);
  }

} // namespace element
} // namespace road
} // namespace carla
