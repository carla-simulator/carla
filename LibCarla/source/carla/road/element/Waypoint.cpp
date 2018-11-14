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
    DEBUG_ASSERT(_lane_id != 0);
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
    DEBUG_ASSERT(_lane_id != 0);
  }

  Waypoint::~Waypoint() = default;

  geom::Transform Waypoint::GetTransform() const {
    road::element::DirectedPoint dp =
        _map->GetData().GetRoad(_road_id)->GetDirectedPointIn(_dist);

    geom::Rotation rot(0.0, geom::Math::to_degrees(dp.tangent), 0.0);
    if (_lane_id > 0) {
      rot.yaw += 180.0;
    }

    const auto *road_segment = _map->GetData().GetRoad(_road_id);
    DEBUG_ASSERT(road_segment != nullptr);
    const auto *info = road_segment->GetInfo<RoadInfoLane>(0.0);
    DEBUG_ASSERT(info != nullptr);

    dp.ApplyLateralOffset(info->getLane(_lane_id)->_lane_center_offset);
    return geom::Transform(dp.location, rot);
  }

  RoadInfoList Waypoint::GetRoadInfo() const {
    return RoadInfoList(_map->GetData().GetRoad(_road_id)->GetInfos(_dist));
  }

} // namespace element
} // namespace road
} // namespace carla
