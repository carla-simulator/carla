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
#include "carla/road/element/RoadInfoMarkRecord.h"
#include "carla/road/element/RoadInfoLaneOffset.h"
#include "carla/geom/Math.h"

#include <stdexcept>

namespace carla {
namespace road {

  using namespace carla::road::element;

  /// We use this epsilon to shift the waypoints away from the edges of the lane
  /// sections to avoid floating point precision errors.
  static constexpr double EPSILON = 10.0 * std::numeric_limits<double>::epsilon();

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

  static double GetDistanceAtStartOfLane(const Lane &lane) {
    if (lane.GetId() <= 0) {
      return lane.GetDistance() + 10.0 * EPSILON;
    } else {
      return lane.GetDistance() + lane.GetLength() - 10.0 * EPSILON;
    }
  }

  static double GetDistanceAtEndOfLane(const Lane &lane) {
    if (lane.GetId() > 0) {
      return lane.GetDistance() + 10.0 * EPSILON;
    } else {
      return lane.GetDistance() + lane.GetLength() - 10.0 * EPSILON;
    }
  }

  /// Return a waypoint for each drivable lane on @a lane_section.
  template <typename FuncT>
  static void ForEachDrivableLaneImpl(
      RoadId road_id,
      const LaneSection &lane_section,
      double distance,
      FuncT &&func) {
    for (const auto &pair : lane_section.GetLanes()) {
      const auto &lane = pair.second;
      if ((static_cast<uint32_t>(lane.GetType()) & static_cast<uint32_t>(Lane::LaneType::Driving)) > 0) {
        std::forward<FuncT>(func)(Waypoint{
            road_id,
            lane_section.GetId(),
            lane.GetId(),
            distance < 0.0 ? GetDistanceAtStartOfLane(lane) : distance});
      }
    }
  }

  /// Return a waypoint for each drivable lane on each lane section of @a road.
  template <typename FuncT>
  static void ForEachDrivableLane(const Road &road, FuncT &&func) {
    for (const auto &lane_section : road.GetLaneSections()) {
      ForEachDrivableLaneImpl(
          road.GetId(),
          lane_section,
          -1.0,
          std::forward<FuncT>(func));
    }
  }

  /// Return a waypoint for each drivable lane at @a distance on @a road.
  template <typename FuncT>
  static void ForEachDrivableLaneAt(const Road &road, double distance, FuncT &&func) {
    for (const auto &lane_section : road.GetLaneSectionsAt(distance)) {
      ForEachDrivableLaneImpl(
          road.GetId(),
          lane_section,
          distance,
          std::forward<FuncT>(func));
    }
  }

  /// Returns a pair containing first = width, second = tangent,
  /// for an specific Lane given an s and a iterator over lanes
  template <typename T>
  static std::pair<double, double> ComputeTotalLaneWidth(
      const T container,
      const double s,
      const LaneId lane_id) {
    const bool negative_lane_id = lane_id < 0;
    double dist = 0.0;
    double tangent = 0.0;
    for (const auto &lane : container) {
      auto info = lane.second.template GetInfo<RoadInfoLaneWidth>(s);
      RELEASE_ASSERT(info != nullptr);
      const auto current_polynomial = info->GetPolynomial();
      auto current_dist = current_polynomial.Evaluate(s);
      auto current_tang = current_polynomial.Tangent(s);
      if (lane.first != lane_id) {
        dist += negative_lane_id ? current_dist : -current_dist;
        tangent += current_tang;
      } else if (lane.first == lane_id) {
        current_dist *= 0.5;
        dist += negative_lane_id ? current_dist : -current_dist;
        tangent += current_tang * 0.5;
        break;
      }
    }
    return std::make_pair(dist, tangent);
  }

  /// Assumes road_id and section_id are valid.
  static bool IsLanePresent(const MapData &data, Waypoint waypoint) {
    const auto &section = data.GetRoad(waypoint.road_id).GetLaneSectionById(waypoint.section_id);
    return section.ContainsLane(waypoint.lane_id);
  }

  // ===========================================================================
  // -- Map: Geometry ----------------------------------------------------------
  // ===========================================================================

  boost::optional<Waypoint> Map::GetClosestWaypointOnRoad(
      const geom::Location &pos,
      uint32_t lane_type) const {
    // max_nearests represents the max nearests roads
    // where we will search for nearests lanes
    constexpr size_t max_nearests = 50u;
    // in case that map has less than max_nearests lanes,
    // we will use the maximum lanes
    const size_t max_nearest_allowed = std::min(_data.GetRoadCount(), max_nearests);

    // Unreal's Y axis hack
    const auto pos_inverted_y = geom::Location(pos.x, -pos.y, pos.z);

    double nearest_dist[max_nearests];
    std::fill(nearest_dist, nearest_dist + max_nearest_allowed,
        std::numeric_limits<double>::max());

    RoadId ids[max_nearests];
    std::fill(ids, ids + max_nearest_allowed, 0);

    double dists[max_nearests];
    std::fill(dists, dists + max_nearest_allowed, 0.0);

    for (const auto &road_pair : _data.GetRoads()) {
      const auto road = &road_pair.second;
      const auto current_dist = road->GetNearestPoint(pos_inverted_y);

      for (size_t i = 0u; i < max_nearest_allowed; ++i) {
        if (current_dist.second < nearest_dist[i]) {
          // reorder nearest_dist
          for (size_t j = max_nearest_allowed - 1u; j > i; --j) {
            DEBUG_ASSERT(j > 0u);
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
    auto nearest_lane_dist = std::numeric_limits<double>::max();
    for (size_t i = 0u; i < max_nearest_allowed; ++i) {
      auto lane_dist = _data.GetRoad(ids[i]).GetNearestLane(dists[i], pos_inverted_y, lane_type);

      if (lane_dist.second < nearest_lane_dist) {
        nearest_lane_dist = lane_dist.second;
        waypoint.lane_id = lane_dist.first->GetId();
        waypoint.road_id = ids[i];
        waypoint.s = dists[i];
      }
    }

    if (nearest_lane_dist == std::numeric_limits<double>::max()) {
      return boost::optional<Waypoint>{};
    }

    const auto &road = _data.GetRoad(waypoint.road_id);

    // Make sure 0.0 < waipoint.s < Road's length
    constexpr double margin = 5.0 * EPSILON;
    DEBUG_ASSERT(margin < road.GetLength() - margin);
    waypoint.s = geom::Math::Clamp(waypoint.s, margin, road.GetLength() - margin);

    auto &lane = road.GetLaneByDistance(waypoint.s, waypoint.lane_id);

    const auto lane_section = lane.GetLaneSection();
    RELEASE_ASSERT(lane_section != nullptr);
    const auto lane_section_id = lane_section->GetId();
    waypoint.section_id = lane_section_id;

    return waypoint;
  }

  boost::optional<Waypoint> Map::GetWaypoint(
      const geom::Location &pos,
      uint32_t lane_type) const {
    boost::optional<Waypoint> w = GetClosestWaypointOnRoad(pos, lane_type);

    if (!w.has_value()) {
      return w;
    }

    const auto dist = geom::Math::Distance2D(ComputeTransform(*w).location, pos);
    const auto lane_width_info = GetLane(*w).GetInfo<RoadInfoLaneWidth>(w->s);
    const auto half_lane_width =
        lane_width_info->GetPolynomial().Evaluate(w->s) * 0.5;

    if (dist < half_lane_width) {
      return w;
    }

    return boost::optional<Waypoint>{};
  }

  geom::Transform Map::ComputeTransform(Waypoint waypoint) const {
    // lane_id can't be 0
    RELEASE_ASSERT(waypoint.lane_id != 0);

    const auto &road = _data.GetRoad(waypoint.road_id);

    // must s be smaller (or eq) than road lenght and bigger (or eq) than 0?
    RELEASE_ASSERT(waypoint.s <= road.GetLength());
    RELEASE_ASSERT(waypoint.s >= 0.0);

    const auto &lane_section = road.GetLaneSectionById(waypoint.section_id);
    const std::map<LaneId, Lane> &lanes = lane_section.GetLanes();

    // check that lane_id exists on the current s
    RELEASE_ASSERT(!lanes.empty());
    RELEASE_ASSERT(waypoint.lane_id >= lanes.begin()->first);
    RELEASE_ASSERT(waypoint.lane_id <= lanes.rbegin()->first);

    float lane_width = 0.0f;
    float lane_tangent = 0.0f;
    if (waypoint.lane_id < 0) {
      // right lane
      const auto side_lanes = MakeListView(
          std::make_reverse_iterator(lanes.lower_bound(0)), lanes.rend());
      const auto computed_width =
          ComputeTotalLaneWidth(side_lanes, waypoint.s, waypoint.lane_id);
      lane_width = static_cast<float>(computed_width.first);
      lane_tangent = static_cast<float>(computed_width.second);
    } else {
      // left lane
      const auto side_lanes = MakeListView(lanes.lower_bound(1), lanes.end());
      const auto computed_width =
          ComputeTotalLaneWidth(side_lanes, waypoint.s, waypoint.lane_id);
      lane_width = static_cast<float>(computed_width.first);
      lane_tangent = static_cast<float>(computed_width.second);
    }

    // get a directed point in s and apply the computed lateral offet
    DirectedPoint dp = road.GetDirectedPointIn(waypoint.s);

    // compute the tangent of the laneOffset
    const auto lane_offset_info = road.GetInfo<RoadInfoLaneOffset>(waypoint.s);
    const auto lane_offset_tangent = static_cast<float>(lane_offset_info->GetPolynomial().Tangent(waypoint.s));

    lane_tangent -= lane_offset_tangent;

    // Unreal's Y axis hack
    lane_tangent *= -1;

    geom::Rotation rot(
        geom::Math::ToDegrees(static_cast<float>(dp.pitch)),
        geom::Math::ToDegrees(-static_cast<float>(dp.tangent)), // Unreal's Y axis hack
        0.0f);

    dp.ApplyLateralOffset(lane_width);

    if (waypoint.lane_id > 0) {
      rot.yaw += 180.0f + geom::Math::ToDegrees(lane_tangent);
      rot.pitch = 360.0f - rot.pitch;
    } else {
      rot.yaw -= geom::Math::ToDegrees(lane_tangent);
    }

    // Unreal's Y axis hack
    dp.location.y *= -1;

    return geom::Transform(dp.location, rot);
  }

  // ===========================================================================
  // -- Map: Road information --------------------------------------------------
  // ===========================================================================

  Lane::LaneType Map::GetLaneType(const Waypoint waypoint) const {
    return GetLane(waypoint).GetType();
  }

  double Map::GetLaneWidth(const Waypoint waypoint) const {
    const auto s = waypoint.s;

    const auto &lane = GetLane(waypoint);
    RELEASE_ASSERT(lane.GetRoad() != nullptr);
    RELEASE_ASSERT(s <= lane.GetRoad()->GetLength());

    const auto lane_width_info = lane.GetInfo<RoadInfoLaneWidth>(s);
    RELEASE_ASSERT(lane_width_info != nullptr);

    return lane_width_info->GetPolynomial().Evaluate(s);
  }

  JuncId Map::GetJunctionId(RoadId road_id) const {
    return _data.GetRoad(road_id).GetJunctionId();
  }

  bool Map::IsJunction(RoadId road_id) const {
    return _data.GetRoad(road_id).IsJunction();
  }

  std::pair<const RoadInfoMarkRecord *, const RoadInfoMarkRecord *>
  Map::GetMarkRecord(const Waypoint waypoint) const {
    const auto s = waypoint.s;

    const auto &current_lane = GetLane(waypoint);
    RELEASE_ASSERT(current_lane.GetRoad() != nullptr);
    RELEASE_ASSERT(s <= current_lane.GetRoad()->GetLength());

    const auto inner_lane_id = waypoint.lane_id < 0 ?
        waypoint.lane_id + 1 :
        waypoint.lane_id - 1;

    const auto &inner_lane = current_lane.GetRoad()->GetLaneById(waypoint.section_id, inner_lane_id);

    auto current_lane_info = current_lane.GetInfo<RoadInfoMarkRecord>(s);
    auto inner_lane_info = inner_lane.GetInfo<RoadInfoMarkRecord>(s);

    return std::make_pair(current_lane_info, inner_lane_info);
  }

  std::vector<LaneMarking> Map::CalculateCrossedLanes(
      const geom::Location &origin,
      const geom::Location &destination) const {
    return LaneCrossingCalculator::Calculate(*this, origin, destination);
  }

  // ===========================================================================
  // -- Map: Waypoint generation -----------------------------------------------
  // ===========================================================================

  std::vector<Waypoint> Map::GetSuccessors(const Waypoint waypoint) const {
    const auto &next_lanes = GetLane(waypoint).GetNextLanes();
    std::vector<Waypoint> result;
    result.reserve(next_lanes.size());
    for (auto *next_lane : next_lanes) {
      RELEASE_ASSERT(next_lane != nullptr);
      const auto lane_id = next_lane->GetId();
      RELEASE_ASSERT(lane_id != 0);
      const auto *section = next_lane->GetLaneSection();
      RELEASE_ASSERT(section != nullptr);
      const auto *road = next_lane->GetRoad();
      RELEASE_ASSERT(road != nullptr);
      const auto distance = GetDistanceAtStartOfLane(*next_lane);
      result.emplace_back(Waypoint{road->GetId(), section->GetId(), lane_id, distance});
    }
    return result;
  }

  std::vector<Waypoint> Map::GetPredecessors(const Waypoint waypoint) const {
    const auto &prev_lanes = GetLane(waypoint).GetPreviousLanes();
    std::vector<Waypoint> result;
    result.reserve(prev_lanes.size());
    for (auto *next_lane : prev_lanes) {
      RELEASE_ASSERT(next_lane != nullptr);
      const auto lane_id = next_lane->GetId();
      RELEASE_ASSERT(lane_id != 0);
      const auto *section = next_lane->GetLaneSection();
      RELEASE_ASSERT(section != nullptr);
      const auto *road = next_lane->GetRoad();
      RELEASE_ASSERT(road != nullptr);
      const auto distance = GetDistanceAtEndOfLane(*next_lane);
      result.emplace_back(Waypoint{road->GetId(), section->GetId(), lane_id, distance});
    }
    return result;
  }

  std::vector<Waypoint> Map::GetNext(
      const Waypoint waypoint,
      const double distance) const {
    RELEASE_ASSERT(distance > 0.0);
    const auto &lane = GetLane(waypoint);
    const bool forward = (waypoint.lane_id <= 0);
    const double signed_distance = forward ? distance : -distance;
    const double relative_s = waypoint.s - lane.GetDistance() + EPSILON;
    const double remaining_lane_length = forward ? lane.GetLength() - relative_s : relative_s;
    DEBUG_ASSERT(remaining_lane_length >= 0.0);

    // If after subtracting the distance we are still in the same lane, return
    // same waypoint with the extra distance.
    if (distance <= remaining_lane_length) {
      Waypoint result = waypoint;
      result.s += signed_distance;
      result.s += forward ? -EPSILON : EPSILON;
      RELEASE_ASSERT(result.s > 0.0);
      return { result };
    }

    // If we run out of remaining_lane_length we have to go to the successors.
    std::vector<Waypoint> result;
    for (const auto &successor : GetSuccessors(waypoint)) {
      DEBUG_ASSERT(
          successor.road_id != waypoint.road_id ||
          successor.section_id != waypoint.section_id ||
          successor.lane_id != waypoint.lane_id);
      result = ConcatVectors(result, GetNext(successor, distance - remaining_lane_length));
    }
    return result;
  }

  boost::optional<Waypoint> Map::GetRight(Waypoint waypoint) const {
    RELEASE_ASSERT(waypoint.lane_id != 0);
    if (waypoint.lane_id > 0) {
      ++waypoint.lane_id;
    } else {
      --waypoint.lane_id;
    }
    return IsLanePresent(_data, waypoint) ? waypoint : boost::optional<Waypoint>{};
  }

  boost::optional<Waypoint> Map::GetLeft(Waypoint waypoint) const {
    RELEASE_ASSERT(waypoint.lane_id != 0);
    if (std::abs(waypoint.lane_id) == 1) {
      waypoint.lane_id *= -1;
    } else if (waypoint.lane_id > 0) {
      --waypoint.lane_id;
    } else {
      ++waypoint.lane_id;
    }
    return IsLanePresent(_data, waypoint) ? waypoint : boost::optional<Waypoint>{};
  }

  std::vector<Waypoint> Map::GenerateWaypoints(const double distance) const {
    RELEASE_ASSERT(distance > 0.0);
    std::vector<Waypoint> result;
    for (const auto &pair : _data.GetRoads()) {
      const auto &road = pair.second;
      for (double s = EPSILON; s < (road.GetLength() - EPSILON); s += distance) {
        ForEachDrivableLaneAt(road, s, [&](auto &&waypoint) {
          result.emplace_back(waypoint);
        });
      }
    }
    return result;
  }

  std::vector<Waypoint> Map::GenerateWaypointsOnRoadEntries() const {
    std::vector<Waypoint> result;
    for (const auto &pair : _data.GetRoads()) {
      const auto &road = pair.second;
      // right lanes start at s 0
      for (const auto &lane_section : road.GetLaneSectionsAt(0.0)) {
        for (const auto &lane : lane_section.GetLanes()) {
          // add only the right (negative) lanes
          if (lane.first < 0 && lane.second.GetType() == Lane::LaneType::Driving) {
            result.emplace_back(Waypoint{ road.GetId(), lane_section.GetId(), lane.second.GetId(), 0.0 });
          }
        }
      }
      // left lanes start at s max
      const auto road_len = road.GetLength();
      for (const auto &lane_section : road.GetLaneSectionsAt(road_len)) {
        for (const auto &lane : lane_section.GetLanes()) {
          // add only the left (positive) lanes
          if (lane.first > 0 && lane.second.GetType() == Lane::LaneType::Driving) {
            result.emplace_back(Waypoint{ road.GetId(), lane_section.GetId(), lane.second.GetId(), road_len });
          }
        }
      }
    }
    return result;
  }

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

  const Lane &Map::GetLane(Waypoint waypoint) const {
    return _data.GetRoad(waypoint.road_id).GetLaneById(waypoint.section_id, waypoint.lane_id);
  }

} // namespace road
} // namespace carla
