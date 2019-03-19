// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "carla/road/element/LaneCrossingCalculator.h"

#include "carla/geom/Location.h"
#include "carla/road/Map.h"

namespace carla {
namespace road {
namespace element {

  /// Calculate the lane markings that need to be crossed from @a lane_id_origin
  /// to @a lane_id_destination.
  ///
  /// @todo This should use the info in the OpenDrive instead.
  static std::vector<LaneMarking> CrossingAtSameSection(
      const int lane_id_origin,
      const int lane_id_destination,
      const bool origin_is_offroad,
      const bool destination_is_offroad) {
    if (origin_is_offroad != destination_is_offroad) {
      return { LaneMarking::Solid };
    } else if (lane_id_origin == lane_id_destination) {
      return {};
    } else if (lane_id_origin * lane_id_destination < 0) {
      return { LaneMarking::Solid };
    } else {
      return { LaneMarking::Broken };
    }
  }

  static bool IsOffRoad(const Map &map, const geom::Location &location) {
    return !map.GetWaypoint(location).has_value();
  }

  std::vector<LaneMarking> LaneCrossingCalculator::Calculate(
      const Map &map,
      const geom::Location &origin,
      const geom::Location &destination) {
    auto w0 = map.GetClosestWaypointOnRoad(origin);
    auto w1 = map.GetClosestWaypointOnRoad(destination);
    if (w0.road_id != w1.road_id) {
      /// @todo This case should also be handled.
      return {};
    }
    if (map.IsJunction(w0.road_id) || map.IsJunction(w1.road_id)) {
      return {};
    }
    return CrossingAtSameSection(
        w0.lane_id,
        w1.lane_id,
        IsOffRoad(map, origin),
        IsOffRoad(map, destination));
  }

} // namespace element
} // namespace road
} // namespace carla
