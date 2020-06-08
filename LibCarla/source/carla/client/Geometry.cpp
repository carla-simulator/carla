// Copyright (c) 2020 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "carla/client/Geometry.h"

#include "carla/road/element/Waypoint.h"
#include "carla/road/element/Geometry.h"
#include "carla/road/element/RoadInfoGeometry.h"
#include "carla/client/Map.h"

namespace carla {
namespace client {

  static const road::element::Geometry& GeometryAt(const Map& map, const road::element::Waypoint& waypoint) {
    const auto &lane = map.GetMap().GetLane(waypoint);
    const auto &road = lane.GetRoad();
    const auto& geom = road->GetInfo<road::element::RoadInfoGeometry>(waypoint.s);
    return geom->GetGeometry();
  }

  Geometry::Geometry(SharedPtr<const Map> parent, const road::element::Waypoint& waypoint)
    : _parent(std::move(parent)), _geometry(GeometryAt(*_parent, waypoint)) {}

} // namespace client
} // namespace carla
