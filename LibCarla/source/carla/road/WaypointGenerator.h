// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/road/element/Waypoint.h"

#include <utility>
#include <vector>

namespace carla {
namespace road {

  class Map;

  class WaypointGenerator {
  public:

    using Waypoint = element::Waypoint;

    /// Return the list of waypoints placed at the entrance of each drivable
    /// successor lane; i.e., the list of each waypoint in the next road segment
    /// that a vehicle could drive from @a waypoint.
    static std::vector<Waypoint> GetSuccessors(
        const Waypoint &waypoint);

    /// Return the list of waypoints at @a distance such that a vehicle at @a
    /// waypoint could drive to.
    static std::vector<Waypoint> GetNext(
        const Waypoint &waypoint,
        double distance);

    /// Generate all the waypoints in @a map separated by @a approx_distance.
    static std::vector<Waypoint> GenerateAll(
        const Map &map,
        double approx_distance);

    /// Generate the minimum set of waypoints that define the topology of @a
    /// map. The waypoints are placed at the entrance of each lane.
    static std::vector<std::pair<Waypoint, Waypoint>> GenerateTopology(
        const Map &map);
  };

} // namespace road
} // namespace carla
