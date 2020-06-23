// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include <carla/client/Geometry.h>
#include <ostream>

namespace carla {
namespace road {
namespace element {
  std::ostream &operator<<(std::ostream &out, const GeometryType &tp) {
    switch(tp)
    {
      case GeometryType::LINE:
          return out << "LINE";
      case GeometryType::ARC:
          return out << "ARC";
      case GeometryType::SPIRAL:
          return out << "SPIRAL";
      case GeometryType::POLY3:
          return out << "POLY3";
      case GeometryType::POLY3PARAM:
          return out << "POLY3PARAM";
    }

    throw std::runtime_error("Missing enum case");
  }
} // namespace element
} // namespace road

namespace client {

  std::ostream &operator<<(std::ostream &out, const Geometry &g) {
    return out << "Geometry(type=" << g.GetType() << ")";
  }

  std::ostream &operator<<(std::ostream &out, const RoadGeometries &rg) {
    return out << "RoadGeometries(road_id=" << rg.GetRoadId() << ")";
  }

} // namespace geom
} // namespace carla

void export_geometry() {
  using namespace boost::python;
  namespace cc = carla::client;
  namespace cre = carla::road::element;

  enum_<cre::GeometryType>("GeometryType")
      .value("Line", cre::GeometryType::LINE) // None is reserved in Python3
      .value("Arc", cre::GeometryType::ARC)
      .value("Spiral", cre::GeometryType::SPIRAL)
      .value("Poly3", cre::GeometryType::POLY3)
      .value("Poly3Param", cre::GeometryType::POLY3PARAM)

      ;

  class_<cc::Geometry, boost::noncopyable, boost::shared_ptr<cc::Geometry>>("Geometry", no_init)
      .def("get_type", &cc::Geometry::GetType)
      .def("get_curvature", &cc::Geometry::GetCurvature)
      .def("get_curvature_from_waypoint", &cc::Geometry::GetCurvatureFromWaypoint)
      .def("get_length", &cc::Geometry::GetLength)
      .def("get_start_offset", &cc::Geometry::GetStartOffset)
      .def("get_heading", &cc::Geometry::GetHeading)
      .def(self_ns::str(self_ns::self))

      ;

  class_<cc::RoadGeometries, boost::noncopyable, boost::shared_ptr<cc::RoadGeometries>>("RoadGeometries", no_init)
      //.def("get_type", &cc::RoadGeometries::GetType)
      .def("get_curvature", &cc::RoadGeometries::GetCurvature)
      .def("get_curvature_from_waypoint", &cc::RoadGeometries::GetCurvatureFromWaypoint)
      .def("get_geometry_from_waypoint", &cc::RoadGeometries::GetGeometryFromWaypoint)
      .def("get_geometry_at_distance", &cc::RoadGeometries::GetGeometry)
      .def(self_ns::str(self_ns::self))

      ;
}
