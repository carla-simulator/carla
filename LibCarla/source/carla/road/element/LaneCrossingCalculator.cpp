// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "carla/road/element/LaneCrossingCalculator.h"
#include "carla/road/element/LaneMarking.h"

#include "carla/geom/Location.h"
#include "carla/road/Map.h"

#include <cstdlib>

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
      const bool dest_is_at_outer_side) {
    auto w0_marks = map.GetMarkRecord(*w0);
    auto w1_marks = map.GetMarkRecord(*w1);

    if (dest_is_at_outer_side) {
      if (w0_is_offroad) {
        if (w1_marks.second != nullptr) {
          return { LaneMarking(*w1_marks.second) };
        }
      } else {
        if (w0_marks.first != nullptr) {
          return { LaneMarking(*w0_marks.first) };
        }
      }
    } else {
      if (w0_is_offroad) {
        if (w1_marks.first != nullptr) {
          return { LaneMarking(*w1_marks.first) };
        }
      } else {
        if (w0_marks.second != nullptr) {
          return { LaneMarking(*w0_marks.second) };
        }
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
    geom::Vector3D dest_vec = (destination - origin).MakeUnitVector(2 * std::numeric_limits<float>::epsilon());

    // cross product
    const auto dest_is_at_right =
        (-orig_vec.x * dest_vec.y + orig_vec.y * dest_vec.x) < 0;

    // Mark records are stored outer-first. Driver-right is the outer side
    // only on RHT roads; LHT roads have the opposite boundary ordering.
    const bool dest_is_at_outer_side =
        dest_is_at_right == map.GetLane(*w0).GetRoad()->IsRHT();

    // A lateral move may cross several boundaries within one simulation step.
    // Include the entry/exit boundary when exactly one endpoint is off-road.
    const auto lane_distance = std::abs(w1->lane_id - w0->lane_id);
    const auto boundaries = lane_distance - ((w0->lane_id < 0) != (w1->lane_id < 0)) +
        (w0_is_offroad || w1_is_offroad);
    if (boundaries > 1) {
      std::vector<LaneMarking> result;
      const auto append_edge = [&](const Waypoint &edge, bool entering) {
        // The far edge can belong to the opposite carriageway, whose driver
        // direction is reversed. Classify this boundary using its own lane.
        const auto forward = map.ComputeTransform(edge).GetForwardVector();
        const bool towards_right =
            (-forward.x * dest_vec.y + forward.y * dest_vec.x) < 0;
        const bool towards_outer = towards_right == map.GetLane(edge).GetRoad()->IsRHT();
        const auto marks = CrossingAtSameSection(
            map, &edge, &edge, entering, towards_outer);
        result.insert(result.end(), marks.begin(), marks.end());
      };
      if (w0_is_offroad) {
        append_edge(*w0, true);
      }
      auto waypoint = *w0;
      const auto step = w1->lane_id > w0->lane_id ? 1 : -1;
      while (waypoint.lane_id != w1->lane_id) {
        const auto marks = map.GetMarkRecord(waypoint);
        const bool moving_outwards = (waypoint.lane_id < 0) == (step < 0);
        const auto *mark = moving_outwards ? marks.first : marks.second;
        if (mark != nullptr) {
          result.emplace_back(*mark);
        }
        waypoint.lane_id += step;
        if (waypoint.lane_id == 0) {
          waypoint.lane_id += step;
        }
      }
      if (w1_is_offroad) {
        append_edge(*w1, false);
      }
      return result;
    }

    return CrossingAtSameSection(
        map,
        &*w0,
        &*w1,
        w0_is_offroad,
        dest_is_at_outer_side);
  }

} // namespace element
} // namespace road
} // namespace carla
