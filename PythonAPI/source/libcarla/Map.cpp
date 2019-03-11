// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include <carla/FileSystem.h>
#include <carla/PythonUtil.h>
#include <carla/client/Map.h>
#include <carla/client/Waypoint.h>

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

void export_map() {
  using namespace boost::python;
  namespace cc = carla::client;
  namespace cg = carla::geom;

  class_<cc::Map, boost::noncopyable, boost::shared_ptr<cc::Map>>("Map", no_init)
    .add_property("name", CALL_RETURNING_COPY(cc::Map, GetName))
    .def("get_spawn_points", CALL_RETURNING_LIST(cc::Map, GetRecommendedSpawnPoints))
    .def("get_waypoint", &cc::Map::GetWaypoint, (arg("location"), arg("project_to_road")=true))
    .def("get_topology", &GetTopology)
    .def("generate_waypoints", CALL_RETURNING_LIST_1(cc::Map, GenerateWaypoints, double), (args("distance")))
    .def("transform_to_geolocation", &ToGeolocation, (arg("location")))
    .def("to_opendrive", CALL_RETURNING_COPY(cc::Map, GetOpenDrive))
    .def("save_to_disk", &SaveOpenDriveToDisk, (arg("path")=""))
    .def(self_ns::str(self_ns::self))
  ;

  enum_<cc::Waypoint::LaneChange>("LaneChange")
    .value("None", cc::Waypoint::LaneChange::None)
    .value("Right", cc::Waypoint::LaneChange::Right)
    .value("Left", cc::Waypoint::LaneChange::Left)
    .value("Both", cc::Waypoint::LaneChange::Both)
  ;

  class_<cc::Waypoint, boost::noncopyable, boost::shared_ptr<cc::Waypoint>>("Waypoint", no_init)
    .add_property("id", &cc::Waypoint::GetId)
    .add_property("transform", CALL_RETURNING_COPY(cc::Waypoint, GetTransform))
    .add_property("is_intersection", &cc::Waypoint::IsIntersection)
    .add_property("lane_width", &cc::Waypoint::GetLaneWidth)
    .add_property("road_id", &cc::Waypoint::GetRoadId)
    .add_property("lane_id", &cc::Waypoint::GetLaneId)
    .add_property("s", &cc::Waypoint::GetDistance)
    .add_property("lane_change", &cc::Waypoint::GetLaneChange)
    .add_property("lane_type", &cc::Waypoint::GetType)
    .def("next", CALL_RETURNING_LIST_1(cc::Waypoint, Next, double), (args("distance")))
    .def("get_right_lane", &cc::Waypoint::Right)
    .def("get_left_lane", &cc::Waypoint::Left)
    .def(self_ns::str(self_ns::self))
  ;
}
