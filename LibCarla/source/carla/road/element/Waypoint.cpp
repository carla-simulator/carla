// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "carla/road/element/Waypoint.h"
#include "carla/Logging.h"
#include "carla/road/Map.h"

#include <algorithm>

namespace carla {
namespace road {
namespace element {

  // ===========================================================================
  // -- Static local methods ---------------------------------------------------
  // ===========================================================================

  static const RoadSegment &GetRoad(const Map &map, id_type road_id) {
    auto *road = map.GetData().GetRoad(road_id);
    DEBUG_ASSERT(road != nullptr);
    return *road;
  }

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

  // ===========================================================================
  // -- Waypoint ---------------------------------------------------------------
  // ===========================================================================

  Waypoint::Waypoint(SharedPtr<const Map> m, const geom::Location &loc)
    : _map(m) {
    DEBUG_ASSERT(_map != nullptr);
    // max_nearests represents the max nearests roads
    // where we will search for nearests lanes
    constexpr int max_nearests = 10;
    // in case that map has less than max_nearests lanes,
    // we will use the maximum lanes
    const int max_nearest_allowed = _map->GetData().GetRoadCount()  <
        max_nearests ? _map->GetData().GetRoadCount() : max_nearests;

    double nearest_dist[max_nearests];
    std::fill(nearest_dist, nearest_dist + max_nearest_allowed,
        std::numeric_limits<double>::max());

    id_type ids[max_nearests];
    std::fill(ids, ids + max_nearest_allowed, 0);

    double dists[max_nearests];
    std::fill(dists, dists + max_nearest_allowed, 0.0);


    for (auto &&r : _map->GetData().GetRoadSegments()) {
      auto current_dist = r.GetNearestPoint(loc);

      // search for nearests points
      for (int i = 0; i < max_nearest_allowed; ++i) {
        if (current_dist.second < nearest_dist[i]) {
          // reorder nearest_dist
          for (int j = max_nearest_allowed - 1; j > i; --j) {
            nearest_dist[j] = nearest_dist[j - 1];
            ids[j] = ids[j - 1];
            dists[j] = dists[j - 1];
          }
          nearest_dist[i] = current_dist.second;
          ids[i] = r.GetId();
          dists[i] = current_dist.first;

          break;
        }
      }
    }

    // search for the nearest lane in nearest_dist
    auto nearest_lane_dist = std::numeric_limits<double>::max();
    for (int i = 0; i < max_nearest_allowed; ++i) {
      auto lane_dist = _map->GetData().GetRoad(ids[i])->GetNearestLane(dists[i], loc);

      if (lane_dist.second < nearest_lane_dist) {
        nearest_lane_dist = lane_dist.second;
        _lane_id = lane_dist.first;
        _road_id = ids[i];
        _dist = dists[i];
      }
    }

    DEBUG_ASSERT(_dist <= _map->GetData().GetRoad(_road_id)->GetLength());
  }

  Waypoint::Waypoint(
      SharedPtr<const Map> map,
      id_type road_id,
      id_type lane_id,
      double distance)
    : _map(std::move(map)),
      _road_id(road_id),
      _lane_id(lane_id),
      _dist(distance) {
    DEBUG_ASSERT(_map != nullptr);
  }

  Waypoint::~Waypoint() = default;

  geom::Transform Waypoint::GetTransform() const {
    road::element::DirectedPoint dp =
        _map->GetData().GetRoad(_road_id)->GetDirectedPointIn(_dist);

    geom::Rotation rot(0.0, geom::Math::to_degrees(dp.tangent), 0.0);
    if (_lane_id > 0) {
      rot.yaw += 180.0;
    }

    auto *info = GetRoad(*_map, _road_id).GetInfo<RoadInfoLane>(0.0);
    DEBUG_ASSERT(info != nullptr);

    dp.ApplyLateralOffset(info->getLane(_lane_id)->_lane_center_offset);
    return geom::Transform(dp.location, rot);
  }

  RoadInfoList Waypoint::GetRoadInfo() const {
    return RoadInfoList(_map->GetData().GetRoad(_road_id)->GetInfos(_dist));
  }

  std::vector<Waypoint> Waypoint::Next(const double distance) const {
    DEBUG_ASSERT(_lane_id != 0);

    const auto &road_segment = GetRoad(*_map, _road_id);
    double distance_on_next_segment;

    if (_lane_id < 0) {
      // road goes forward.
      const auto total_distance = _dist + distance;
      const auto road_length = road_segment.GetLength();
      if (total_distance <= road_length) {
        return { Waypoint(_map, _road_id, _lane_id, total_distance) };
      }
      distance_on_next_segment = total_distance - road_length;
    } else {
      // road goes backward.
      const auto total_distance = _dist - distance;
      if (total_distance >= 0.0) {
        return { Waypoint(_map, _road_id, _lane_id, total_distance) };
      }
      distance_on_next_segment = std::abs(total_distance);
    }

    std::vector<Waypoint> result;
    const auto &next_lanes = _lane_id < 0 ? road_segment.GetNextLane(_lane_id) : road_segment.GetPrevLane(
        _lane_id);
    if (next_lanes.empty()) {
      log_error("no lanes!! lane id =", _lane_id, " road id=", _road_id);
    }
    for (auto &&pair : next_lanes) {
      auto lane_id = pair.first;
      auto road_id = pair.second;
      DEBUG_ASSERT(lane_id != 0);
      auto d = lane_id < 0 ? 0.0 : GetRoad(*_map, road_id).GetLength();
      auto waypoint = Waypoint(_map, road_id, lane_id, d);
      result = ConcatVectors(result, waypoint.Next(distance_on_next_segment));
    }
    return result;
  }

} // namespace element
} // namespace road
} // namespace carla
