// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "carla/geom/Math.h"
#include "carla/road/Map.h"
#include "carla/road/element/Types.h"

#include <limits>


namespace carla {
namespace road {

  using namespace element;

  Waypoint Map::GetClosestWaypointOnRoad(const geom::Location &loc) const {
    return Waypoint(shared_from_this(), loc);
  }

  Optional<Waypoint> Map::GetWaypoint(const geom::Location &loc) const {
    Waypoint w = Waypoint(shared_from_this(), loc);
    auto d = geom::Math::Distance2D(w.GetTransform().location, loc);
    const RoadInfoLane *inf = _data.GetRoad(w._road_id)->GetInfo<RoadInfoLane>(w._dist);

    if (d < inf->getLane(w._lane_id)->_width * 0.5) {
      return Optional<Waypoint>(w);
    }

    return Optional<Waypoint>();
  }

  const MapData &Map::GetData() const {
    return _data;
  }

} // namespace road
} // namespace carla
