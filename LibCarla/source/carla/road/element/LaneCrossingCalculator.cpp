// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "carla/road/element/LaneCrossingCalculator.h"
#include "carla/road/element/LaneMarking.h"

#include "carla/geom/Location.h"
#include "carla/road/Map.h"

namespace carla {
namespace road {
namespace element {

  /// @todo Temporary flags to search lanes where we can find road marks.
  /// This needs to be expanded searching in shoulders too, but since
  /// shouders have a small width, they can cause problems while finding
  /// the nearest center of a lane given a location that are in a road,
  /// but very close to a shoulder.
  static constexpr uint32_t FLAGS =
      static_cast<uint32_t>(Lane::LaneType::Driving) |
      static_cast<uint32_t>(Lane::LaneType::Bidirectional) |
      static_cast<uint32_t>(Lane::LaneType::Biking) |
      static_cast<uint32_t>(Lane::LaneType::Parking);

  /// Calculate the lane markings that need to be crossed from @a lane_id_origin
  /// to @a lane_id_destination.
  static std::vector<LaneMarking> CrossingAtSameSection(
      const Map &map,
      const Waypoint *w0,
      const Waypoint *w1,
      const bool w0_is_offroad,
      const bool dest_is_at_right) {
    auto w0_marks = map.GetMarkRecord(*w0);
    auto w1_marks = map.GetMarkRecord(*w1);

    if (dest_is_at_right) {
      if (w0_is_offroad) {
        return { LaneMarking(*w1_marks.second) };
      } else {
        return { LaneMarking(*w0_marks.first) };
      }
    } else {
      if (w0_is_offroad) {
        return { LaneMarking(*w1_marks.first) };
      } else {
        return { LaneMarking(*w0_marks.second) };
      }
    }

    return {};
  }

  static bool IsOffRoad(const Map &map, const geom::Location &location) {
    return !map.GetWaypoint(location, FLAGS).has_value();
  }

  std::vector<LaneMarking> LaneCrossingCalculator::Calculate(
      const Map &map,
      const geom::Location &origin,
      const geom::Location &destination) {
    auto w0 = map.GetClosestWaypointOnRoad(origin, FLAGS);
    auto w1 = map.GetClosestWaypointOnRoad(destination, FLAGS);

    if (!w0.has_value() || !w1.has_value()) {
      return {};
    }

    if (w0->road_id != w1->road_id || w0->section_id != w1->section_id) {
      /// @todo This case should also be handled.
      return {};
    }

    if (map.IsJunction(w0->road_id) || map.IsJunction(w1->road_id)) {
      return {};
    }

    const auto w0_is_offroad = IsOffRoad(map, origin);
    const auto w1_is_offroad = IsOffRoad(map, destination);

    if (w0_is_offroad && w1_is_offroad) {
      // outside the road
      return {};
    }

    if ((w0->lane_id == w1->lane_id) && !w0_is_offroad && !w1_is_offroad) {
      // both at the same lane and inside the road
      return {};
    }

    const auto transform = map.ComputeTransform(*w0);
    geom::Vector3D orig_vec = transform.GetForwardVector();
    geom::Vector3D dest_vec = (destination - origin).MakeSafeUnitVector(2 * std::numeric_limits<float>::epsilon());

    // cross product
    const auto dest_is_at_right =
        (-orig_vec.x * dest_vec.y + orig_vec.y * dest_vec.x) < 0;

    return CrossingAtSameSection(
        map,
        &*w0,
        &*w1,
        w0_is_offroad,
        dest_is_at_right);
  }

} // namespace element
} // namespace road
} // namespace carla
