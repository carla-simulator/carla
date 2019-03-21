// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "carla/road/Map.h"

#include "carla/Exception.h"
#include "carla/road/element/LaneCrossingCalculator.h"
#include "carla/road/element/RoadInfoGeometry.h"
#include "carla/road/element/RoadInfoLaneWidth.h"
#include "carla/geom/Math.h"

#include <stdexcept>

namespace carla {
namespace road {

  using namespace carla::road::element;

  // ===========================================================================
  // -- Error handling ---------------------------------------------------------
  // ===========================================================================

  [[ noreturn ]] static void throw_invalid_input(const char *message) {
    throw_exception(std::invalid_argument(message));
  }

#define THROW_INVALID_INPUT_ASSERT(pred) if (!(pred)) { throw_invalid_input("assert failed: " #pred); }

  // ===========================================================================
  // -- Static local methods ---------------------------------------------------
  // ===========================================================================

  template <typename T>
  static std::vector<T> ConcatVectors(std::vector<T> dst, std::vector<T> src) {
    if (src.size() > dst.size()) {
      return ConcatVectors(src, dst);
    }
    dst.insert(
        dst.end(),
        std::make_move_iterator(src.begin()),
        std::make_move_iterator(src.end()));
    return dst;
  }

  /// Return a waypoint for each drivable lane on @a lane_section.
  template <typename FuncT>
  static void ForEachDrivableLane(RoadId road_id, const LaneSection &lane_section, FuncT &&func) {
    for (const auto &pair : lane_section.GetLanes()) {
      const auto &lane = pair.second;
      if (lane.GetType() == "driving") {
        std::forward<FuncT>(func)(Waypoint{road_id, lane.GetId(), lane_section.GetDistance()});
      }
    }
  }

  /// Return a waypoint for each drivable lane on each lane section of @a road.
  template <typename FuncT>
  static void ForEachDrivableLane(const Road &road, FuncT &&func) {
    for (const auto &lane_section : road.GetLaneSections()) {
      ForEachDrivableLane(road.GetId(), lane_section, std::forward<FuncT>(func));
    }
  }

  /// Return a waypoint for each drivable lane at @a distance on @a road.
  template <typename FuncT>
  static void ForEachDrivableLaneAt(const Road &road, float distance, FuncT &&func) {
    for (const auto &lane_section : road.GetLaneSectionsAt(distance)) {
      ForEachDrivableLane(road.GetId(), lane_section, std::forward<FuncT>(func));
    }
  }

  /// returns a pair containing first = width, second = tangent
  template <typename T>
  static std::pair<float, float> ComputeTotalLaneWidth(
    const T container, const float s, const LaneId lane_id) {
    const bool negative_lane_id = lane_id < 0;
    float dist = 0.0;
    float tangent = 0.0;
    for (const auto &lane : container) {
      const auto current_polynomial =
          lane.second->template GetInfo<element::RoadInfoLaneWidth>(s)->GetPolynomial();
      float current_dist = current_polynomial.Evaluate(s);
      if (lane.first != lane_id) {
        dist += negative_lane_id ? current_dist : - current_dist;
      } else if (lane.first == lane_id) {
        current_dist *= 0.5;
        dist += negative_lane_id ? current_dist : - current_dist;
        tangent = current_polynomial.Tangent(s);
        break;
      }
    }
    return std::make_pair(dist, tangent);
  }

  // ===========================================================================
  // -- Map: Geometry ----------------------------------------------------------
  // ===========================================================================

  Waypoint Map::GetClosestWaypointOnRoad(const geom::Location &pos) const {
    // max_nearests represents the max nearests roads
    // where we will search for nearests lanes
    constexpr int max_nearests = 10;
    // in case that map has less than max_nearests lanes,
    // we will use the maximum lanes
    const int max_nearest_allowed = _data.GetRoadCount() < max_nearests ?
        _data.GetRoadCount() : max_nearests;

    double nearest_dist[max_nearests];
    std::fill(nearest_dist, nearest_dist + max_nearest_allowed,
        std::numeric_limits<double>::max());

    RoadId ids[max_nearests];
    std::fill(ids, ids + max_nearest_allowed, 0);

    double dists[max_nearests];
    std::fill(dists, dists + max_nearest_allowed, 0.0);

    for (const auto &road_pair : _data.GetRoads()) {
      const auto road = &road_pair.second;
      const auto current_dist = road->GetNearestPoint(pos);

      for (int i = 0; i < max_nearest_allowed; ++i) {
        if (current_dist.second < nearest_dist[i]) {
          // reorder nearest_dist
          for (int j = max_nearest_allowed - 1; j > i; --j) {
            nearest_dist[j] = nearest_dist[j - 1];
            ids[j] = ids[j - 1];
            dists[j] = dists[j - 1];
          }
          nearest_dist[i] = current_dist.second;
          ids[i] = road->GetId();
          dists[i] = current_dist.first;
          break;
        }
      }
    }

    // search for the nearest lane in nearest_dist
    Waypoint waypoint;
    auto nearest_lane_dist = std::numeric_limits<float>::max();
    for (int i = 0; i < max_nearest_allowed; ++i) {
      auto lane_dist = _data.GetRoad(ids[i])->GetNearestLane(dists[i], pos);

      if (lane_dist.second < nearest_lane_dist) {
        nearest_lane_dist = lane_dist.second;
        waypoint.lane_id = lane_dist.first->GetId();
        waypoint.road_id = ids[i];
        waypoint.s = dists[i];
      }
    }

    THROW_INVALID_INPUT_ASSERT(
        waypoint.s <= _data.GetRoad(waypoint.road_id)->GetLength());
    THROW_INVALID_INPUT_ASSERT(waypoint.lane_id != 0);

    return waypoint;
  }

  boost::optional<Waypoint> Map::GetWaypoint(const geom::Location &pos) const {
    Waypoint w = GetClosestWaypointOnRoad(pos);
    const auto dist = geom::Math::Distance2D(ComputeTransform(w).location, pos);
    const auto lane_width_info = GetLane(w)->GetInfo<RoadInfoLaneWidth>(w.s);
    const auto half_lane_width =
        lane_width_info->GetPolynomial().Evaluate(w.s) * 0.5f;

    if (dist < half_lane_width) {
      return w;
    }

    return boost::optional<Waypoint>{};
  }

  geom::Transform Map::ComputeTransform(Waypoint waypoint) const {
    // lane_id can't be 0
    THROW_INVALID_INPUT_ASSERT(waypoint.lane_id != 0);

    const auto road = _data.GetRoad(waypoint.road_id);
    // road cannot be nullptr
    THROW_INVALID_INPUT_ASSERT(road != nullptr);

    // must s be smaller (or eq) than road lenght and bigger (or eq) than 0?
    THROW_INVALID_INPUT_ASSERT(waypoint.s <= road->GetLength());
    THROW_INVALID_INPUT_ASSERT(waypoint.s >= 0.0f);

    const std::map<LaneId, const Lane *> lanes = road->GetLanesAt(waypoint.s);
    // check that lane_id exists on the current s
    THROW_INVALID_INPUT_ASSERT(waypoint.lane_id > lanes.begin()->first);
    THROW_INVALID_INPUT_ASSERT(waypoint.lane_id <= lanes.end()->first);

    float lane_width = 0;
    float lane_tangent = 0;
    if (waypoint.lane_id < 0) {
      // right lane
      const auto side_lanes = MakeListView(
          std::make_reverse_iterator(lanes.lower_bound(0)), lanes.rend());
      const auto computed_width =
          ComputeTotalLaneWidth(side_lanes, waypoint.s, waypoint.lane_id);
      lane_width = computed_width.first;
      lane_tangent = computed_width.second;
    } else {
      // left lane
      const auto side_lanes = MakeListView(lanes.lower_bound(1), lanes.end());
      const auto computed_width =
          ComputeTotalLaneWidth(side_lanes, waypoint.s, waypoint.lane_id);
      lane_width = computed_width.first;
      lane_tangent = computed_width.second;
    }

    // get a directed pooint in s and apply the computed lateral offet
    element::DirectedPoint dp = road->GetDirectedPointIn(waypoint.s);

    geom::Rotation rot(
        geom::Math::to_degrees(dp.pitch),
        geom::Math::to_degrees(dp.tangent),
        0.0);

    dp.ApplyLateralOffset(lane_width);

    if (waypoint.lane_id > 0) {
      rot.yaw += 180.0f + lane_tangent;
      rot.pitch = 360.0f - rot.pitch;
    } else {
      rot.yaw -= lane_tangent;
    }

    return geom::Transform(dp.location, rot);
  }

  // ===========================================================================
  // -- Map: Road information --------------------------------------------------
  // ===========================================================================

  std::string Map::GetLaneType(const Waypoint waypoint) const {
    const auto *lane = GetLane(waypoint);
    THROW_INVALID_INPUT_ASSERT(lane != nullptr);
    return lane->GetType();
  }

  double Map::GetLaneWidth(const Waypoint waypoint) const {
    const auto *lane = GetLane(waypoint);
    THROW_INVALID_INPUT_ASSERT(lane != nullptr);

    throw_exception(std::runtime_error("not implemented"));
  }

  bool Map::IsJunction(const RoadId road_id) const {
    const auto *road = _data.GetRoad(road_id);
    THROW_INVALID_INPUT_ASSERT(road != nullptr);
    return road->IsJunction();
  }

  std::pair<element::RoadInfoMarkRecord *, element::RoadInfoMarkRecord *>
  Map::GetMarkRecord(const Waypoint /*waypoint*/) const {
    throw_exception(std::runtime_error("not implemented"));
  }

  std::vector<element::LaneMarking> Map::CalculateCrossedLanes(
      const geom::Location &origin,
      const geom::Location &destination) const {
    return element::LaneCrossingCalculator::Calculate(*this, origin, destination);
  }

  // ===========================================================================
  // -- Map: Waypoint generation -----------------------------------------------
  // ===========================================================================

  bool Map::IsValid(Waypoint waypoint) const {
    /// @todo Check if it's inside the road length.
    return GetLane(waypoint) != nullptr;
  }

  std::vector<Waypoint> Map::GetSuccessors(const Waypoint waypoint) const {
    auto *lane = GetLane(waypoint);
    THROW_INVALID_INPUT_ASSERT(lane != nullptr);

    const auto &next_lanes =
        lane->GetId() <= 0 ?
            lane->GetNextLanes() :
            lane->GetPreviousLanes();

    std::vector<Waypoint> result;
    result.reserve(next_lanes.size());
    for (auto *next_lane : next_lanes) {
      THROW_INVALID_INPUT_ASSERT(next_lane != nullptr);
      const auto lane_id = next_lane->GetId();
      THROW_INVALID_INPUT_ASSERT(lane_id != 0);
      const auto *road = next_lane->GetRoad();
      THROW_INVALID_INPUT_ASSERT(road != nullptr);
      const auto distance = lane_id < 0 ? 0.0f : road->GetLength();
      result.emplace_back(Waypoint{road->GetId(), lane_id, distance});
    }
    return result;
  }

  std::vector<Waypoint> Map::GetNext(
      const Waypoint waypoint,
      const float distance) const {
    THROW_INVALID_INPUT_ASSERT(waypoint.lane_id != 0);

    float distance_on_next_segment;

    if (waypoint.lane_id <= 0) {
      // road goes forward.
      const auto total_distance = waypoint.s + distance;
      const auto *road = _data.GetRoad(waypoint.road_id);
      THROW_INVALID_INPUT_ASSERT(road != nullptr);
      const auto road_length = road->GetLength();
      if (total_distance <= road_length) {
        return { Waypoint{waypoint.road_id, waypoint.lane_id, total_distance} };
      }
      distance_on_next_segment = total_distance - road_length;
    } else {
      // road goes backward.
      const auto total_distance = waypoint.s - distance;
      if (total_distance >= 0.0f) {
        return { Waypoint{waypoint.road_id, waypoint.lane_id, total_distance} };
      }
      distance_on_next_segment = std::abs(total_distance);
    }

    std::vector<Waypoint> result;
    for (const auto &next_waypoint : GetSuccessors(waypoint)) {
      result = ConcatVectors(result, GetNext(next_waypoint, distance_on_next_segment));
    }
    return result;
  }

  boost::optional<Waypoint> Map::GetRight(Waypoint waypoint) const {
    THROW_INVALID_INPUT_ASSERT(waypoint.lane_id != 0);
    if (waypoint.lane_id > 0) {
      ++waypoint.lane_id;
    } else {
      --waypoint.lane_id;
    }
    return GetLane(waypoint) != nullptr ? waypoint : boost::optional<Waypoint>{};
  }

  boost::optional<Waypoint> Map::GetLeft(Waypoint waypoint) const {
    THROW_INVALID_INPUT_ASSERT(waypoint.lane_id != 0);
    if (std::abs(waypoint.lane_id) == 1) {
      waypoint.lane_id *= -1;
    } else if (waypoint.lane_id > 0) {
      --waypoint.lane_id;
    } else {
      ++waypoint.lane_id;
    }
    return GetLane(waypoint) != nullptr ? waypoint : boost::optional<Waypoint>{};
  }

  std::vector<Waypoint> Map::GenerateWaypoints(const float distance) const {
    std::vector<Waypoint> result;
    for (const auto &pair : _data.GetRoads()) {
      const auto &road = pair.second;
      for (float s = 0.0f; s < road.GetLength(); s += distance) {
        ForEachDrivableLaneAt(road, s, [&](auto &&waypoint) {
          result.emplace_back(waypoint);
        });
      }
    }
    return result;
  }

  // std::vector<Waypoint> Map::GenerateLaneBegin() const {
  //   // std::vector<Waypoint> result;
  //   // for (auto &&road_segment : map.GetData().GetRoadSegments()) {
  //   //   ForEachDrivableLane(road_segment, 0.0, [&](auto lane_id) {
  //   //     auto distance = lane_id < 0 ? 0.0 : road_segment.GetLength();
  //   //     auto this_waypoint = Waypoint(
  //   //         map.shared_from_this(),
  //   //         road_segment.GetId(),
  //   //         lane_id,
  //   //         distance);
  //   //     result.push_back(this_waypoint);
  //   //   });
  //   // }
  //   // return result;
  //   throw_exception(std::runtime_error("not implemented"));
  //   return {};
  // }

  // std::vector<Waypoint> Map::GenerateLaneEnd() const {
  //   // std::vector<Waypoint> result;
  //   // for (auto &&road_segment : map.GetData().GetRoadSegments()) {
  //   //   ForEachDrivableLane(road_segment, 0.0, [&](auto lane_id) {
  //   //     auto distance = lane_id > 0 ? 0.0 : road_segment.GetLength();
  //   //     auto this_waypoint = Waypoint(
  //   //         map.shared_from_this(),
  //   //         road_segment.GetId(),
  //   //         lane_id,
  //   //         distance);
  //   //     result.push_back(this_waypoint);
  //   //   });
  //   // }
  //   // return result;
  //   throw_exception(std::runtime_error("not implemented"));
  //   return {};
  // }

  std::vector<std::pair<Waypoint, Waypoint>> Map::GenerateTopology() const {
    std::vector<std::pair<Waypoint, Waypoint>> result;
    for (const auto &pair : _data.GetRoads()) {
      const auto &road = pair.second;
      ForEachDrivableLane(road, [&](auto &&waypoint) {
        for (auto &&successor : GetSuccessors(waypoint)) {
          result.push_back({waypoint, successor});
        }
      });
    }
    return result;
  }

  // ===========================================================================
  // -- Map: Private functions -------------------------------------------------
  // ===========================================================================

  const Lane *Map::GetLane(Waypoint waypoint) const {
    return _data.GetLane(waypoint.road_id, waypoint.lane_id, waypoint.s);
  }

} // namespace road
} // namespace carla

#undef THROW_INVALID_INPUT_ASSERT
