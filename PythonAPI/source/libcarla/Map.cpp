// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include <carla/FileSystem.h>
#include <carla/PythonUtil.h>
#include <carla/client/Map.h>
#include <carla/client/Waypoint.h>
#include "carla/road/element/WaypointInformationTypes.h"
#include <fstream>

namespace carla {
namespace client {

  std::ostream &operator<<(std::ostream &out, const Map &map) {
    out << "Map(name=" << map.GetName() << ')';
    return out;
  }

  std::ostream &operator<<(std::ostream &out, const Waypoint &waypoint) {
    out << "Waypoint(" << waypoint.GetTransform() << ')';
    return out;
  }

} // namespace client
} // namespace carla

static void SaveOpenDriveToDisk(const carla::client::Map &self, std::string path) {
  carla::PythonUtil::ReleaseGIL unlock;
  if (path.empty()) {
    path = self.GetName();
  }
  carla::FileSystem::ValidateFilePath(path, ".xodr");
  std::ofstream out(path);
  out << self.GetOpenDrive() << std::endl;
}

static auto GetTopology(const carla::client::Map &self) {
  namespace py = boost::python;
  auto topology = self.GetTopology();
  py::list result;
  for (auto &&pair : topology) {
    result.append(py::make_tuple(pair.first, pair.second));
  }
  return result;
}

static carla::geom::GeoLocation ToGeolocation(
    const carla::client::Map &self,
    const carla::geom::Location &location) {
  return self.GetGeoReference().Transform(location);
}

template <typename T>
static boost::python::object OptionalToPythonObject(const T opt) {
  namespace py = boost::python;
  if (opt.has_value()) {
    return py::object(opt.value());
  }
  return py::object();
}

static boost::python::object MakeRightWaypointInfoRoadMark(
    const carla::client::Waypoint &self) {
  return OptionalToPythonObject(self.GetRightRoadMark());
}

static boost::python::object MakeLeftWaypointInfoRoadMark(
    const carla::client::Waypoint &self) {
  return OptionalToPythonObject(self.GetLeftRoadMark());
}

void export_map() {
  using namespace boost::python;
  namespace cc = carla::client;
  namespace cr = carla::road;
  namespace cg = carla::geom;
  namespace cre = carla::road::element;

  enum_<cr::Lane::LaneType>("LaneType")
    .value("NONE", cr::Lane::LaneType::None) // None is reserved in Python3
    .value("Driving", cr::Lane::LaneType::Driving)
    .value("Stop", cr::Lane::LaneType::Stop)
    .value("Shoulder", cr::Lane::LaneType::Shoulder)
    .value("Biking", cr::Lane::LaneType::Biking)
    .value("Sidewalk", cr::Lane::LaneType::Sidewalk)
    .value("Border", cr::Lane::LaneType::Border)
    .value("Restricted", cr::Lane::LaneType::Restricted)
    .value("Parking", cr::Lane::LaneType::Parking)
    .value("Bidirectional", cr::Lane::LaneType::Bidirectional)
    .value("Median", cr::Lane::LaneType::Median)
    .value("Special1", cr::Lane::LaneType::Special1)
    .value("Special2", cr::Lane::LaneType::Special2)
    .value("Special3", cr::Lane::LaneType::Special3)
    .value("RoadWorks", cr::Lane::LaneType::RoadWorks)
    .value("Tram", cr::Lane::LaneType::Tram)
    .value("Rail", cr::Lane::LaneType::Rail)
    .value("Entry", cr::Lane::LaneType::Entry)
    .value("Exit", cr::Lane::LaneType::Exit)
    .value("OffRamp", cr::Lane::LaneType::OffRamp)
    .value("OnRamp", cr::Lane::LaneType::OnRamp)
    .value("Any", cr::Lane::LaneType::Any)
  ;

  class_<cc::Map, boost::noncopyable, boost::shared_ptr<cc::Map>>("Map", no_init)
    .def(init<std::string, std::string>((arg("name"), arg("xodr_content"))))
    .add_property("name", CALL_RETURNING_COPY(cc::Map, GetName))
    .def("get_spawn_points", CALL_RETURNING_LIST(cc::Map, GetRecommendedSpawnPoints))
    .def("get_waypoint", &cc::Map::GetWaypoint, (arg("location"), arg("project_to_road")=true, arg("lane_type")=cr::Lane::LaneType::Driving))
    .def("get_topology", &GetTopology)
    .def("generate_waypoints", CALL_RETURNING_LIST_1(cc::Map, GenerateWaypoints, double), (args("distance")))
    .def("transform_to_geolocation", &ToGeolocation, (arg("location")))
    .def("to_opendrive", CALL_RETURNING_COPY(cc::Map, GetOpenDrive))
    .def("save_to_disk", &SaveOpenDriveToDisk, (arg("path")=""))
    .def(self_ns::str(self_ns::self))
  ;

  enum_<cre::WaypointInfoRoadMark::LaneChange>("LaneChange")
    .value("NONE", cre::WaypointInfoRoadMark::LaneChange::None) // None is reserved in Python3
    .value("Right", cre::WaypointInfoRoadMark::LaneChange::Right)
    .value("Left", cre::WaypointInfoRoadMark::LaneChange::Left)
    .value("Both", cre::WaypointInfoRoadMark::LaneChange::Both)
  ;

  enum_<cre::WaypointInfoRoadMark::Color>("RoadMarkColor")
    .value("Standard", cre::WaypointInfoRoadMark::Color::Standard)
    .value("Blue", cre::WaypointInfoRoadMark::Color::Blue)
    .value("Green", cre::WaypointInfoRoadMark::Color::Green)
    .value("Red", cre::WaypointInfoRoadMark::Color::Red)
    .value("White", cre::WaypointInfoRoadMark::Color::White)
    .value("Yellow", cre::WaypointInfoRoadMark::Color::Yellow)
    .value("Other", cre::WaypointInfoRoadMark::Color::Other)
  ;

  enum_<cre::LaneMarking>("LaneMarking")
    .value("NONE", cre::LaneMarking::None) // None is reserved in Python3
    .value("Other", cre::LaneMarking::Other)
    .value("Broken", cre::LaneMarking::Broken)
    .value("Solid", cre::LaneMarking::Solid)
    .value("SolidSolid", cre::LaneMarking::SolidSolid)
    .value("SolidBroken", cre::LaneMarking::SolidBroken)
    .value("BrokenSolid", cre::LaneMarking::BrokenSolid)
    .value("BrokenBroken", cre::LaneMarking::BrokenBroken)
    .value("BottsDots", cre::LaneMarking::BottsDots)
    .value("Grass", cre::LaneMarking::Grass)
    .value("Curb", cre::LaneMarking::Curb)
  ;

  class_<cre::WaypointInfoRoadMark>("WaypointInfoRoadMark", no_init)
    .add_property("type", &cre::WaypointInfoRoadMark::type)
    .add_property("color", &cre::WaypointInfoRoadMark::color)
    .add_property("lane_change", &cre::WaypointInfoRoadMark::lane_change)
    .add_property("width", &cre::WaypointInfoRoadMark::width)
  ;

  class_<cc::Waypoint, boost::noncopyable, boost::shared_ptr<cc::Waypoint>>("Waypoint", no_init)
    .add_property("id", &cc::Waypoint::GetId)
    .add_property("transform", CALL_RETURNING_COPY(cc::Waypoint, GetTransform))
    .add_property("is_intersection", &cc::Waypoint::IsIntersection)
    .add_property("lane_width", &cc::Waypoint::GetLaneWidth)
    .add_property("road_id", &cc::Waypoint::GetRoadId)
    .add_property("section_id", &cc::Waypoint::GetSectionId)
    .add_property("lane_id", &cc::Waypoint::GetLaneId)
    .add_property("s", &cc::Waypoint::GetDistance)
    .add_property("lane_change", &cc::Waypoint::GetLaneChange)
    .add_property("lane_type", &cc::Waypoint::GetType)
    .add_property("right_road_mark", &MakeRightWaypointInfoRoadMark)
    .add_property("left_road_mark", &MakeLeftWaypointInfoRoadMark)
    .def("next", CALL_RETURNING_LIST_1(cc::Waypoint, Next, double), (args("distance")))
    .def("get_right_lane", &cc::Waypoint::Right)
    .def("get_left_lane", &cc::Waypoint::Left)
    .def(self_ns::str(self_ns::self))
  ;
}
