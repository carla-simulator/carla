// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "carla/road/element/WaypointHash.h"

#include "carla/road/element/Waypoint.h"

#include <boost/container_hash/hash.hpp>

namespace carla {
namespace road {
namespace element {

  uint64_t WaypointHash::operator()(const Waypoint &waypoint) const {
    uint64_t seed = 0u;
    boost::hash_combine(seed, waypoint.GetRoadId());
    boost::hash_combine(seed, waypoint.GetLaneId());
    boost::hash_combine(seed, static_cast<float>(std::floor(waypoint.GetDistance() * 200.0)));
    return seed;
  }

} // namespace element
} // namespace road
} // namespace carla
