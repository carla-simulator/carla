// Copyright (c) 2020 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "carla/client/Geometry.h"
#include "carla/client/Waypoint.h"

#include "carla/client/Map.h"
#include "carla/road/element/Geometry.h"
#include "carla/road/element/RoadInfoGeometry.h"
#include "carla/road/element/Waypoint.h"

namespace carla {
namespace client {

  Geometry::Geometry(SharedPtr<const Map> parent, const road::Road& road, const road::element::Geometry& geo)
    : _parent(std::move(parent)), _road(road), _geometry(geo) {}

  road::element::GeometryType Geometry::GetType() const {
    return _geometry.GetType();
  }
  double Geometry::GetCurvatureFromWaypoint(const Waypoint& w) const {
    if (_road.GetId() != w.GetRoadId()) {
      throw std::runtime_error("waypoint is not on the road");
    }
    return GetCurvature(w.GetDistance());
  }
  double Geometry::GetCurvature(double dist) const {
    dist = dist - _geometry.GetStartOffset();
    if (dist < 0 || dist > _geometry.GetLength()) {
      throw std::out_of_range("distance is out of range for the current geometry");
    }
    return _geometry.GetCurvature(dist - _geometry.GetStartOffset());
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

  RoadGeometries::RoadGeometries(SharedPtr<const Map> parent, const road::element::Waypoint& waypoint)
    : _parent(std::move(parent)), _road(*_parent->GetMap().GetLane(waypoint).GetRoad()) {}

  road::RoadId RoadGeometries::GetRoadId() const {
    return _road.GetId();
  }

  SharedPtr<Geometry> RoadGeometries::GetGeometryFromWaypoint(const Waypoint& w) const {
    auto& geo = GeometryAt(w);
    return SharedPtr<Geometry>(new Geometry(_parent, _road, geo));
  }

  SharedPtr<Geometry> RoadGeometries::GetGeometry(double dist) const {
    auto& geo = GeometryAt(dist);
    return SharedPtr<Geometry>(new Geometry(_parent, _road, geo));
  }

  double RoadGeometries::GetCurvatureFromWaypoint(const Waypoint& w) const {
    auto& geom = GeometryAt(w);
    auto s = w.GetDistance() - geom.GetStartOffset();
    return GetCurvature(s);
  }

  const road::element::Geometry& RoadGeometries::GeometryAt(const Waypoint& waypoint) const {
    if (_road.GetId() != waypoint.GetRoadId()) {
      throw std::runtime_error("waypoint is not on the road");
    }
    auto geom = _road.GetInfo<road::element::RoadInfoGeometry>(waypoint.GetDistance());
    return geom->GetGeometry();
  }

  const road::element::Geometry& RoadGeometries::GeometryAt(double distance) const {
    auto geom = _road.GetInfo<road::element::RoadInfoGeometry>(distance);
    if (!geom)
      throw std::out_of_range("distance is out of range for the current geometry");
    return geom->GetGeometry();
  }

  double RoadGeometries::GetCurvature(double dist) const {
    const auto geom = _road.GetInfo<road::element::RoadInfoGeometry>(dist);
    if (!geom) throw std::out_of_range("distance is out of range for the current geometry");
    const auto& g = geom->GetGeometry();
    return g.GetCurvature(dist - g.GetStartOffset());
  }

} // namespace client
} // namespace carla
