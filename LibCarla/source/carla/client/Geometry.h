// Copyright (c) 2020 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/Memory.h"
#include "carla/NonCopyable.h"

#include "carla/road/RoadTypes.h"
#include "carla/road/element/Geometry.h"

namespace carla {
namespace road {
  class Road;
  namespace element {
    struct Waypoint;
  } // namespace element
} // namespace road

namespace client {

  class Map;
  class Waypoint;
  class RoadGeometries;

  class Geometry : public EnableSharedFromThis<Geometry>, private NonCopyable {
  public:
    road::element::GeometryType GetType() const;
    double GetCurvatureFromWaypoint(const Waypoint& w) const;
    double GetCurvature(double dist) const;
    double GetLength() const;
    double GetStartOffset() const;
    double GetHeading() const;

  private:
    friend class RoadGeometries;
    Geometry(SharedPtr<const Map> parent, const road::Road& road, const road::element::Geometry& geo);

  private:
    SharedPtr<const Map> _parent;
    const road::Road& _road;
    const road::element::Geometry& _geometry;
  };

  class RoadGeometries : public EnableSharedFromThis<RoadGeometries>, private NonCopyable {
  public:
      double GetCurvatureFromWaypoint(const Waypoint& w) const;
      double GetCurvature(double dist) const;

      road::RoadId GetRoadId() const;

      SharedPtr<Geometry> GetGeometryFromWaypoint(const Waypoint& w) const;
      SharedPtr<Geometry> GetGeometry(double dist) const;

    private:
      friend class Waypoint;
      RoadGeometries(SharedPtr<const Map> parent, const road::element::Waypoint& waypoint);

      const road::element::Geometry& GeometryAt(const Waypoint& waypoint) const;
      const road::element::Geometry& GeometryAt(double distance) const;

    private:
      SharedPtr<const Map> _parent;
      const road::Road& _road;
  };
}
} // namespace carla
