// Copyright (c) 2020 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "carla/client/Geometry.h"
#include "carla/client/Waypoint.h"

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

  road::element::GeometryType Geometry::GetType() const {
    return _geometry.GetType();
  }

  double Geometry::GetCurvatureFromWaypoint(const Waypoint& w) const {
    auto& other_geometry = GeometryAt(*_parent, {
                                                    w.GetRoadId(),
                                                    w.GetSectionId(),
                                                    w.GetLaneId(),
                                                    w.GetDistance(),
                                                });
    if (std::addressof(other_geometry) != std::addressof(_geometry)) {
      throw std::runtime_error("Waypoint is on another geometry");
    }

    auto s = w.GetDistance() - _geometry.GetStartOffset();
    return GetCurvature(s);
  }

  double Geometry::GetCurvature(double dist) const {
    if (dist < 0 || dist > _geometry.GetLength()) {
      throw std::invalid_argument("dist must be within the geometry length");
    }
    return _geometry.GetCurvature(dist);
  }

  double Geometry::GetLength() const {
    return _geometry.GetLength();
  }

  double Geometry::GetStartOffset() const {
    return _geometry.GetStartOffset();
  }

  double Geometry::GetHeading() const {
    return _geometry.GetHeading();
  }

} // namespace client
} // namespace carla
