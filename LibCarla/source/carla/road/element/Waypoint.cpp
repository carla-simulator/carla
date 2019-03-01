// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "carla/road/element/Waypoint.h"
#include "carla/Logging.h"
#include "carla/road/Map.h"
#include "carla/geom/CubicPolynomial.h"
#include "carla/geom/Math.h"

#include <unordered_map>
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

  geom::Transform Waypoint::ComputeTransform() const {
    const auto road_segment = _map->GetData().GetRoad(_road_id);
    DEBUG_ASSERT(road_segment != nullptr);

    road::element::DirectedPoint dp = road_segment->GetDirectedPointIn(_dist);

    geom::Rotation rot(geom::Math::to_degrees(dp.pitch), geom::Math::to_degrees(dp.tangent), 0.0);
    if (_lane_id > 0) {
      rot.yaw += 180.0;
      rot.pitch = 360 - rot.pitch;
    }

    if (IsIntersection()) {
      // @todo: fix intersection lane_id to allow compute the lane distance in a correct way
      // old way to calculate lane position
      const auto *road_segment = _map->GetData().GetRoad(_road_id);
      DEBUG_ASSERT(road_segment != nullptr);
      const auto info = road_segment->GetInfo<RoadInfoLane>(0.0);
      DEBUG_ASSERT(info != nullptr);

      dp.ApplyLateralOffset(info->getLane(_lane_id)->_lane_center_offset);
    } else {
      // new way to calculate lane position
      const auto lane_offset_info = road_segment->GetInfo<RoadInfoLaneOffset>(_dist);
      geom::CubicPolynomial final_polynomial = lane_offset_info->GetPolynomial();

      // fill a map (lane_info_map) with first lane id <id, RoadInfoLaneWidth> found
      // because will be the nearest one that is affecting at this dist (t)
      const auto lane_width_info = road_segment->GetInfos<RoadInfoLaneWidth>(_dist);
      std::unordered_map<int, std::shared_ptr<const RoadInfoLaneWidth>> lane_info_map;
      std::unordered_set<int> inserted_lanes;

      for (auto &&lane_offset : lane_width_info) {
        const int current_lane_id = lane_offset->GetLaneId();
        lane_info_map.emplace(
            std::pair<int, std::shared_ptr<const RoadInfoLaneWidth>>(
              current_lane_id,
              lane_offset));
      }

      DEBUG_ASSERT(_lane_id != 0);

      // iterate over the previous lanes until lane_id is 0 and add the polynomial info
      const int inc = _lane_id < 0 ? - 1 : + 1;
      // increase or decrease the lane_id depending on if _lane_id is
      // positive or negative in order to get closer to that id
      for (int lane_id = inc; lane_id != _lane_id; lane_id += inc) {
        // final_polynomial += geom::CubicPolynomial();
        final_polynomial += lane_info_map[lane_id]->GetPolynomial() * (_lane_id < 0 ? -1.0 : 1.0);
      }

      // use half of the last polynomial to get the center of the road
      final_polynomial += lane_info_map[_lane_id]->GetPolynomial() * (_lane_id < 0 ? -0.5 : 0.5);

      // compute the final lane offset
      dp.ApplyLateralOffset(final_polynomial.Evaluate(_dist));

      const auto tangent = geom::Math::to_degrees(final_polynomial.Tangent(_dist));
      rot.yaw += _lane_id < 0 ? -tangent : tangent;
    }

    return geom::Transform(dp.location, rot);
  }

  const std::string &Waypoint::GetType() const {
    return _map->GetData().GetRoad(_road_id)->GetInfo<RoadInfoLane>(_dist)->getLane(_lane_id)->_type;
  }

  const RoadSegment &Waypoint::GetRoadSegment() const {
    const auto *road_segment = _map->GetData().GetRoad(_road_id);
    DEBUG_ASSERT(road_segment != nullptr);
    return *road_segment;
  }

  bool Waypoint::IsIntersection() const {
    const auto info = GetRoadSegment().GetInfo<RoadGeneralInfo>(_dist);
    return info != nullptr ? info->IsJunction() : false;
  }

  double Waypoint::GetLaneWidth() const {
    const auto *road_segment = _map->GetData().GetRoad(_road_id);
    const auto lane_width_info = road_segment->GetInfos<RoadInfoLaneWidth>(_dist);
    for (auto &&lane : lane_width_info) {
      if (lane->GetLaneId() == _lane_id) {
        return lane->GetPolynomial().Evaluate(_dist);
      }
    }
    return 0.0;
  }

  std::pair<RoadInfoMarkRecord, RoadInfoMarkRecord> Waypoint::GetMarkRecord() const {
    const auto lane_id_right = _lane_id;
    // If the lane is bigger than 0, is a backward lane,
    // so the inner lane marking is the opposite one
    const auto lane_id_left = _lane_id <= 0 ? _lane_id + 1 : _lane_id - 1;

    const auto mark_record_list = GetRoadSegment().GetRoadInfoMarkRecord(_dist);

    std::pair<RoadInfoMarkRecord, RoadInfoMarkRecord> result = std::make_pair(
        RoadInfoMarkRecord(_dist, lane_id_right),
        RoadInfoMarkRecord(_dist, lane_id_left));

    for (auto &&i : mark_record_list) {
      if (i->GetLaneId() == lane_id_right) {
        result.first = *i;
      } else if (i->GetLaneId() == lane_id_left) {
        result.second = *i;
      }
    }

    return result;
  }

} // namespace element
} // namespace road
} // namespace carla
