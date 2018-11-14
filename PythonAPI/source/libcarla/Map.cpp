// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include <carla/FileSystem.h>
#include <carla/PythonUtil.h>
#include <carla/client/Map.h>
#include <carla/client/Waypoint.h>

#include <boost/python/suite/indexing/vector_indexing_suite.hpp>

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

static auto GenerateWaypoints(const carla::client::Map &self, double distance) {
  namespace py = boost::python;
  auto waypoints = self.GenerateWaypoints(distance);
  py::list result;
  for (auto &&waypoint : waypoints) {
    result.append(waypoint);
  }
  return result;
}

void export_map() {
  using namespace boost::python;
  namespace cc = carla::client;
  namespace cg = carla::geom;

  class_<std::vector<cg::Transform>>("vector_of_transforms")
    .def(vector_indexing_suite<std::vector<cg::Transform>>())
    .def(self_ns::str(self_ns::self))
  ;

  class_<std::vector<carla::SharedPtr<cc::Waypoint>>>("vector_of_waypoints")
    .def(vector_indexing_suite<std::vector<carla::SharedPtr<cc::Waypoint>>, true>())
    .def(self_ns::str(self_ns::self))
  ;

  class_<cc::Map, boost::noncopyable, boost::shared_ptr<cc::Map>>("Map", no_init)
    .add_property("name", CALL_RETURNING_COPY(cc::Map, GetName))
    .def("get_spawn_points", CALL_RETURNING_COPY(cc::Map, GetRecommendedSpawnPoints))
    .def("get_waypoint", &cc::Map::GetWaypoint, (arg("location"), arg("project_to_road")=true))
    .def("get_topology", &GetTopology)
    .def("generate_waypoints", &GenerateWaypoints)
    .def("to_opendrive", CALL_RETURNING_COPY(cc::Map, GetOpenDrive))
    .def("save_to_disk", &SaveOpenDriveToDisk, (arg("path")=""))
    .def(self_ns::str(self_ns::self))
  ;

  class_<cc::Waypoint, boost::noncopyable, boost::shared_ptr<cc::Waypoint>>("Waypoint", no_init)
    .add_property("transform", CALL_RETURNING_COPY(cc::Waypoint, GetTransform))
    .add_property("road_id", &cc::Waypoint::GetRoadId)
    .add_property("lane_id", &cc::Waypoint::GetLaneId)
    .def("next", &cc::Waypoint::Next, (args("distance")))
    .def(self_ns::str(self_ns::self))
  ;
}
