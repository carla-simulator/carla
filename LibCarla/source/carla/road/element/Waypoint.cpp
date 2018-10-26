// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "carla/road/element/Waypoint.h"

#include "carla/road/Map.h"

namespace carla {
namespace road {
namespace element {

  Waypoint::Waypoint() {}

  Waypoint::Waypoint(SharedPtr<const Map> m, const geom::Location &loc)
    : _map(m) {

    _road_id = 0;
    _dist = 0;

    double nearest_dist = std::numeric_limits<double>::max();
    for (auto &&r : _map->GetData()._elements) {
      auto current_dist = r.second->GetNearestPoint(loc);
      if (current_dist.second < nearest_dist) {
        nearest_dist = current_dist.second;
        _road_id = r.first;
        _dist = current_dist.first;
      }
    }

    assert(_dist <= _map->GetData().GetRoad(_road_id)->GetLength());

    //
    const geom::Location loc_in_geom =
        _map->GetData().GetRoad(_road_id)->GetDirectedPointIn(_dist).location;
    const double heading =
         _map->GetData().GetRoad(_road_id)->GetDirectedPointIn(_dist).tangent;

    const geom::Rotation rot(0.0, heading * geom::Math::to_radiants(), 0.0);
    _transform = geom::Transform(loc_in_geom, rot);

    _lane_id = _map->GetData().GetRoad(_road_id)->GetNearestLane(_dist, loc);
  }

  RoadInfoList Waypoint::GetRoadInfo() const {
    return RoadInfoList(_map->GetData().GetRoad(_road_id)->GetInfos(_dist));
  }

  Waypoint::~Waypoint() = default;

} // namespace element
} // namespace road
} // namespace carla
