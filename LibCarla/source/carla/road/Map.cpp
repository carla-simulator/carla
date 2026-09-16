// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "carla/road/Map.h"
#include "carla/Exception.h"
#include "carla/geom/Math.h"
#include "carla/geom/Vector3D.h"
#include "carla/road/MeshFactory.h"
#include "carla/road/Deformation.h"
#include "carla/road/element/LaneCrossingCalculator.h"
#include "carla/road/element/RoadInfoCrosswalk.h"
#include "carla/road/element/RoadInfoElevation.h"
#include "carla/road/element/RoadInfoGeometry.h"
#include "carla/road/element/RoadInfoLaneOffset.h"
#include "carla/road/element/RoadInfoLaneWidth.h"
#include "carla/road/element/RoadInfoMarkRecord.h"
#include "carla/road/element/RoadInfoSpeed.h"
#include "carla/road/element/RoadInfoSignal.h"

#include <third-party/marchingcube/MeshReconstruction.h>

#include <algorithm>
#include <limits>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <stdexcept>
#include <chrono>
#include <thread>
#include <iomanip>
#include <cmath>

namespace carla {
namespace road {

  using namespace carla::road::element;

  /// We use this epsilon to shift the waypoints away from the edges of the lane
  /// sections to avoid floating point precision errors.
  static constexpr double EPSILON = 10.0 * std::numeric_limits<double>::epsilon();
  static constexpr double TREE_PLACEMENT_EPSILON = 1.0e-4;

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
    if (lane.IsPositiveDirection()) {
      return lane.GetDistance() + 10.0 * EPSILON;
    } else {
      return lane.GetDistance() + lane.GetLength() - 10.0 * EPSILON;
    }
  }

  static double GetDistanceAtEndOfLane(const Lane &lane) {
    if (!lane.IsPositiveDirection()) {
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
      if (lane.GetId() == 0) {
        continue;
      }
      if ((static_cast<uint32_t>(lane.GetType()) & static_cast<uint32_t>(Lane::LaneType::Driving)) > 0) {
        std::forward<FuncT>(func)(Waypoint{
            road_id,
            lane_section.GetId(),
            lane.GetId(),
            distance < 0.0 ? GetDistanceAtStartOfLane(lane) : distance});
      }
    }
  }

  template <typename FuncT>
  static void ForEachLaneImpl(
      RoadId road_id,
      const LaneSection &lane_section,
      double distance,
      Lane::LaneType lane_type,
      FuncT &&func) {
    for (const auto &pair : lane_section.GetLanes()) {
      const auto &lane = pair.second;
      if (lane.GetId() == 0) {
        continue;
      }
      if ((static_cast<int32_t>(lane.GetType()) & static_cast<int32_t>(lane_type)) > 0) {
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
          -1.0, // At start of the lane
          std::forward<FuncT>(func));
    }
  }

  /// Return a waypoint for each lane of the specified type on each lane section of @a road.
  template <typename FuncT>
  static void ForEachLane(const Road &road, Lane::LaneType lane_type, FuncT &&func) {
    for (const auto &lane_section : road.GetLaneSections()) {
      ForEachLaneImpl(
          road.GetId(),
          lane_section,
          -1.0, // At start of the lane
          lane_type,
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

  /// Assumes road_id and section_id are valid.
  static bool IsLanePresent(const MapData &data, Waypoint waypoint) {
    const auto &section = data.GetRoad(waypoint.road_id).GetLaneSectionById(waypoint.section_id);
    return section.ContainsLane(waypoint.lane_id);
  }

  // ===========================================================================
  // -- Map: Geometry ----------------------------------------------------------
  // ===========================================================================

  std::optional<Waypoint> Map::GetClosestWaypointOnRoad(
      const geom::Location &pos,
      int32_t lane_type) const {
    std::vector<Rtree::TreeElement> query_result =
        _rtree.GetNearestNeighboursWithFilter(Rtree::BPoint(pos.x, pos.y, pos.z),
        [&](Rtree::TreeElement const &element) {
          const Lane &lane = GetLane(element.second.first);
          return (lane_type & static_cast<int32_t>(lane.GetType())) > 0;
        });

    if (query_result.size() == 0) {
      return std::optional<Waypoint>{};
    }

    Rtree::BSegment segment = query_result.front().first;
    Rtree::BPoint s1 = segment.first;
    Rtree::BPoint s2 = segment.second;
    auto distance_to_segment = geom::Math::DistanceSegmentToPoint(pos,
        geom::Vector3D(s1.get<0>(), s1.get<1>(), s1.get<2>()),
        geom::Vector3D(s2.get<0>(), s2.get<1>(), s2.get<2>()));

    Waypoint result_start = query_result.front().second.first;
    Waypoint result_end = query_result.front().second.second;

    if (GetLane(result_start).IsPositiveDirection()) {
      double delta_s = distance_to_segment.first;
      double final_s = result_start.s + delta_s;
      if (final_s >= result_end.s) {
        return result_end;
      } else if (delta_s <= 0) {
        return result_start;
      } else {
        return GetNext(result_start, delta_s).front();
      }
    } else {
      double delta_s = distance_to_segment.first;
      double final_s = result_start.s - delta_s;
      if (final_s <= result_end.s) {
        return result_end;
      } else if (delta_s <= 0) {
        return result_start;
      } else {
        return GetNext(result_start, delta_s).front();
      }
    }
  }

  std::optional<Waypoint> Map::GetWaypoint(
      const geom::Location &pos,
      int32_t lane_type) const {
    std::optional<Waypoint> w = GetClosestWaypointOnRoad(pos, lane_type);

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

    return std::optional<Waypoint>{};
  }

  std::optional<Waypoint> Map::GetWaypoint(
      RoadId road_id,
      LaneId lane_id,
      float s) const {

    // define the waypoint with the known parameters
    Waypoint waypoint;
    waypoint.road_id = road_id;
    waypoint.lane_id = lane_id;
    waypoint.s = s;

    // check the road
    if (!_data.ContainsRoad(waypoint.road_id)) {
      return std::optional<Waypoint>{};
    }
    const Road &road = _data.GetRoad(waypoint.road_id);

    // check the 's' distance
    if (s < 0.0f || s >= road.GetLength()) {
      return std::optional<Waypoint>{};
    }

    // check the section
    bool lane_found = false;
    for (auto &section : road.GetLaneSectionsAt(s)) {
      if (section.ContainsLane(lane_id)) {
        waypoint.section_id = section.GetId();
        lane_found = true;
        break;
      }
    }

    // check the lane id
    if (!lane_found) {
      return std::optional<Waypoint>{};
    }

    return waypoint;
  }

  geom::Transform Map::ComputeTransform(Waypoint waypoint) const {
    return GetLane(waypoint).ComputeTransform(waypoint.s);
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
    // if lane Id is 0, just return a pair of nulls
    if (waypoint.lane_id == 0)
      return std::make_pair(nullptr, nullptr);

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

  std::vector<Map::SignalSearchData> Map::GetSignalsInDistance(
      Waypoint waypoint, double distance, bool stop_at_junction) const {

    const auto &lane = GetLane(waypoint);
    const bool forward = lane.IsPositiveDirection();
    const double signed_distance = forward ? distance : -distance;
    const double relative_s = waypoint.s - lane.GetDistance();
    const double remaining_lane_length = forward ? lane.GetLength() - relative_s : relative_s;
    DEBUG_ASSERT(remaining_lane_length >= 0.0);

    auto &road =_data.GetRoad(waypoint.road_id);
    std::vector<SignalSearchData> result;

    // If after subtracting the distance we are still in the same lane, return
    // same waypoint with the extra distance.
    if (distance <= remaining_lane_length) {
      auto signals = road.GetInfosInRange<RoadInfoSignal>(
          waypoint.s, waypoint.s + signed_distance);
      for(auto* signal : signals){
        double distance_to_signal = 0;
        if (lane.IsPositiveDirection()){
          distance_to_signal = signal->GetDistance() - waypoint.s;
        } else {
          distance_to_signal = waypoint.s - signal->GetDistance();
        }
        // check that the signal affects the waypoint
        bool is_valid = false;
        for (auto &validity : signal->GetValidities()) {
          if (waypoint.lane_id >= validity._from_lane &&
              waypoint.lane_id <= validity._to_lane) {
            is_valid = true;
            break;
          }
        }
        if(!is_valid){
          continue;
        }
        if (distance_to_signal == 0) {
          result.emplace_back(SignalSearchData
              {signal, waypoint,
              distance_to_signal});
        } else {
          result.emplace_back(SignalSearchData
              {signal, GetNext(waypoint, distance_to_signal).front(),
              distance_to_signal});
        }

      }
      return result;
    }
    const double signed_remaining_length = forward ? remaining_lane_length : -remaining_lane_length;

    //result = road.GetInfosInRange<RoadInfoSignal>(waypoint.s, waypoint.s + signed_remaining_length);
    auto signals = road.GetInfosInRange<RoadInfoSignal>(
        waypoint.s, waypoint.s + signed_remaining_length);
    for(auto* signal : signals){
      double distance_to_signal = 0;
      if (lane.IsPositiveDirection()){
        distance_to_signal = signal->GetDistance() - waypoint.s;
      } else {
        distance_to_signal = waypoint.s - signal->GetDistance();
      }
      // check that the signal affects the waypoint
      bool is_valid = false;
      for (auto &validity : signal->GetValidities()) {
        if (waypoint.lane_id >= validity._from_lane &&
            waypoint.lane_id <= validity._to_lane) {
          is_valid = true;
          break;
        }
      }
      if(!is_valid){
        continue;
      }
      if (distance_to_signal == 0) {
        result.emplace_back(SignalSearchData
            {signal, waypoint,
            distance_to_signal});
      } else {
        result.emplace_back(SignalSearchData
            {signal, GetNext(waypoint, distance_to_signal).front(),
            distance_to_signal});
      }
    }
    // If we run out of remaining_lane_length we have to go to the successors.
    for (auto &successor : GetSuccessors(waypoint)) {
      if(_data.GetRoad(successor.road_id).IsJunction() && stop_at_junction){
        continue;
      }
      auto& sucessor_lane = _data.GetRoad(successor.road_id).
            GetLaneByDistance(successor.s, successor.lane_id);

      if (GetLane(successor).IsPositiveDirection()) {
        successor.s = sucessor_lane.GetDistance();
      } else {
        successor.s = sucessor_lane.GetDistance() + sucessor_lane.GetLength();
      }
      auto sucessor_signals = GetSignalsInDistance(
          successor, distance - remaining_lane_length, stop_at_junction);
      for(auto& signal : sucessor_signals){
        signal.accumulated_s += remaining_lane_length;
      }
      result = ConcatVectors(result, sucessor_signals);
    }
    return result;
  }

  std::vector<const element::RoadInfoSignal*>
      Map::GetAllSignalReferences() const {
    std::vector<const element::RoadInfoSignal*> result;
    for (const auto& road_pair : _data.GetRoads()) {
      const auto &road = road_pair.second;
      auto road_infos = road.GetInfos<element::RoadInfoSignal>();
      for(const auto* road_info : road_infos) {
        result.push_back(road_info);
      }
    }
    return result;
  }

  std::vector<LaneMarking> Map::CalculateCrossedLanes(
      const geom::Location &origin,
      const geom::Location &destination) const {
    return LaneCrossingCalculator::Calculate(*this, origin, destination);
  }

  std::vector<geom::Location> Map::GetAllCrosswalkZones() const {
    std::vector<geom::Location> result;

    for (const auto &pair : _data.GetRoads()) {
      const auto &road = pair.second;
      std::vector<const RoadInfoCrosswalk *> crosswalks = road.GetInfos<RoadInfoCrosswalk>();
      if (crosswalks.size() > 0) {
        for (auto crosswalk : crosswalks) {
          // waypoint only at start position
          std::vector<geom::Location> points;
          Waypoint waypoint;
          geom::Transform base;
          for (const auto &section : road.GetLaneSectionsAt(crosswalk->GetS())) {
            // get the section with the center lane
            for (const auto &lane : section.GetLanes()) {
              // is the center line
              if (lane.first == 0) {
                // get the center point
                waypoint.road_id = pair.first;
                waypoint.section_id = section.GetId();
                waypoint.lane_id = 0;
                waypoint.s = crosswalk->GetS();
                base = ComputeTransform(waypoint);
              }
            }
          }

          // The outline corners are given at height 0 above the road surface.
          // Placing them with the full centre-line transform at the crosswalk's
          // s applies that one point's pitch and roll to the whole outline, which
          // on a graded road tilts the slab about the pivot: the corners ahead of
          // it stand 10-60 cm proud of the asphalt and the ones behind it sink.
          // Rotate the outline in the horizontal plane only and give every corner
          // the road's own elevation at that corner's s.
          geom::Transform pivot = base;
          pivot.rotation.pitch = 0.0f;
          pivot.rotation.roll = 0.0f;
          const float heading_deg =
              geom::Math::ToDegrees<float>(static_cast<float>(crosswalk->GetHeading()));

          // move perpendicular ('t')
          pivot.rotation.yaw -= heading_deg;
          pivot.rotation.yaw -= 90;   // move perpendicular to 's' for the lateral offset
          geom::Vector3D v(static_cast<float>(crosswalk->GetT()), 0.0f, 0.0f);
          pivot.TransformPoint(v);
          // restore pivot position and orientation (yaw only)
          pivot.location = v;
          pivot.rotation.yaw += 90;

          // s direction of the road at the crosswalk, in the horizontal plane
          const geom::Vector3D forward =
              geom::Rotation(0.0f, base.rotation.yaw, 0.0f).GetForwardVector();
          const double s0 = crosswalk->GetS();
          const double road_length = road.GetLength();
          const double z0 = road.GetElevationOn(s0).Evaluate(s0);

          // calculate all the corners
          for (auto corner : crosswalk->GetPoints()) {
            geom::Vector3D v2(
                static_cast<float>(corner.u),
                static_cast<float>(corner.v),
                static_cast<float>(corner.z));
            // set the width larger to contact with the sidewalk (in case they have gutter area)
            if (corner.u < 0) {
              v2.x -= 1.0f;
            } else {
              v2.x += 1.0f;
            }
            pivot.TransformPoint(v2);
            // elevation of the road at this corner's own s (not the pivot's)
            const double ds =
                static_cast<double>((v2.x - base.location.x) * forward.x +
                                    (v2.y - base.location.y) * forward.y);
            const double s_corner = geom::Math::Clamp(s0 + ds, 0.0, road_length);
            const double z_corner = road.GetElevationOn(s_corner).Evaluate(s_corner);
            v2.z = base.location.z + static_cast<float>(z_corner - z0) +
                   static_cast<float>(corner.z);
            result.push_back(v2);
          }
        }
      }
    }
    return result;
  }

  // ===========================================================================
  // -- Map: Waypoint generation -----------------------------------------------
  // ===========================================================================

  std::vector<Waypoint> Map::GetSuccessors(const Waypoint waypoint) const {
    const auto &next_lanes = GetLane(waypoint).GetNextLanes();
    std::vector<Waypoint> result;
    result.reserve(next_lanes.size());
    for (auto *next_lane : next_lanes) {
      if (next_lane == nullptr) {
        continue;
      }
      const auto lane_id = next_lane->GetId();
      if (lane_id == 0) {
        continue;
      }
      const auto *section = next_lane->GetLaneSection();
      if (section == nullptr) {
        continue;
      }
      const auto *road = next_lane->GetRoad();
      if (road == nullptr) {
        continue;
      }
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
      if (next_lane == nullptr) {
        continue;
      }
      const auto lane_id = next_lane->GetId();
      if (lane_id == 0) {
        continue;
      }
      const auto *section = next_lane->GetLaneSection();
      if (section == nullptr) {
        continue;
      }
      const auto *road = next_lane->GetRoad();
      if (road == nullptr) {
        continue;
      }
      const auto distance = GetDistanceAtEndOfLane(*next_lane);
      result.emplace_back(Waypoint{road->GetId(), section->GetId(), lane_id, distance});
    }
    return result;
  }

  std::vector<Waypoint> Map::GetNext(
      const Waypoint waypoint,
      const double distance) const {
    RELEASE_ASSERT(distance > 0.0);
    if (distance <= EPSILON) {
      return {waypoint};
    }
    const auto &lane = GetLane(waypoint);
    const bool forward = lane.IsPositiveDirection(); 
    const double signed_distance = forward ? distance : -distance;
    const double relative_s = waypoint.s - lane.GetDistance();
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
      // Fix situations where the next waypoint is in the opposite direction and
      // this waypoint is its successor, so this function would end up in a loop
      bool is_broken = false;
      for (const auto &future_successor : GetSuccessors(successor)) {
          if (future_successor.road_id == waypoint.road_id
               && future_successor.lane_id == waypoint.lane_id
               && future_successor.section_id == waypoint.section_id){
            is_broken = true;
            break;
          }
      }
      if (!is_broken){
        result = ConcatVectors(result, GetNext(successor, distance - remaining_lane_length));
      }
    }
    return result;
  }

  std::vector<Waypoint> Map::GetPrevious(
      const Waypoint waypoint,
      const double distance) const {
    RELEASE_ASSERT(distance > 0.0);
    if (distance <= EPSILON) {
      return {waypoint};
    }
    const auto &lane = GetLane(waypoint);
    const bool forward = !lane.IsPositiveDirection();
    const double signed_distance = forward ? distance : -distance;
    const double relative_s = waypoint.s - lane.GetDistance();
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
    for (const auto &successor : GetPredecessors(waypoint)) {
      DEBUG_ASSERT(
          successor.road_id != waypoint.road_id ||
          successor.section_id != waypoint.section_id ||
          successor.lane_id != waypoint.lane_id);
      // Fix situations, when next waypoint is in the opposite direction and
      // this waypoint is his predecessor, so this function would end up in a loop
      bool is_broken = false;
      for (const auto &future_predecessor : GetPredecessors(successor)) {
          if (future_predecessor.road_id == waypoint.road_id
               && future_predecessor.lane_id == waypoint.lane_id
               && future_predecessor.section_id == waypoint.section_id){
            is_broken = true;
            break;
          }
      }
      if (!is_broken){
        result = ConcatVectors(result, GetPrevious(successor, distance - remaining_lane_length));
      }
    }
    return result;
  }

  std::optional<Waypoint> Map::GetRight(Waypoint waypoint) const {
    RELEASE_ASSERT(waypoint.lane_id != 0);
    bool is_rht = GetLane(waypoint).GetRoad()->IsRHT();
    if (is_rht){
      if (waypoint.lane_id > 0) {
        ++waypoint.lane_id;
      } else {
        --waypoint.lane_id;
      }
      return IsLanePresent(_data, waypoint) ? waypoint : std::optional<Waypoint>{};
    } else {
      if (std::abs(waypoint.lane_id) == 1) {
        waypoint.lane_id *= -1;
      } else if (waypoint.lane_id > 0) {
        --waypoint.lane_id;
      } else {
        ++waypoint.lane_id;
      }
      return IsLanePresent(_data, waypoint) ? waypoint : std::optional<Waypoint>{};
    }
  }

  std::optional<Waypoint> Map::GetLeft(Waypoint waypoint) const {
    RELEASE_ASSERT(waypoint.lane_id != 0);
    bool is_rht = GetLane(waypoint).GetRoad()->IsRHT();
    if (is_rht){
      if (std::abs(waypoint.lane_id) == 1) {
        waypoint.lane_id *= -1;
      } else if (waypoint.lane_id > 0) {
        --waypoint.lane_id;
      } else {
        ++waypoint.lane_id;
      }
      return IsLanePresent(_data, waypoint) ? waypoint : std::optional<Waypoint>{};
    } else {
      if (waypoint.lane_id > 0) {
        ++waypoint.lane_id;
      } else {
        --waypoint.lane_id;
      }
      return IsLanePresent(_data, waypoint) ? waypoint : std::optional<Waypoint>{};
    }
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

  std::vector<Waypoint> Map::GenerateWaypointsOnRoadEntries(Lane::LaneType lane_type) const {
    std::vector<Waypoint> result;
    for (const auto &pair : _data.GetRoads()) {
      const auto &road = pair.second;
      // right lanes start at s 0
      for (const auto &lane_section : road.GetLaneSectionsAt(0.0)) {
        for (const auto &lane : lane_section.GetLanes()) {
          if (lane.second.IsPositiveDirection() &&
              static_cast<int32_t>(lane.second.GetType()) & static_cast<int32_t>(lane_type)) {
            result.emplace_back(Waypoint{ road.GetId(), lane_section.GetId(), lane.second.GetId(), 0.0 });
          }
        }
      }
      // left lanes start at s max
      const auto road_len = road.GetLength();
      for (const auto &lane_section : road.GetLaneSectionsAt(road_len)) {
        for (const auto &lane : lane_section.GetLanes()) {
          // LHT reversed. add the right (negative) lanes
          if (!lane.second.IsPositiveDirection() &&
              static_cast<int32_t>(lane.second.GetType()) & static_cast<int32_t>(lane_type)) {
            result.emplace_back(
              Waypoint{ road.GetId(), lane_section.GetId(), lane.second.GetId(), road_len });
          }
        }
      }
    }
    return result;
  }

  std::vector<Waypoint> Map::GenerateWaypointsInRoad(
      RoadId road_id,
      Lane::LaneType lane_type) const {
    std::vector<Waypoint> result;
    if(_data.GetRoads().count(road_id)) {
      const auto &road = _data.GetRoads().at(road_id);
      // right lanes start at s 0
      for (const auto &lane_section : road.GetLaneSectionsAt(0.0)) {
        for (const auto &lane : lane_section.GetLanes()) {
          if (lane.second.IsPositiveDirection() &&
              static_cast<int32_t>(lane.second.GetType()) & static_cast<int32_t>(lane_type)) {
            result.emplace_back(Waypoint{ road.GetId(), lane_section.GetId(), lane.second.GetId(), 0.0 });
          }
        }
      }
      // left lanes start at s max
      const auto road_len = road.GetLength();
      for (const auto &lane_section : road.GetLaneSectionsAt(road_len)) {
        for (const auto &lane : lane_section.GetLanes()) {
          if (!lane.second.IsPositiveDirection() &&
              static_cast<int32_t>(lane.second.GetType()) & static_cast<int32_t>(lane_type)) {
            result.emplace_back(
              Waypoint{ road.GetId(), lane_section.GetId(), lane.second.GetId(), road_len });
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
        auto successors = GetSuccessors(waypoint);
        if (successors.size() == 0){
          auto distance = static_cast<float>(GetDistanceAtEndOfLane(GetLane(waypoint)));
          auto last_waypoint = GetWaypoint(waypoint.road_id, waypoint.lane_id, distance);
          if (last_waypoint.has_value()){
            result.push_back({waypoint, *last_waypoint});
          }
        }
        else{
          for (auto &&successor : GetSuccessors(waypoint)) {
            result.push_back({waypoint, successor});
          }
        }
      });
    }
    return result;
  }

  std::vector<std::pair<Waypoint, Waypoint>> Map::GetJunctionWaypoints(JuncId id, Lane::LaneType lane_type) const {
    std::vector<std::pair<Waypoint, Waypoint>> result;
    const Junction * junction = GetJunction(id);
    for(auto &connections : junction->GetConnections()) {
      const Road &road = _data.GetRoad(connections.second.connecting_road);
      ForEachLane(road, lane_type, [&](auto &&waypoint) {
        const auto& lane = GetLane(waypoint);
        const double final_s = GetDistanceAtEndOfLane(lane);
        Waypoint lane_end(waypoint);
        lane_end.s = final_s;
        result.push_back({waypoint, lane_end});
      });
    }
    return result;
  }

  std::unordered_map<road::RoadId, std::unordered_set<road::RoadId>>
      Map::ComputeJunctionConflicts(JuncId id) const {

    const float epsilon = 0.0001f; // small delta in the road (set to 0.1
                                     // millimeters to prevent numeric errors)
    const Junction *junction = GetJunction(id);
    std::unordered_map<road::RoadId, std::unordered_set<road::RoadId>>
        conflicts;

    // 2d typedefs
    typedef boost::geometry::model::point
        <float, 2, boost::geometry::cs::cartesian> Point2d;
    typedef boost::geometry::model::segment<Point2d> Segment2d;
    typedef boost::geometry::model::box<Rtree::BPoint> Box;

    // box range
    auto bbox_pos = junction->GetBoundingBox().location;
    auto bbox_ext = junction->GetBoundingBox().extent;
    auto min_corner = geom::Vector3D(
        bbox_pos.x - bbox_ext.x,
        bbox_pos.y - bbox_ext.y,
        bbox_pos.z - bbox_ext.z - epsilon);
    auto max_corner = geom::Vector3D(
        bbox_pos.x + bbox_ext.x,
        bbox_pos.y + bbox_ext.y,
        bbox_pos.z + bbox_ext.z + epsilon);
    Box box({min_corner.x, min_corner.y, min_corner.z},
        {max_corner.x, max_corner.y, max_corner.z});
    auto segments = _rtree.GetIntersections(box);

    for (size_t i = 0; i < segments.size(); ++i){
      auto &segment1 = segments[i];
      auto waypoint1 = segment1.second.first;
      JuncId junc_id1 = _data.GetRoad(waypoint1.road_id).GetJunctionId();
      // only segments in the junction
      if(junc_id1 != id){
        continue;
      }
      Segment2d seg1{{segment1.first.first.get<0>(), segment1.first.first.get<1>()},
          {segment1.first.second.get<0>(), segment1.first.second.get<1>()}};
      for (size_t j = i + 1; j < segments.size(); ++j){
        auto &segment2 = segments[j];
        auto waypoint2 = segment2.second.first;
        JuncId junc_id2 = _data.GetRoad(waypoint2.road_id).GetJunctionId();
        // only segments in the junction
        if(junc_id2 != id){
          continue;
        }
        // discard same road
        if(waypoint1.road_id == waypoint2.road_id){
          continue;
        }
        Segment2d seg2{{segment2.first.first.get<0>(), segment2.first.first.get<1>()},
            {segment2.first.second.get<0>(), segment2.first.second.get<1>()}};

        double distance = boost::geometry::distance(seg1, seg2);
        // better to set distance to lanewidth
        if(distance > 2.0){
          continue;
        }
        if(conflicts[waypoint1.road_id].count(waypoint2.road_id) == 0){
          conflicts[waypoint1.road_id].insert(waypoint2.road_id);
        }
        if(conflicts[waypoint2.road_id].count(waypoint1.road_id) == 0){
          conflicts[waypoint2.road_id].insert(waypoint1.road_id);
        }
      }
    }
    return conflicts;
  }

  const Lane &Map::GetLane(Waypoint waypoint) const {
    return _data.GetRoad(waypoint.road_id).GetLaneById(waypoint.section_id, waypoint.lane_id);
  }

  // ===========================================================================
  // -- Map: Private functions -------------------------------------------------
  // ===========================================================================

  // Adds a new element to the rtree element list using the position of the
  // waypoints both ends of the segment
  void Map::AddElementToRtree(
      std::vector<Rtree::TreeElement> &rtree_elements,
      geom::Transform &current_transform,
      geom::Transform &next_transform,
      Waypoint &current_waypoint,
      Waypoint &next_waypoint) {
    Rtree::BPoint init =
        Rtree::BPoint(
        current_transform.location.x,
        current_transform.location.y,
        current_transform.location.z);
    Rtree::BPoint end =
        Rtree::BPoint(
        next_transform.location.x,
        next_transform.location.y,
        next_transform.location.z);
    rtree_elements.emplace_back(std::make_pair(Rtree::BSegment(init, end),
        std::make_pair(current_waypoint, next_waypoint)));
  }
  // Adds a new element to the rtree element list using the position of the
  // waypoints, both ends of the segment
  void Map::AddElementToRtreeAndUpdateTransforms(
      std::vector<Rtree::TreeElement> &rtree_elements,
      geom::Transform &current_transform,
      Waypoint &current_waypoint,
      Waypoint &next_waypoint) {
    geom::Transform next_transform = ComputeTransform(next_waypoint);
    AddElementToRtree(rtree_elements, current_transform, next_transform,
    current_waypoint, next_waypoint);
    current_waypoint = next_waypoint;
    current_transform = next_transform;
  }

  // returns the remaining length of the geometry depending on the lane
  // direction
  double GetRemainingLength(const Lane &lane, double current_s) {
    if (lane.IsPositiveDirection()) {
      return (lane.GetDistance() + lane.GetLength() - current_s);
    } else {
      return (current_s - lane.GetDistance());
    }
  }

  void Map::CreateRtree() {
    const double epsilon = 0.000001; // small delta in the road (set to 1
                                     // micrometer to prevent numeric errors)
    const double min_delta_s = 1;    // segments of minimum 1m through the road

    // 1.8 degrees, maximum angle in a curve to place a segment
    constexpr double angle_threshold = geom::Math::Pi<double>() / 100.0;
    // maximum distance of a segment
    constexpr double max_segment_length = 100.0;

    // Generate waypoints at start of every lane
    std::vector<Waypoint> topology;
    for (const auto &pair : _data.GetRoads()) {
      const auto &road = pair.second;
      ForEachLane(road, Lane::LaneType::Any, [&](auto &&waypoint) {
        if(waypoint.lane_id != 0) {
          topology.push_back(waypoint);
        }
      });
    }

    // Container of segments and waypoints
    std::vector<Rtree::TreeElement> rtree_elements;
    // Loop through all lanes
    for (auto &waypoint : topology) {
      auto &lane_start_waypoint = waypoint;

      auto current_waypoint = lane_start_waypoint;

      const Lane &lane = GetLane(current_waypoint);

      geom::Transform current_transform = ComputeTransform(current_waypoint);

      // Save computation time in straight lines
      if (lane.IsStraight()) {
        double delta_s = min_delta_s;
        double remaining_length =
            GetRemainingLength(lane, current_waypoint.s);
        remaining_length -= epsilon;
        delta_s = remaining_length;
        if (delta_s < epsilon) {
          continue;
        }
        auto next = GetNext(current_waypoint, delta_s);

        RELEASE_ASSERT(next.size() == 1);
        RELEASE_ASSERT(next.front().road_id == current_waypoint.road_id);
        auto next_waypoint = next.front();

        AddElementToRtreeAndUpdateTransforms(
            rtree_elements,
            current_transform,
            current_waypoint,
            next_waypoint);
        // end of lane
      } else {
        auto next_waypoint = current_waypoint;

        // Loop until the end of the lane
        // Advance in small s-increments
        while (true) {
          double delta_s = min_delta_s;
          double remaining_length =
              GetRemainingLength(lane, next_waypoint.s);
          remaining_length -= epsilon;
          delta_s = std::min(delta_s, remaining_length);

          if (delta_s < epsilon) {
            AddElementToRtreeAndUpdateTransforms(
                rtree_elements,
                current_transform,
                current_waypoint,
                next_waypoint);
            break;
          }

          auto next = GetNext(next_waypoint, delta_s);
          if (next.size() != 1 ||
          current_waypoint.section_id != next.front().section_id) {
            AddElementToRtreeAndUpdateTransforms(
                rtree_elements,
                current_transform,
                current_waypoint,
                next_waypoint);
            break;
          }

          next_waypoint = next.front();
          geom::Transform next_transform = ComputeTransform(next_waypoint);
          double angle = geom::Math::GetVectorAngle(
              current_transform.GetForwardVector(), next_transform.GetForwardVector());

          if (std::abs(angle) > angle_threshold ||
              std::abs(current_waypoint.s - next_waypoint.s) > max_segment_length) {
            AddElementToRtree(
                rtree_elements,
                current_transform,
                next_transform,
                current_waypoint,
                next_waypoint);
            current_waypoint = next_waypoint;
            current_transform = next_transform;
          }
        }
      }
    }
    // Add segments to Rtree
    _rtree.InsertElements(rtree_elements);
  }

  Junction* Map::GetJunction(JuncId id) {
    return _data.GetJunction(id);
  }

  const Junction* Map::GetJunction(JuncId id) const {
    return _data.GetJunction(id);
  }

  geom::Mesh Map::GenerateMesh(
      const double distance,
      const float extra_width,
      const  bool smooth_junctions) const {
    RELEASE_ASSERT(distance > 0.0);
    geom::MeshFactory mesh_factory;
    geom::Mesh out_mesh;

    mesh_factory.road_param.resolution = static_cast<float>(distance);
    mesh_factory.road_param.extra_lane_width = extra_width;

    // Generate roads outside junctions
    for (auto &&pair : _data.GetRoads()) {
      const auto &road = pair.second;
      if (road.IsJunction()) {
        continue;
      }
      out_mesh += *mesh_factory.Generate(road);
    }

    // Generate roads within junctions and smooth them
    for (const auto &junc_pair : _data.GetJunctions()) {
      const auto &junction = junc_pair.second;
      std::vector<std::unique_ptr<geom::Mesh>> lane_meshes;
      for(const auto &connection_pair : junction.GetConnections()) {
        const auto &connection = connection_pair.second;
        const auto &road = _data.GetRoads().at(connection.connecting_road);
        for (auto &&lane_section : road.GetLaneSections()) {
          for (auto &&lane_pair : lane_section.GetLanes()) {
            lane_meshes.push_back(mesh_factory.Generate(lane_pair.second));
          }
        }
      }
      if(smooth_junctions) {
        out_mesh += *mesh_factory.MergeAndSmooth(lane_meshes);
      } else {
        geom::Mesh junction_mesh;
        for(auto& lane : lane_meshes) {
          junction_mesh += *lane;
        }
        out_mesh += junction_mesh;
      }
    }

    return out_mesh;
  }


  // Some real-world OpenDRIVE exports (DeepMap/NuRec) contain sub-2m road
  // stubs with an empty <link> element: not connected to anything, not part
  // of any junction, and geometrically sitting ON TOP of the real road they
  // were traced from -- often centimetres above it. Rendering them produces
  // a slab of road floating over the actual carriageway, with their torn
  // marking fragments poking through the surface. They carry no routable
  // topology (nothing can drive onto a link-less road), so skipping their
  // meshes loses nothing. Longer link-less roads are genuine dead-end
  // streets and are kept.
  static bool IsOrphanSliverRoad(const road::Road &road) {
    constexpr double kMaxSliverLength = 2.0; // meters
    return !road.IsJunction() &&
           road.GetLength() < kMaxSliverLength &&
           road.GetNexts().empty() &&
           road.GetPrevs().empty();
  }

  // The same exports also carry LONGER link-less roads (tens of meters).
  // Some are genuine dead-end streets; others are duplicate traces laid
  // over a road that already exists, floating centimetres above it (a
  // 30m slab of road on top of the real carriageway). Topology cannot
  // tell them apart -- both are unconnected -- so decide geometrically:
  // a link-less road most of whose driving-lane samples lie within a
  // lane-width of another (connected) road's lane centers at road level
  // is a duplicate trace. Support from fellow link-less candidates is
  // deliberately ignored so two orphans overlapping only each other are
  // both conservatively kept.
  static std::unordered_set<RoadId> ComputeDuplicateTraceRoads(
      const Map &map, const std::unordered_set<RoadId> &candidate_ids) {
    std::unordered_set<RoadId> duplicates;
    if (candidate_ids.empty()) {
      return duplicates;
    }
    constexpr double kSampleStep = 4.0;  // m along lanes
    constexpr double kMaxXY = 2.0;       // m to a supporting lane center
    constexpr double kMaxZ = 1.0;        // m height difference
    constexpr double kMinOverlapFraction = 0.4;
    const auto waypoints = map.GenerateWaypoints(kSampleStep);
    const auto CellKey = [](double x, double y) {
      const auto ix = static_cast<int64_t>(std::floor(x / kMaxXY));
      const auto iy = static_cast<int64_t>(std::floor(y / kMaxXY));
      return (ix << 32) ^ (iy & 0xffffffff);
    };
    std::unordered_map<int64_t, std::vector<geom::Location>> support_grid;
    std::unordered_map<RoadId, std::vector<geom::Location>> candidate_points;
    for (const auto &wp : waypoints) {
      const geom::Location loc = map.ComputeTransform(wp).location;
      if (candidate_ids.count(wp.road_id) != 0u) {
        candidate_points[wp.road_id].push_back(loc);
      } else {
        support_grid[CellKey(loc.x, loc.y)].push_back(loc);
      }
    }
    for (const auto &entry : candidate_points) {
      size_t supported = 0u;
      for (const geom::Location &loc : entry.second) {
        bool found = false;
        for (int dx = -1; dx <= 1 && !found; ++dx) {
          for (int dy = -1; dy <= 1 && !found; ++dy) {
            const auto it = support_grid.find(
                CellKey(loc.x + dx * kMaxXY, loc.y + dy * kMaxXY));
            if (it == support_grid.end()) {
              continue;
            }
            for (const geom::Location &other : it->second) {
              const double dxy = std::hypot(loc.x - other.x, loc.y - other.y);
              if (dxy < kMaxXY && std::abs(loc.z - other.z) < kMaxZ) {
                found = true;
                break;
              }
            }
          }
        }
        if (found) {
          ++supported;
        }
      }
      if (!entry.second.empty() &&
          static_cast<double>(supported) / static_cast<double>(entry.second.size()) >=
              kMinOverlapFraction) {
        duplicates.insert(entry.first);
      }
    }
    return duplicates;
  }

  std::unordered_set<RoadId> Map::ComputeSkippedGenerationRoads() const {
    std::unordered_set<RoadId> candidate_ids;
    std::unordered_set<RoadId> skipped;
    for (const auto &pair : _data.GetRoads()) {
      const auto &road = pair.second;
      if (road.IsJunction() || !road.GetNexts().empty() || !road.GetPrevs().empty()) {
        continue;
      }
      if (IsOrphanSliverRoad(road)) {
        skipped.insert(pair.first);
      } else {
        candidate_ids.insert(pair.first);
      }
    }
    for (RoadId id : ComputeDuplicateTraceRoads(*this, candidate_ids)) {
      skipped.insert(id);
    }
    return skipped;
  }

  std::vector<std::unique_ptr<geom::Mesh>> Map::GenerateChunkedMesh(
      const rpc::OpendriveGenerationParameters& params) const {
    geom::MeshFactory mesh_factory(params);
    std::vector<std::unique_ptr<geom::Mesh>> out_mesh_list;

    std::unordered_map<JuncId, geom::Mesh> junction_map;
    const auto skipped_roads = ComputeSkippedGenerationRoads();
    for (auto &&pair : _data.GetRoads()) {
      const auto &road = pair.second;
      if (!road.IsJunction() && skipped_roads.count(pair.first) == 0u) {
        std::vector<std::unique_ptr<geom::Mesh>> road_mesh_list =
            mesh_factory.GenerateAllWithMaxLen(road);

        out_mesh_list.insert(
            out_mesh_list.end(),
            std::make_move_iterator(road_mesh_list.begin()),
            std::make_move_iterator(road_mesh_list.end()));
      }
    }

    // Generate roads within junctions and smooth them
    for (const auto &junc_pair : _data.GetJunctions()) {
      const auto &junction = junc_pair.second;
      std::vector<std::unique_ptr<geom::Mesh>> lane_meshes;
      std::vector<std::unique_ptr<geom::Mesh>> sidewalk_lane_meshes;
      for(const auto &connection_pair : junction.GetConnections()) {
        const auto &connection = connection_pair.second;
        const auto &road = _data.GetRoads().at(connection.connecting_road);
        for (auto &&lane_section : road.GetLaneSections()) {
          for (auto &&lane_pair : lane_section.GetLanes()) {
            const auto &lane = lane_pair.second;
            if (lane.GetType() != road::Lane::LaneType::Sidewalk) {
              lane_meshes.push_back(mesh_factory.Generate(lane));
            } else {
              sidewalk_lane_meshes.push_back(mesh_factory.Generate(lane));
            }
          }
        }
      }
      if(params.smooth_junctions) {
        auto merged_mesh = mesh_factory.MergeAndSmooth(lane_meshes);
        // Pave the wedges left between the (smoothed) lane corridors before
        // the sidewalks are appended, so the fill follows the final z.
        if (auto fill = GenerateJunctionFill(junction, *merged_mesh)) {
          *merged_mesh += *fill;
        }
        for(auto& lane : sidewalk_lane_meshes) {
          *merged_mesh += *lane;
        }
        out_mesh_list.push_back(std::move(merged_mesh));
      } else {
        std::unique_ptr<geom::Mesh> junction_mesh = std::make_unique<geom::Mesh>();
        for(auto& lane : lane_meshes) {
          *junction_mesh += *lane;
        }
        if (auto fill = GenerateJunctionFill(junction, *junction_mesh)) {
          *junction_mesh += *fill;
        }
        for(auto& lane : sidewalk_lane_meshes) {
          *junction_mesh += *lane;
        }
        out_mesh_list.push_back(std::move(junction_mesh));
      }
    }

    // Guard against a valid-but-geometry-free xodr (e.g. no roads/junctions
    // at all, or every generated mesh happening to have zero vertices):
    // out_mesh_list.front() and per-mesh GetVertices().front() below used to
    // be called unconditionally, which segfaulted the server instead of
    // returning an error. Bail out with an empty chunk list; the only
    // caller (AOpenDriveGenerator::GenerateRoadMesh) already skips
    // zero-vertex meshes and tolerates an empty result.
    if (out_mesh_list.empty()) {
      return {};
    }

    bool found_seed = false;
    geom::Vector2D min_pos;
    for (auto & mesh : out_mesh_list) {
      if (!mesh->GetVertices().empty()) {
        const auto &seed_vertex = mesh->GetVertices().front();
        min_pos = geom::Vector2D(seed_vertex.x, seed_vertex.y);
        found_seed = true;
        break;
      }
    }
    if (!found_seed) {
      // Every generated mesh was empty; nothing to bin.
      return {};
    }

    auto max_pos = min_pos;
    for (auto & mesh : out_mesh_list) {
      if (mesh->GetVertices().empty()) {
        continue;
      }
      auto vertex = mesh->GetVertices().front();
      min_pos.x = std::min(min_pos.x, vertex.x);
      min_pos.y = std::min(min_pos.y, vertex.y);
      max_pos.x = std::max(max_pos.x, vertex.x);
      max_pos.y = std::max(max_pos.y, vertex.y);
    }
    size_t mesh_amount_x = static_cast<size_t>((max_pos.x - min_pos.x)/params.max_road_length) + 1;
    size_t mesh_amount_y = static_cast<size_t>((max_pos.y - min_pos.y)/params.max_road_length) + 1;
    std::vector<std::unique_ptr<geom::Mesh>> result;
    result.reserve(mesh_amount_x*mesh_amount_y);
    for (size_t i = 0; i < mesh_amount_x*mesh_amount_y; ++i) {
      result.emplace_back(std::make_unique<geom::Mesh>());
    }
    for (auto & mesh : out_mesh_list) {
      if (mesh->GetVertices().empty()) {
        continue;
      }
      auto vertex = mesh->GetVertices().front();
      size_t x_pos = static_cast<size_t>((vertex.x - min_pos.x) / params.max_road_length);
      size_t y_pos = static_cast<size_t>((vertex.y - min_pos.y) / params.max_road_length);
      *(result[x_pos + mesh_amount_x*y_pos]) += *mesh;
    }

    return result;
  }

  std::map<road::Lane::LaneType , std::vector<std::unique_ptr<geom::Mesh>>>
    Map::GenerateOrderedChunkedMeshInLocations( const rpc::OpendriveGenerationParameters& params,
                                     const geom::Vector3D& minpos,
                                     const geom::Vector3D& maxpos) const
  {

    geom::MeshFactory mesh_factory(params);
    std::map<road::Lane::LaneType, std::vector<std::unique_ptr<geom::Mesh>>> road_out_mesh_list;
    std::map<road::Lane::LaneType, std::vector<std::unique_ptr<geom::Mesh>>> junction_out_mesh_list;

    std::thread juntction_thread( &Map::GenerateJunctions, this, mesh_factory, params,
      minpos, maxpos, &junction_out_mesh_list);

    std::vector<RoadId> RoadsIDToGenerate = FilterRoadsByPosition(minpos, maxpos);
    // Drop link-less sliver/duplicate-trace roads (floating slabs stacked
    // on the real carriageway in DeepMap/NuRec exports).
    const auto skipped_roads = ComputeSkippedGenerationRoads();
    RoadsIDToGenerate.erase(
        std::remove_if(RoadsIDToGenerate.begin(), RoadsIDToGenerate.end(),
            [&skipped_roads](RoadId id) { return skipped_roads.count(id) != 0u; }),
        RoadsIDToGenerate.end());

    size_t num_roads = RoadsIDToGenerate.size();
    size_t num_roads_per_thread = 30;
    size_t num_threads = (num_roads / num_roads_per_thread) + 1;
    num_threads = num_threads > 1 ? num_threads : 1;
    std::vector<std::thread> workers;
    std::mutex write_mutex;
    std::cout << "Generating " << std::to_string(num_roads) << " roads" << std::endl;

    for ( size_t i = 0; i < num_threads; ++i ) {
      std::thread neworker(
        [this, &write_mutex, &mesh_factory, &RoadsIDToGenerate, &road_out_mesh_list, i, num_roads_per_thread]() {
        std::map<road::Lane::LaneType, std::vector<std::unique_ptr<geom::Mesh>>> Current =
          GenerateRoadsMultithreaded(mesh_factory, RoadsIDToGenerate,i, num_roads_per_thread );
        std::scoped_lock<std::mutex> guard(write_mutex);
        for ( auto&& pair : Current ) {
          if (road_out_mesh_list.find(pair.first) != road_out_mesh_list.end()) {
            road_out_mesh_list[pair.first].insert(road_out_mesh_list[pair.first].end(),
              std::make_move_iterator(pair.second.begin()),
              std::make_move_iterator(pair.second.end()));
          } else {
            road_out_mesh_list[pair.first] = std::move(pair.second);
          }
        }
      });
      workers.push_back(std::move(neworker));
    }

    for (size_t i = 0; i < workers.size(); ++i) {
      workers[i].join();
    }
    workers.clear();
    for (size_t i = 0; i < workers.size(); ++i) {
      if (workers[i].joinable()) {
        workers[i].join();
      }
    }

    juntction_thread.join();
    for (auto&& pair : junction_out_mesh_list) {
      if (road_out_mesh_list.find(pair.first) != road_out_mesh_list.end())
      {
        road_out_mesh_list[pair.first].insert(road_out_mesh_list[pair.first].end(),
          std::make_move_iterator(pair.second.begin()),
          std::make_move_iterator(pair.second.end()));
      }
      else
      {
        road_out_mesh_list[pair.first] = std::move(pair.second);
      }
    }
    std::cout << "Generated " << std::to_string(num_roads) << " roads" << std::endl;

    return road_out_mesh_list;
  }

  std::vector<std::pair<geom::Transform, std::string>> Map::GetTreesTransform(
    const geom::Vector3D& minpos,
    const geom::Vector3D& maxpos,
    float distancebetweentrees,
    float distancefromdrivinglineborder,
    float s_offset,
    bool measure_from_curb,
    bool keep_on_sidewalk) const {

    std::vector<std::pair<geom::Transform, std::string>> transforms;

    // Walks outward from the outermost driving lane over every lane that is
    // still roadway (parking, biking, border, shoulder, ...) and returns the
    // last one -- the lane whose outer edge is the curb line -- plus the
    // sidewalk right past it, if any. Only the driving lane's own edge is
    // considered when measure_from_curb is off (legacy tree placement).
    auto FindCurbLane = [measure_from_curb](
        const road::LaneSection &section, const road::Lane *driving_lane,
        const road::Lane *&sidewalk_lane) -> const road::Lane * {
      sidewalk_lane = nullptr;
      const road::Lane *curb_lane = driving_lane;
      if (!measure_from_curb) {
        return curb_lane;
      }
      const auto &lanes = section.GetLanes();
      const LaneId step = driving_lane->GetId() < 0 ? -1 : 1;
      for (LaneId id = driving_lane->GetId() + step;; id += step) {
        const auto it = lanes.find(id);
        if (it == lanes.end()) {
          break;
        }
        const auto type = it->second.GetType();
        if (type == Lane::LaneType::Sidewalk) {
          sidewalk_lane = &it->second;
          break;
        }
        if (type == Lane::LaneType::None) {
          break;
        }
        curb_lane = &it->second;
      }
      return curb_lane;
    };

    const std::vector<RoadId> RoadsIDToGenerate = FilterRoadsByPosition(minpos, maxpos);
    for ( RoadId id : RoadsIDToGenerate ) {
      const auto& road = _data.GetRoads().at(id);
      if (!road.IsJunction()) {
        for (auto &&lane_section : road.GetLaneSections()) {
          LaneId min_lane = 0; // most negative (outermost right-side) driving lane
          LaneId max_lane = 0; // most positive (outermost left-side) driving lane
          for (auto &pairlane : lane_section.GetLanes()) {
            if (pairlane.second.GetType() == Lane::LaneType::Driving) {
              if (pairlane.first < 0 && (min_lane == 0 || pairlane.first < min_lane)) {
                min_lane = pairlane.first;
              } else if (pairlane.first > 0 && pairlane.first > max_lane) {
                max_lane = pairlane.first;
              }
            }
          }

          // Prefer the outermost right-side lane; fall back to the outermost
          // left-side lane for one-way roads that only have positive-ID lanes.
          // Skip if no driving lane is found on either side (avoids using the
          // reference lane whose near-zero width places trees at road centre).
          const LaneId outer_lane = (min_lane != 0) ? min_lane : max_lane;
          if (outer_lane == 0) continue;

          const road::Lane* lane = lane_section.GetLane(outer_lane);
          if( lane ) {
            double s_current = lane_section.GetDistance() + s_offset;
            const double s_end = lane_section.GetDistance() + lane_section.GetLength();
            while(s_current < s_end){
              if(lane->GetWidth(s_current) != 0.0f){
                const auto edges = lane->GetCornerPositions(s_current, 0);
                geom::Vector3D director = edges.second - edges.first;
                // Use double precision for the length check to avoid false
                // negatives from float cancellation on near-equal corners.
                const double director_squared_length =
                  static_cast<double>(director.x) * static_cast<double>(director.x) +
                  static_cast<double>(director.y) * static_cast<double>(director.y) +
                  static_cast<double>(director.z) * static_cast<double>(director.z);
                // Skip degenerate or near-degenerate lane widths; normalising a
                // near-zero vector produces unstable directions and can place
                // trees on or very close to the road surface.
                if (director_squared_length <= (TREE_PLACEMENT_EPSILON * TREE_PLACEMENT_EPSILON)) {
                  s_current += distancebetweentrees;
                  continue;
                }
                // GetCornerPositions returns the lane corners in (t_offset + width, t_offset - width) order.
                // The true outer edge therefore depends on the lane side: for positive lane IDs it is the second corner,
                // and for negative lane IDs it is the first. Offset farther outward so trees are always placed away from the driving surface.
                const bool is_positive_lane = (lane->GetId() > 0);
                const geom::Vector3D first_corner = edges.first;
                const geom::Vector3D second_corner = edges.second;
                geom::Vector3D outer_corner =
                  is_positive_lane ? second_corner : first_corner;
                const geom::Vector3D inner_corner =
                  is_positive_lane ? first_corner : second_corner;
                const geom::Vector3D outward_direction =
                    (outer_corner - inner_corner).MakeUnitVector();
                // Street furniture is measured from the curb, not from the
                // driving lane: a parking or bike lane in between would
                // otherwise put a fixed 4 m offset in the middle of the
                // roadway, and where there is nothing in between the same
                // offset overshoots a 1.5 m sidewalk into the buildings.
                // With keep_on_sidewalk the anchor never goes past the
                // middle of the sidewalk lane that follows the curb.
                float offset = distancefromdrivinglineborder;
                const road::Lane *sidewalk_lane = nullptr;
                const road::Lane *curb_lane = FindCurbLane(lane_section, lane, sidewalk_lane);
                if (curb_lane != lane) {
                  const auto curb_edges = curb_lane->GetCornerPositions(s_current, 0);
                  // Pick the curb lane's corner farthest along the outward
                  // direction: robust to the corner ordering convention.
                  const float d_first = (curb_edges.first - inner_corner).x * outward_direction.x +
                                        (curb_edges.first - inner_corner).y * outward_direction.y;
                  const float d_second = (curb_edges.second - inner_corner).x * outward_direction.x +
                                         (curb_edges.second - inner_corner).y * outward_direction.y;
                  outer_corner = d_first > d_second ? curb_edges.first : curb_edges.second;
                }
                if (keep_on_sidewalk && sidewalk_lane != nullptr) {
                  const float sidewalk_width = static_cast<float>(sidewalk_lane->GetWidth(s_current));
                  if (sidewalk_width > 0.0f) {
                    offset = std::min(offset, 0.5f * sidewalk_width);
                  }
                }
                geom::Vector3D treeposition =
                    outer_corner + outward_direction * offset;
                // Face the anchor toward the road instead of along it: the
                // street furniture spawned on these transforms (lamps, signage)
                // extends along its local +X, so the lane heading would leave a
                // street lamp's arm -- and its light cone -- running parallel
                // to the road over the shoulder. outward_direction is already
                // the road->anchor lateral direction in UE frame; its opposite
                // is the yaw the furniture should face.
                const geom::Vector3D inward_direction = outward_direction * (-1.0f);
                const geom::Rotation facing_rotation(
                    0.0f,
                    geom::Math::ToDegrees(std::atan2(inward_direction.y, inward_direction.x)),
                    0.0f);
                geom::Transform treeTransform(treeposition, facing_rotation);
                const carla::road::element::RoadInfoSpeed* roadinfo = lane->GetInfo<carla::road::element::RoadInfoSpeed>(s_current);
                // roadinfo is null for roads without an explicit maxspeed OSM tag
                // (common in urban areas that rely on default speed limits).
                if (roadinfo) {
                  transforms.push_back(std::make_pair(treeTransform, roadinfo->GetType()));
                } else {
                  transforms.push_back(std::make_pair(treeTransform, "Town"));
                }
              }
              s_current += distancebetweentrees;
            }

          }
        }
      }
    }
    return transforms;
  }

  geom::Mesh Map::GetAllCrosswalkMesh() const {
    geom::Mesh out_mesh;

    // Get the crosswalk vertices for the current map
    const std::vector<geom::Location> crosswalk_vertex = GetAllCrosswalkZones();
    if (crosswalk_vertex.empty()) {
      return out_mesh;
    }

    // One triangle fan per crosswalk polygon, with planar UVs laid out in
    // the polygon's own frame: U runs along its longest edge (across the
    // road, the direction the stripes repeat in), V spans the short side
    // (the crosswalk's depth along the road) exactly once, and U tiles
    // with the same metre-to-texel scale so the texture isn't stretched.
    // Without UVs every vertex sampled texel (0,0) and the slabs rendered
    // as one flat colour.
    auto AddCrosswalkPolygon = [&out_mesh](const std::vector<geom::Vector3D> &poly) {
      if (poly.size() < 3) {
        return;
      }
      size_t longest = 0;
      float longest_len2 = -1.0f;
      for (size_t k = 0; k < poly.size(); ++k) {
        const auto &p = poly[k];
        const auto &q = poly[(k + 1) % poly.size()];
        const float dx = q.x - p.x, dy = q.y - p.y;
        const float len2 = dx * dx + dy * dy;
        if (len2 > longest_len2) {
          longest_len2 = len2;
          longest = k;
        }
      }
      geom::Vector2D u_dir(1.0f, 0.0f);
      if (longest_len2 > 1e-6f) {
        const auto &p = poly[longest];
        const auto &q = poly[(longest + 1) % poly.size()];
        const float inv = 1.0f / std::sqrt(longest_len2);
        u_dir = geom::Vector2D((q.x - p.x) * inv, (q.y - p.y) * inv);
      }
      const geom::Vector2D v_dir(-u_dir.y, u_dir.x);
      float a_min = std::numeric_limits<float>::max(), b_min = a_min;
      float a_max = std::numeric_limits<float>::lowest(), b_max = a_max;
      std::vector<std::pair<float, float>> local;
      local.reserve(poly.size());
      for (const auto &p : poly) {
        const float a = p.x * u_dir.x + p.y * u_dir.y;
        const float b = p.x * v_dir.x + p.y * v_dir.y;
        local.emplace_back(a, b);
        a_min = std::min(a_min, a); a_max = std::max(a_max, a);
        b_min = std::min(b_min, b); b_max = std::max(b_max, b);
      }
      const float depth = std::max(b_max - b_min, 0.5f);
      const size_t first = out_mesh.GetVerticesNum() + 1; // 1-based
      for (size_t k = 0; k < poly.size(); ++k) {
        out_mesh.AddVertex(poly[k]);
        out_mesh.AddUV(geom::Vector2D(
            (local[k].first - a_min) / depth,
            (local[k].second - b_min) / depth));
      }
      for (size_t k = 1; k + 1 < poly.size(); ++k) {
        out_mesh.AddIndex(first);
        out_mesh.AddIndex(first + k);
        out_mesh.AddIndex(first + k + 1);
      }
    };

    // Create a a list of triangle fans with material "crosswalk"
    out_mesh.AddMaterial("crosswalk");
    size_t start_vertex_index = 0;
    size_t i = 0;
    std::vector<geom::Vector3D> vertices;
    // Iterate the vertices until a repeated one is found, this indicates
    // the triangle fan is done and another one must start
    do {
      // Except for the first iteration && triangle fan done
      if (i != 0 && crosswalk_vertex[start_vertex_index] == crosswalk_vertex[i]) {
        // Create the actual fan
        AddCrosswalkPolygon(vertices);
        vertices.clear();
        // End the loop if i reached the end of the vertex list
        if (i >= crosswalk_vertex.size() - 1) {
          break;
        }
        start_vertex_index = ++i;
      }
      // Append a new Vector3D that will be added to the triangle fan
      vertices.push_back(crosswalk_vertex[i++]);
    } while (i < crosswalk_vertex.size());

    out_mesh.EndMaterial();
    return out_mesh;
  }

  /// Buids a list of meshes related with LineMarkings
  std::vector<std::unique_ptr<geom::Mesh>> Map::GenerateLineMarkings(
    const rpc::OpendriveGenerationParameters& params,
    const geom::Vector3D& minpos,
    const geom::Vector3D& maxpos,
    std::vector<std::string>& outinfo ) const
  {
    std::vector<std::unique_ptr<geom::Mesh>> LineMarks;
    geom::MeshFactory mesh_factory(params);

    // A real junction fans one lane out into several connecting roads, so
    // sweeping markings per-lane the way ordinary roads do would be wrong
    // there. Some OpenDRIVE exports (e.g. DeepMap/NuRec real-world
    // reconstructions) instead wrap a junction record around what is really
    // just a single road-to-road splice at a curvature break -- one real
    // incoming->connecting link, occasionally padded with a placeholder
    // incomingRoad="-1" connection. Unconditionally skipping every junction
    // road left "clean seam" bald patches wherever a continuously-marked
    // street happened to run through one of these trivial junctions, even
    // though the connecting road carries its own real per-lane roadMark data.
    auto HasSingleConnectingRoad = [this](JuncId junction_id) {
      const Junction *junction = _data.GetJunction(junction_id);
      if (junction == nullptr) {
        return false;
      }
      std::unordered_set<RoadId> connecting_roads;
      for (const auto &connection_pair : junction->GetConnections()) {
        connecting_roads.insert(connection_pair.second.connecting_road);
      }
      return connecting_roads.size() == 1;
    };

    const auto skipped_roads = ComputeSkippedGenerationRoads();
    const std::vector<RoadId> RoadsIDToGenerate = FilterRoadsByPosition(minpos, maxpos);
    for ( RoadId id : RoadsIDToGenerate ) {
      const auto& road = _data.GetRoads().at(id);
      if (skipped_roads.count(id) != 0u) {
        continue; // road mesh is skipped too -- see GenerateChunkedMesh
      }
      if (!road.IsJunction() || HasSingleConnectingRoad(road.GetJunctionId())) {
        mesh_factory.GenerateLaneMarkForRoad(road, LineMarks, outinfo);
      }
    }

    return LineMarks;
  }

  std::vector<carla::geom::BoundingBox> Map::GetJunctionsBoundingBoxes() const {
    std::vector<carla::geom::BoundingBox> returning;
    for ( const auto& junc_pair : _data.GetJunctions() ) {
      const auto& junction = junc_pair.second;
      float box_extraextension_factor = 1.5f;
      carla::geom::BoundingBox bb = junction.GetBoundingBox();
      bb.extent *= box_extraextension_factor;
      returning.push_back(bb);
    }
    return returning;
  }

  inline float Map::GetZPosInDeformation(float posx, float posy) const {
    return geom::deformation::GetZPosInDeformation(posx, posy) +
      geom::deformation::GetBumpDeformation(posx,posy);
  }

  std::map<road::Lane::LaneType, std::vector<std::unique_ptr<geom::Mesh>>>
      Map::GenerateRoadsMultithreaded( const carla::geom::MeshFactory& mesh_factory,
                                        const std::vector<RoadId>& RoadsId,
                                        const size_t index, const size_t number_of_roads_per_thread) const
  {
    std::map<road::Lane::LaneType, std::vector<std::unique_ptr<geom::Mesh>>> out;

    size_t start = index * number_of_roads_per_thread;
    size_t endoffset = (index+1) * number_of_roads_per_thread;
    size_t end = RoadsId.size();

    for (size_t i = start; i < endoffset && i < end; ++i) {
      const auto& road = _data.GetRoads().at(RoadsId[i]);
      if (!road.IsJunction()) {
        mesh_factory.GenerateAllOrderedWithMaxLen(road, out);
      }
    }
    std::cout << "Generated roads from " + std::to_string(index * number_of_roads_per_thread) + " to " + std::to_string((index+1) * number_of_roads_per_thread ) << std::endl;
    return out;
  }

  void Map::GenerateJunctions(const carla::geom::MeshFactory& mesh_factory,
    const rpc::OpendriveGenerationParameters& /*params*/,
    const geom::Vector3D& minpos,
    const geom::Vector3D& maxpos,
    std::map<road::Lane::LaneType,
    std::vector<std::unique_ptr<geom::Mesh>>>* junction_out_mesh_list) const {

    std::vector<JuncId> JunctionsToGenerate = FilterJunctionsByPosition(minpos, maxpos);
    size_t num_junctions = JunctionsToGenerate.size();
    std::cout << "Generating " << std::to_string(num_junctions) << " junctions" << std::endl;
    size_t num_junctions_per_thread = 5;
    size_t num_threads = (num_junctions / num_junctions_per_thread) + 1;
    num_threads = num_threads > 1 ? num_threads : 1;
    std::vector<std::thread> workers;
    std::mutex write_mutex;

    for ( size_t i = 0; i < num_threads; ++i ) {
      std::thread neworker(
        [this, &write_mutex, &mesh_factory, &junction_out_mesh_list, JunctionsToGenerate, i, num_junctions_per_thread, num_junctions]() {
        std::map<road::Lane::LaneType,
          std::vector<std::unique_ptr<geom::Mesh>>> junctionsofthisthread;

        size_t minimum = 0;
        if( (i + 1) * num_junctions_per_thread < num_junctions ){
          minimum = (i + 1) * num_junctions_per_thread;
        }else{
          minimum = num_junctions;
        }
        std::cout << "Generating Junctions between  " << std::to_string(i * num_junctions_per_thread) << " and " << std::to_string(minimum) << std::endl;

        for ( size_t junctionindex = i * num_junctions_per_thread;
                        junctionindex < minimum;
                        ++junctionindex )
        {
          GenerateSingleJunction(mesh_factory, JunctionsToGenerate[junctionindex], &junctionsofthisthread);
        }
        std::cout << "Generated Junctions between  " << std::to_string(i * num_junctions_per_thread) << " and " << std::to_string(minimum) << std::endl;
        std::scoped_lock<std::mutex> guard(write_mutex);
        for ( auto&& pair : junctionsofthisthread ) {
          if ((*junction_out_mesh_list).find(pair.first) != (*junction_out_mesh_list).end()) {
            (*junction_out_mesh_list)[pair.first].insert((*junction_out_mesh_list)[pair.first].end(),
              std::make_move_iterator(pair.second.begin()),
              std::make_move_iterator(pair.second.end()));
          } else {
            (*junction_out_mesh_list)[pair.first] = std::move(pair.second);
          }
        }
      });
      workers.push_back(std::move(neworker));
    }

    for (size_t i = 0; i < workers.size(); ++i) {
      workers[i].join();
    }
    workers.clear();
    for (size_t i = 0; i < workers.size(); ++i) {
      if (workers[i].joinable()) {
        workers[i].join();
      }
    }
  }

  std::vector<JuncId> Map::FilterJunctionsByPosition( const geom::Vector3D& minpos,
    const geom::Vector3D& maxpos ) const {

    std::cout << "Filtered from " + std::to_string(_data.GetJunctions().size() ) + " junctions " << std::endl;
    std::vector<JuncId> ToReturn;
    for( auto& junction : _data.GetJunctions() ){
      geom::Location junctionLocation = junction.second.GetBoundingBox().location;
      if( minpos.x < junctionLocation.x && junctionLocation.x < maxpos.x &&
            minpos.y > junctionLocation.y && junctionLocation.y > maxpos.y ) {
        ToReturn.push_back(junction.first);
      }
    }
    std::cout << "To " + std::to_string(ToReturn.size() ) + " junctions " << std::endl;

    return ToReturn;
  }

  std::vector<RoadId> Map::FilterRoadsByPosition( const geom::Vector3D& minpos,
    const geom::Vector3D& maxpos ) const {

    std::vector<RoadId> ToReturn;
    std::cout << "Filtered from " + std::to_string(_data.GetRoads().size() ) + " roads " << std::endl;
    for( auto& road : _data.GetRoads() ){
      auto &&lane_section = (*road.second.GetLaneSections().begin());
      const road::Lane* lane = road.second.IsRHT() ? lane_section.GetLane(-1) : lane_section.GetLane(1);
      if (!lane) {
        // Fallback: the expected innermost lane (−1 for RHT, +1 for LHT) is
        // absent (common on one-way streets and complex urban junctions).
        // Pick the driving lane closest to the reference line (smallest abs id)
        // to minimise the position error used for bounding-box filtering.
        int best_abs_id = std::numeric_limits<int>::max();
        for (const auto& pairlane : lane_section.GetLanes()) {
          if (pairlane.first != 0 && pairlane.second.GetType() == Lane::LaneType::Driving) {
            const int abs_id = std::abs(pairlane.first);
            if (abs_id < best_abs_id) {
              best_abs_id = abs_id;
              lane = &pairlane.second;
            }
          }
        }
      }
      if( lane ) {
        const double s_check = lane_section.GetDistance() + lane_section.GetLength() * 0.5;
        geom::Location roadLocation = lane->ComputeTransform(s_check).location;
        if( minpos.x < roadLocation.x && roadLocation.x < maxpos.x &&
              minpos.y > roadLocation.y && roadLocation.y > maxpos.y ) {
          ToReturn.push_back(road.first);
        }
      }
    }
    std::cout << "To " + std::to_string(ToReturn.size() ) + " roads " << std::endl;
    return ToReturn;
  }

  std::unique_ptr<geom::Mesh> Map::GenerateJunctionFill(
      const road::Junction &junction,
      const geom::Mesh &corridor_mesh) const {
    // The runtime junction mesh is the union of the connecting roads' lane
    // corridors. On anything bigger than a simple crossing that leaves the
    // ground showing through every region the corridors enclose but don't
    // cover (the wedges between diverging turns, the middle of wide
    // 4-arm/dual-carriageway crossings). This rasterizes the corridors plus
    // a short "cap" of every arm road at its contact end, floods the grid
    // from the outside, and paves every uncovered cell the flood can't
    // reach -- i.e. exactly the holes enclosed by junction roadway, and
    // nothing outside it (curb returns between adjacent arms, or the open
    // bays of an irregular junction, stay ground: they are not enclosed).
    // The fill follows the corridor surface (barycentric z where it
    // overlaps a corridor, inverse-distance blend of the corridor vertices
    // in the holes) and sits a hair below it so the corridor always wins
    // the depth test where the two coincide.
    using Vec3 = geom::Vector3D;
    struct Tri { Vec3 a, b, c; };

    constexpr float kCell = 0.5f;          // fill grid resolution (m)
    constexpr float kBin = 4.0f;           // triangle/vertex spatial hash (m)
    constexpr float kCapLength = 5.0f;     // arm cap extruded past the contact line (m)
    constexpr float kArmSnap = 3.0f;       // arm end must lie this close to a corridor end (m)
    constexpr float kZBelow = 0.015f;      // fill sits this far below the corridors (m)
    constexpr int kMargin = 2;             // grid cells of padding around the footprint
    constexpr size_t kMaxVertices = 4000000; // sanity cap on the grid size

    const auto &verts = corridor_mesh.GetVertices();
    const auto &idx = corridor_mesh.GetIndexes();
    if (verts.size() < 3 || idx.size() < 3) {
      return nullptr;
    }

    std::vector<Tri> corridor_tris;
    corridor_tris.reserve(idx.size() / 3);
    for (size_t i = 0; i + 2 < idx.size(); i += 3) {
      // geom::Mesh indices are 1-based
      corridor_tris.push_back({verts[idx[i] - 1], verts[idx[i + 1] - 1], verts[idx[i + 2] - 1]});
    }

    // -- arm caps -----------------------------------------------------------
    // Every road that touches the junction (incoming roads and the
    // predecessor/successor of each connecting road) contributes a quad
    // spanning its full roadway width at the contact end, extruded a few
    // metres away from the junction. They close the wedges between
    // corridors that diverge from / converge on the same arm, which are
    // otherwise open toward the arm and would read as "outside".
    std::vector<Vec3> corridor_ends;
    std::unordered_set<RoadId> arm_candidates;
    const auto &roads = _data.GetRoads();
    for (const auto &connection_pair : junction.GetConnections()) {
      const auto &connection = connection_pair.second;
      const auto it = roads.find(connection.connecting_road);
      if (it == roads.end()) {
        continue;
      }
      const auto &cr = it->second;
      corridor_ends.push_back(cr.GetDirectedPointIn(0.0).location);
      corridor_ends.push_back(cr.GetDirectedPointIn(cr.GetLength()).location);
      arm_candidates.insert(connection.incoming_road);
      arm_candidates.insert(cr.GetPredecessor());
      arm_candidates.insert(cr.GetSuccessor());
    }

    auto NearestCorridorEnd = [&corridor_ends](const Vec3 &p) {
      float best = std::numeric_limits<float>::max();
      for (const auto &e : corridor_ends) {
        best = std::min(best, (e - p).Length2D());
      }
      return best;
    };

    std::vector<Tri> cap_tris;
    for (RoadId arm_id : arm_candidates) {
      const auto it = roads.find(arm_id);
      if (it == roads.end() || it->second.IsJunction()) {
        continue;
      }
      const auto &arm = it->second;
      const double length = arm.GetLength();
      if (length < 1e-3) {
        continue;
      }
      const Vec3 p_start = arm.GetDirectedPointIn(0.0).location;
      const Vec3 p_end = arm.GetDirectedPointIn(length).location;
      const float d_start = NearestCorridorEnd(p_start);
      const float d_end = NearestCorridorEnd(p_end);
      if (std::min(d_start, d_end) > kArmSnap) {
        continue; // link id that isn't actually this junction's arm
      }
      const bool at_end = d_end < d_start;
      const double s_c = at_end ? length : 0.0;
      const double s_in = at_end ? std::max(0.0, length - 1.0) : std::min(length, 1.0);
      const Vec3 p_in = arm.GetDirectedPointIn(s_in).location;
      Vec3 outward = at_end ? Vec3(p_end.x - p_in.x, p_end.y - p_in.y, 0.0f)
                            : Vec3(p_start.x - p_in.x, p_start.y - p_in.y, 0.0f);
      outward.z = 0.0f;
      if (outward.Length2D() < 1e-4f) {
        continue;
      }
      outward = outward.MakeUnitVector();
      const Vec3 normal(-outward.y, outward.x, 0.0f);

      bool have_corner = false;
      float t_min = 0.0f, t_max = 0.0f;
      Vec3 c_min, c_max;
      for (const road::Lane *lane : arm.GetLanesByDistance(s_c)) {
        if (lane == nullptr || lane->GetId() == 0 ||
            lane->GetType() == road::Lane::LaneType::Sidewalk ||
            lane->GetType() == road::Lane::LaneType::None) {
          continue;
        }
        const auto corners = lane->GetCornerPositions(s_c, 0.0f);
        for (const Vec3 &c : {corners.first, corners.second}) {
          const float t = (c.x - p_start.x) * normal.x + (c.y - p_start.y) * normal.y;
          if (!have_corner || t < t_min) { t_min = t; c_min = c; }
          if (!have_corner || t > t_max) { t_max = t; c_max = c; }
          have_corner = true;
        }
      }
      if (!have_corner || (t_max - t_min) < 0.5f) {
        continue;
      }
      const Vec3 o = outward * kCapLength;
      cap_tris.push_back({c_min, c_max, c_max + o});
      cap_tris.push_back({c_min, c_max + o, c_min + o});
    }

    // -- grid ---------------------------------------------------------------
    float min_x = std::numeric_limits<float>::max(), min_y = min_x;
    float max_x = std::numeric_limits<float>::lowest(), max_y = max_x;
    auto Extend = [&](const Vec3 &v) {
      min_x = std::min(min_x, v.x); max_x = std::max(max_x, v.x);
      min_y = std::min(min_y, v.y); max_y = std::max(max_y, v.y);
    };
    for (const auto &v : verts) { Extend(v); }
    for (const auto &t : cap_tris) { Extend(t.a); Extend(t.b); Extend(t.c); }

    const float origin_x = min_x - kMargin * kCell;
    const float origin_y = min_y - kMargin * kCell;
    const int nvx = static_cast<int>(std::ceil((max_x - min_x) / kCell)) + 2 * kMargin + 1;
    const int nvy = static_cast<int>(std::ceil((max_y - min_y) / kCell)) + 2 * kMargin + 1;
    if (nvx < 3 || nvy < 3 || static_cast<size_t>(nvx) * static_cast<size_t>(nvy) > kMaxVertices) {
#ifdef LIBCARLA_JFILL_DEBUG
      std::cout << "[jfill] junction " << junction.GetId() << " BAIL grid " << nvx << "x" << nvy << std::endl;
#endif
      return nullptr;
    }
    const int ncx = nvx - 1, ncy = nvy - 1;

    // spatial hash of triangles and corridor vertices
    const int nbx = static_cast<int>((nvx * kCell) / kBin) + 1;
    const int nby = static_cast<int>((nvy * kCell) / kBin) + 1;
    auto BinX = [&](float x) { return std::clamp(static_cast<int>((x - origin_x) / kBin), 0, nbx - 1); };
    auto BinY = [&](float y) { return std::clamp(static_cast<int>((y - origin_y) / kBin), 0, nby - 1); };
    std::vector<std::vector<int>> corridor_bins(static_cast<size_t>(nbx) * nby);
    std::vector<std::vector<int>> cap_bins(static_cast<size_t>(nbx) * nby);
    std::vector<std::vector<int>> vertex_bins(static_cast<size_t>(nbx) * nby);
    auto BinTris = [&](const std::vector<Tri> &tris, std::vector<std::vector<int>> &bins) {
      for (size_t i = 0; i < tris.size(); ++i) {
        const Tri &t = tris[i];
        const int bx0 = BinX(std::min({t.a.x, t.b.x, t.c.x}));
        const int bx1 = BinX(std::max({t.a.x, t.b.x, t.c.x}));
        const int by0 = BinY(std::min({t.a.y, t.b.y, t.c.y}));
        const int by1 = BinY(std::max({t.a.y, t.b.y, t.c.y}));
        for (int by = by0; by <= by1; ++by) {
          for (int bx = bx0; bx <= bx1; ++bx) {
            bins[static_cast<size_t>(by) * nbx + bx].push_back(static_cast<int>(i));
          }
        }
      }
    };
    BinTris(corridor_tris, corridor_bins);
    BinTris(cap_tris, cap_bins);
    for (size_t i = 0; i < verts.size(); ++i) {
      vertex_bins[static_cast<size_t>(BinY(verts[i].y)) * nbx + BinX(verts[i].x)].push_back(static_cast<int>(i));
    }

    // Barycentric point-in-triangle (2D); returns the surface z on a hit.
    auto HitTriangle = [](const Tri &t, float px, float py, float &z_out) {
      const float denom = (t.b.y - t.c.y) * (t.a.x - t.c.x) + (t.c.x - t.b.x) * (t.a.y - t.c.y);
      if (std::abs(denom) < 1e-9f) {
        return false;
      }
      const float w0 = ((t.b.y - t.c.y) * (px - t.c.x) + (t.c.x - t.b.x) * (py - t.c.y)) / denom;
      const float w1 = ((t.c.y - t.a.y) * (px - t.c.x) + (t.a.x - t.c.x) * (py - t.c.y)) / denom;
      const float w2 = 1.0f - w0 - w1;
      constexpr float eps = -1e-3f;
      if (w0 < eps || w1 < eps || w2 < eps) {
        return false;
      }
      z_out = w0 * t.a.z + w1 * t.b.z + w2 * t.c.z;
      return true;
    };

    // per grid vertex: covered by a corridor (with z) / by an arm cap
    std::vector<uint8_t> v_corridor(static_cast<size_t>(nvx) * nvy, 0);
    std::vector<uint8_t> v_cap(static_cast<size_t>(nvx) * nvy, 0);
    std::vector<float> v_z(static_cast<size_t>(nvx) * nvy, 0.0f);
    for (int vy = 0; vy < nvy; ++vy) {
      for (int vx = 0; vx < nvx; ++vx) {
        const float px = origin_x + vx * kCell;
        const float py = origin_y + vy * kCell;
        const size_t vid = static_cast<size_t>(vy) * nvx + vx;
        const size_t bin = static_cast<size_t>(BinY(py)) * nbx + BinX(px);
        float z = 0.0f;
        for (int ti : corridor_bins[bin]) {
          if (HitTriangle(corridor_tris[ti], px, py, z)) {
            v_corridor[vid] = 1;
            v_z[vid] = z;
            break;
          }
        }
        for (int ti : cap_bins[bin]) {
          if (HitTriangle(cap_tris[ti], px, py, z)) {
            v_cap[vid] = 1;
            break;
          }
        }
      }
    }

    // per cell: blocked (fully paved, or an arm cap) vs. open
    auto CellId = [&](int cx, int cy) { return static_cast<size_t>(cy) * ncx + cx; };
    std::vector<uint8_t> blocked(static_cast<size_t>(ncx) * ncy, 0);
    std::vector<uint8_t> full(static_cast<size_t>(ncx) * ncy, 0);
    for (int cy = 0; cy < ncy; ++cy) {
      for (int cx = 0; cx < ncx; ++cx) {
        const size_t v00 = static_cast<size_t>(cy) * nvx + cx;
        const size_t v10 = v00 + 1, v01 = v00 + nvx, v11 = v01 + 1;
        const int n_corr = v_corridor[v00] + v_corridor[v10] + v_corridor[v01] + v_corridor[v11];
        const bool any_cap = v_cap[v00] || v_cap[v10] || v_cap[v01] || v_cap[v11];
        full[CellId(cx, cy)] = (n_corr == 4);
        blocked[CellId(cx, cy)] = (n_corr == 4 || any_cap);
      }
    }

    // Morphological closing of the corridor coverage: concave pockets
    // narrower than ~2*kCloseRadius get paved even when they are open to
    // the outside -- the wedges between corridors of *different* arms and
    // the curb-return mouths between adjacent arms, which a pure
    // enclosed-hole fill leaves green on big irregular junctions (Sunnyvale
    // j59: most wedges connect to the perimeter somewhere). Chamfer
    // distance out from the coverage (dilation), then chamfer back in from
    // the dilation's complement (erosion): closing = cells deeper than the
    // radius inside the dilated set. A straight corridor edge is invariant
    // under closing, so this never pads the junction's convex outline.
    // Radius scaled to the junction: a residential crossing needs only a
    // small curb-return fillet, while a sprawling multi-arm junction
    // (Sunnyvale j59 spans ~150 m) has paved voids tens of metres wide
    // between its corridors that a fixed small radius can never span.
    // Genuinely open bays wider than ~2R stay ground either way.
    const float bbox_extent = std::max(max_x - min_x, max_y - min_y);
    const float close_radius = std::clamp(0.12f * bbox_extent, 8.0f, 20.0f); // m
    {
      const float r_cells = close_radius / kCell;
      const float kInf = 1e9f;
      std::vector<float> dist(static_cast<size_t>(ncx) * ncy, kInf);
      auto Chamfer = [&](std::vector<float> &d) {
        for (int cy = 0; cy < ncy; ++cy) {
          for (int cx = 0; cx < ncx; ++cx) {
            float v = d[CellId(cx, cy)];
            if (cx > 0) v = std::min(v, d[CellId(cx - 1, cy)] + 1.0f);
            if (cy > 0) v = std::min(v, d[CellId(cx, cy - 1)] + 1.0f);
            if (cx > 0 && cy > 0) v = std::min(v, d[CellId(cx - 1, cy - 1)] + 1.4142f);
            if (cx + 1 < ncx && cy > 0) v = std::min(v, d[CellId(cx + 1, cy - 1)] + 1.4142f);
            d[CellId(cx, cy)] = v;
          }
        }
        for (int cy = ncy - 1; cy >= 0; --cy) {
          for (int cx = ncx - 1; cx >= 0; --cx) {
            float v = d[CellId(cx, cy)];
            if (cx + 1 < ncx) v = std::min(v, d[CellId(cx + 1, cy)] + 1.0f);
            if (cy + 1 < ncy) v = std::min(v, d[CellId(cx, cy + 1)] + 1.0f);
            if (cx + 1 < ncx && cy + 1 < ncy) v = std::min(v, d[CellId(cx + 1, cy + 1)] + 1.4142f);
            if (cx > 0 && cy + 1 < ncy) v = std::min(v, d[CellId(cx - 1, cy + 1)] + 1.4142f);
            d[CellId(cx, cy)] = v;
          }
        }
      };
      for (size_t i = 0; i < full.size(); ++i) {
        if (full[i]) dist[i] = 0.0f;
      }
      Chamfer(dist);
      std::vector<float> dist2(static_cast<size_t>(ncx) * ncy, kInf);
      for (size_t i = 0; i < dist.size(); ++i) {
        if (dist[i] > r_cells) dist2[i] = 0.0f; // complement of the dilation
      }
      Chamfer(dist2);
      for (size_t i = 0; i < dist2.size(); ++i) {
        // Inside the closing and not already coverage or an arm cap: pave.
        // Marking it blocked keeps the outside flood from running through
        // it, so enclosed-hole detection composes with the closing.
        if (dist2[i] > r_cells && !blocked[i]) {
          blocked[i] = 2; // 2 = closing fill (any non-zero blocks the flood)
        }
      }
    }

    // flood the open cells from the grid border
    std::vector<uint8_t> outside(static_cast<size_t>(ncx) * ncy, 0);
    std::vector<std::pair<int, int>> stack;
    auto Seed = [&](int cx, int cy) {
      const size_t id = CellId(cx, cy);
      if (!blocked[id] && !outside[id]) {
        outside[id] = 1;
        stack.emplace_back(cx, cy);
      }
    };
    for (int cx = 0; cx < ncx; ++cx) { Seed(cx, 0); Seed(cx, ncy - 1); }
    for (int cy = 0; cy < ncy; ++cy) { Seed(0, cy); Seed(ncx - 1, cy); }
    while (!stack.empty()) {
      const auto [cx, cy] = stack.back();
      stack.pop_back();
      if (cx > 0) Seed(cx - 1, cy);
      if (cx + 1 < ncx) Seed(cx + 1, cy);
      if (cy > 0) Seed(cx, cy - 1);
      if (cy + 1 < ncy) Seed(cx, cy + 1);
    }

    // z of a hole vertex: inverse-distance blend of the nearest corridor
    // vertices (ring search over the spatial hash)
    auto BlendZ = [&](float px, float py, bool &ok) {
      const int bx = BinX(px), by = BinY(py);
      double sum_w = 0.0, sum_z = 0.0;
      int found = 0;
      for (int r = 0; r <= 12 && (found < 4 || r == 0); ++r) {
        for (int y = by - r; y <= by + r; ++y) {
          if (y < 0 || y >= nby) continue;
          for (int x = bx - r; x <= bx + r; ++x) {
            if (x < 0 || x >= nbx) continue;
            if (std::abs(x - bx) != r && std::abs(y - by) != r) continue; // ring only
            for (int vi : vertex_bins[static_cast<size_t>(y) * nbx + x]) {
              const Vec3 &v = verts[vi];
              const double dx = v.x - px, dy = v.y - py;
              const double w = 1.0 / (dx * dx + dy * dy + 1e-2);
              sum_w += w;
              sum_z += w * v.z;
              ++found;
            }
          }
        }
      }
      ok = found > 0;
      return ok ? static_cast<float>(sum_z / sum_w) : 0.0f;
    };

    // -- emit ---------------------------------------------------------------
    geom::Mesh out_mesh;
    std::vector<size_t> mesh_index(static_cast<size_t>(nvx) * nvy, 0); // 1-based, 0 = unset
    auto VertexIndex = [&](int vx, int vy, bool &ok) -> size_t {
      const size_t vid = static_cast<size_t>(vy) * nvx + vx;
      if (mesh_index[vid] != 0) {
        ok = true;
        return mesh_index[vid];
      }
      const float px = origin_x + vx * kCell;
      const float py = origin_y + vy * kCell;
      float z;
      if (v_corridor[vid]) {
        z = v_z[vid];
        ok = true;
      } else {
        z = BlendZ(px, py, ok);
      }
      if (!ok) {
        return 0;
      }
      out_mesh.AddVertex(Vec3(px, py, z - kZBelow));
      mesh_index[vid] = out_mesh.GetVerticesNum();
      return mesh_index[vid];
    };

    size_t filled_cells = 0;
    out_mesh.AddMaterial("road");
    for (int cy = 0; cy < ncy; ++cy) {
      for (int cx = 0; cx < ncx; ++cx) {
        const size_t id = CellId(cx, cy);
        // Fill: closing cells (blocked == 2) and enclosed holes (open cells
        // the outside flood never reached). Never corridor-covered cells or
        // arm caps.
        const bool is_closing_fill = (blocked[id] == 2);
        const bool is_hole = (!blocked[id] && !outside[id]);
        if (!is_closing_fill && !is_hole) {
          continue;
        }
        bool ok0, ok1, ok2, ok3;
        const size_t i00 = VertexIndex(cx, cy, ok0);
        const size_t i10 = VertexIndex(cx + 1, cy, ok1);
        const size_t i11 = VertexIndex(cx + 1, cy + 1, ok2);
        const size_t i01 = VertexIndex(cx, cy + 1, ok3);
        if (!(ok0 && ok1 && ok2 && ok3)) {
          continue;
        }
        // Same winding as MeshFactory's lane strips (clockwise in the
        // stored x/y frame) so the UE side culls consistently.
        out_mesh.AddIndex(i00); out_mesh.AddIndex(i01); out_mesh.AddIndex(i11);
        out_mesh.AddIndex(i00); out_mesh.AddIndex(i11); out_mesh.AddIndex(i10);
        ++filled_cells;
      }
    }
    out_mesh.EndMaterial();

#ifdef LIBCARLA_JFILL_DEBUG
    std::cout << "[jfill] junction " << junction.GetId() << " grid " << nvx << "x" << nvy
              << " corridors " << corridor_tris.size() << " caps " << cap_tris.size()
              << " filled " << filled_cells << std::endl;
#endif
    if (filled_cells == 0) {
      return nullptr;
    }
    return std::make_unique<geom::Mesh>(std::move(out_mesh));
  }

  std::unique_ptr<geom::Mesh> Map::SDFToMesh(const road::Junction& jinput,
    const std::vector<geom::Vector3D>& /*sdfinput*/,
    int /*grid_cells_per_dim*/) const {

    float box_extraextension_factor = 1.2f;
    const double CubeSize = 0.5;
    carla::geom::BoundingBox bb = jinput.GetBoundingBox();
    carla::geom::Vector3D MinOffset = bb.location - geom::Location(bb.extent * box_extraextension_factor);
    auto junctionsdf = [this, CubeSize](MeshReconstruction::Vec3 const& pos)
    {
      geom::Vector3D worldloc(static_cast<float>(pos.x), static_cast<float>(pos.y), static_cast<float>(pos.z));
      std::optional<element::Waypoint> CheckingWaypoint = GetWaypoint(geom::Location(worldloc), 0x1 << 1);
      if (CheckingWaypoint) {
        if ( pos.z < 0.2) {
          return 0.0;
        } else {
          return -abs(pos.z);
        }
      }
      std::optional<element::Waypoint> InRoadWaypoint = GetClosestWaypointOnRoad(geom::Location(worldloc), 0x1 << 1);
      geom::Transform InRoadWPTransform = ComputeTransform(*InRoadWaypoint);

      geom::Vector3D director = geom::Location(worldloc) - (InRoadWPTransform.location);
      geom::Vector3D laneborder = InRoadWPTransform.location + geom::Location(director.MakeUnitVector() * static_cast<float>(GetLaneWidth(*InRoadWaypoint) * 0.5));

      geom::Vector3D Distance = laneborder - worldloc;
      if (Distance.Length2D() < CubeSize * 1.1 && pos.z < 0.2) {
        return 0.0;
      }
      return Distance.Length() * -1.0;
    };

    MeshReconstruction::Rect3 domain;
    domain.min = { MinOffset.x, MinOffset.y, MinOffset.z };
    domain.size = { bb.extent.x * box_extraextension_factor * 2, bb.extent.y * box_extraextension_factor * 2, 0.4 };

    MeshReconstruction::Vec3 cubeSize{ CubeSize, CubeSize, 0.2 };
    auto mesh = MeshReconstruction::MarchCube(junctionsdf, domain, cubeSize );
    geom::Mesh out_mesh;

    for (auto& cv : mesh.vertices) {
      geom::Vector3D newvertex;
      newvertex.x = static_cast<float>(cv.x);
      newvertex.y = static_cast<float>(cv.y);
      newvertex.z = static_cast<float>(cv.z);
      out_mesh.AddVertex(newvertex);
    }

    auto finalvertices = out_mesh.GetVertices();
    for (auto ct : mesh.triangles) {
      out_mesh.AddIndex(static_cast<size_t>(ct[1] + 1));
      out_mesh.AddIndex(static_cast<size_t>(ct[0] + 1));
      out_mesh.AddIndex(static_cast<size_t>(ct[2] + 1));
    }

    for (auto& cv : out_mesh.GetVertices() ) {
      std::optional<element::Waypoint> CheckingWaypoint = GetWaypoint(geom::Location(cv), 0x1 << 1);
      if (!CheckingWaypoint)
      {
        std::optional<element::Waypoint> InRoadWaypoint = GetClosestWaypointOnRoad(geom::Location(cv), 0x1 << 1);
        geom::Transform InRoadWPTransform = ComputeTransform(*InRoadWaypoint);

        geom::Vector3D director = geom::Location(cv) - (InRoadWPTransform.location);
        geom::Vector3D laneborder = InRoadWPTransform.location + geom::Location(director.MakeUnitVector() * static_cast<float>(GetLaneWidth(*InRoadWaypoint) * 0.5));
        cv = laneborder;
      }
    }
    return std::make_unique<geom::Mesh>(out_mesh);
  }

  void Map::GenerateSingleJunction(const carla::geom::MeshFactory& mesh_factory,
      const JuncId Id,
      std::map<road::Lane::LaneType, std::vector<std::unique_ptr<geom::Mesh>>>*
      junction_out_mesh_list) const {

      const auto& junction = _data.GetJunctions().at(Id);
      if (junction.GetConnections().size() > 2) {
        std::vector<std::unique_ptr<geom::Mesh>> lane_meshes;
        std::vector<std::unique_ptr<geom::Mesh>> sidewalk_lane_meshes;
        std::vector<carla::geom::Vector3D> perimeterpoints;

        auto pmesh = SDFToMesh(junction, perimeterpoints, 75);
        (*junction_out_mesh_list)[road::Lane::LaneType::Driving].push_back(std::move(pmesh));

        for (const auto& connection_pair : junction.GetConnections()) {
          const auto& connection = connection_pair.second;
          const auto& road = _data.GetRoads().at(connection.connecting_road);
          for (auto&& lane_section : road.GetLaneSections()) {
            for (auto&& lane_pair : lane_section.GetLanes()) {
              const auto& lane = lane_pair.second;
              if ( lane.GetType() == road::Lane::LaneType::Sidewalk ) {
                std::optional<element::Waypoint> sw =
                  GetWaypoint(road.GetId(), lane_pair.first, static_cast<float>(lane.GetDistance() + (lane.GetLength() * 0.5)));
                if (!GetWaypoint(ComputeTransform(*sw).location).has_value()){
                  sidewalk_lane_meshes.push_back(mesh_factory.GenerateSidewalk(lane));
                }
              }
            }
          }
        }
        std::unique_ptr<geom::Mesh> sidewalk_mesh = std::make_unique<geom::Mesh>();
        for (auto& lane : sidewalk_lane_meshes) {
          *sidewalk_mesh += *lane;
        }
        (*junction_out_mesh_list)[road::Lane::LaneType::Sidewalk].push_back(std::move(sidewalk_mesh));
      } else {
        std::vector<std::unique_ptr<geom::Mesh>> lane_meshes;
        std::vector<std::unique_ptr<geom::Mesh>> sidewalk_lane_meshes;
        for (const auto& connection_pair : junction.GetConnections()) {
          const auto& connection = connection_pair.second;
          const auto& road = _data.GetRoads().at(connection.connecting_road);
          for (auto&& lane_section : road.GetLaneSections()) {
            for (auto&& lane_pair : lane_section.GetLanes()) {
              const auto& lane = lane_pair.second;
              if (lane.GetType() != road::Lane::LaneType::Sidewalk) {
                lane_meshes.push_back(mesh_factory.GenerateTesselated(lane));
              }
              else {
                sidewalk_lane_meshes.push_back(mesh_factory.GenerateSidewalk(lane));
              }
            }
          }
        }
        std::unique_ptr<geom::Mesh> merged_mesh = std::make_unique<geom::Mesh>();
        for (auto& lane : lane_meshes) {
          *merged_mesh += *lane;
        }
        std::unique_ptr<geom::Mesh> sidewalk_mesh = std::make_unique<geom::Mesh>();
        for (auto& lane : sidewalk_lane_meshes) {
          *sidewalk_mesh += *lane;
        }

        (*junction_out_mesh_list)[road::Lane::LaneType::Driving].push_back(std::move(merged_mesh));
        (*junction_out_mesh_list)[road::Lane::LaneType::Sidewalk].push_back(std::move(sidewalk_mesh));
      }
    }

} // namespace road
} // namespace carla
