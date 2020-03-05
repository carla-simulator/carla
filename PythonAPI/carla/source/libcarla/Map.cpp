// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include <carla/FileSystem.h>
#include <carla/PythonUtil.h>
#include <carla/client/Junction.h>
#include <carla/client/Map.h>
#include <carla/client/Waypoint.h>
#include <carla/road/element/LaneMarking.h>
#include <carla/client/Landmark.h>

#include <ostream>
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

static auto GetJunctionWaypoints(const carla::client::Junction &self, const carla::road::Lane::LaneType lane_type) {
  namespace py = boost::python;
  auto topology = self.GetWaypoints(lane_type);
  py::list result;
  for (auto &pair : topology) {
    result.append(py::make_tuple(pair.first, pair.second));
  }
  return result;
}

static auto GetLaneValidities(const carla::client::Landmark &self){
  namespace py = boost::python;
  auto &validities = self.GetValidities();
  py::list result;
  for(auto &validity : validities) {
    result.append(py::make_tuple(validity._from_lane, validity._to_lane));
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
  namespace cr = carla::road;
  namespace cg = carla::geom;
  namespace cre = carla::road::element;

  // ===========================================================================
  // -- Enums ------------------------------------------------------------------
  // ===========================================================================

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

  enum_<cre::LaneMarking::LaneChange>("LaneChange")
    .value("NONE", cre::LaneMarking::LaneChange::None)
    .value("Right", cre::LaneMarking::LaneChange::Right)
    .value("Left", cre::LaneMarking::LaneChange::Left)
    .value("Both", cre::LaneMarking::LaneChange::Both)
  ;

  enum_<cre::LaneMarking::Color>("LaneMarkingColor")
    .value("Standard", cre::LaneMarking::Color::Standard)
    .value("Blue", cre::LaneMarking::Color::Blue)
    .value("Green", cre::LaneMarking::Color::Green)
    .value("Red", cre::LaneMarking::Color::Red)
    .value("White", cre::LaneMarking::Color::White)
    .value("Yellow", cre::LaneMarking::Color::Yellow)
    .value("Other", cre::LaneMarking::Color::Other)
  ;

  enum_<cre::LaneMarking::Type>("LaneMarkingType")
    .value("NONE", cre::LaneMarking::Type::None)
    .value("Other", cre::LaneMarking::Type::Other)
    .value("Broken", cre::LaneMarking::Type::Broken)
    .value("Solid", cre::LaneMarking::Type::Solid)
    .value("SolidSolid", cre::LaneMarking::Type::SolidSolid)
    .value("SolidBroken", cre::LaneMarking::Type::SolidBroken)
    .value("BrokenSolid", cre::LaneMarking::Type::BrokenSolid)
    .value("BrokenBroken", cre::LaneMarking::Type::BrokenBroken)
    .value("BottsDots", cre::LaneMarking::Type::BottsDots)
    .value("Grass", cre::LaneMarking::Type::Grass)
    .value("Curb", cre::LaneMarking::Type::Curb)
  ;

  enum_<cr::SignalOrientation>("LandmarkOrientation")
    .value("Positive", cr::SignalOrientation::Positive)
    .value("Negative", cr::SignalOrientation::Negative)
    .value("Both", cr::SignalOrientation::Both)
  ;
  // ===========================================================================
  // -- Map --------------------------------------------------------------------
  // ===========================================================================

  class_<cc::Map, boost::noncopyable, boost::shared_ptr<cc::Map>>("Map", no_init)
    .def(init<std::string, std::string>((arg("name"), arg("xodr_content"))))
    .add_property("name", CALL_RETURNING_COPY(cc::Map, GetName))
    .def("get_spawn_points", CALL_RETURNING_LIST(cc::Map, GetRecommendedSpawnPoints))
    .def("get_waypoint", &cc::Map::GetWaypoint, (arg("location"), arg("project_to_road")=true, arg("lane_type")=cr::Lane::LaneType::Driving))
    .def("get_waypoint_xodr", &cc::Map::GetWaypointXODR, (arg("road_id"), arg("lane_id"), arg("s")))
    .def("get_topology", &GetTopology)
    .def("generate_waypoints", CALL_RETURNING_LIST_1(cc::Map, GenerateWaypoints, double), (args("distance")))
    .def("transform_to_geolocation", &ToGeolocation, (arg("location")))
    .def("to_opendrive", CALL_RETURNING_COPY(cc::Map, GetOpenDrive))
    .def("save_to_disk", &SaveOpenDriveToDisk, (arg("path")=""))
    .def("get_crosswalks", CALL_RETURNING_LIST(cc::Map, GetAllCrosswalkZones))
    .def(self_ns::str(self_ns::self))
  ;

  // ===========================================================================
  // -- Helper objects ---------------------------------------------------------
  // ===========================================================================

  class_<cre::LaneMarking>("LaneMarking", no_init)
    .add_property("type", &cre::LaneMarking::type)
    .add_property("color", &cre::LaneMarking::color)
    .add_property("lane_change", &cre::LaneMarking::lane_change)
    .add_property("width", &cre::LaneMarking::width)
  ;

  class_<cc::Waypoint, boost::noncopyable, boost::shared_ptr<cc::Waypoint>>("Waypoint", no_init)
    .add_property("id", &cc::Waypoint::GetId)
    .add_property("transform", CALL_RETURNING_COPY(cc::Waypoint, GetTransform))
    .add_property("is_intersection", &cc::Waypoint::IsJunction) // deprecated
    .add_property("is_junction", &cc::Waypoint::IsJunction)
    .add_property("lane_width", &cc::Waypoint::GetLaneWidth)
    .add_property("road_id", &cc::Waypoint::GetRoadId)
    .add_property("section_id", &cc::Waypoint::GetSectionId)
    .add_property("lane_id", &cc::Waypoint::GetLaneId)
    .add_property("s", &cc::Waypoint::GetDistance)
    .add_property("junction_id", &cc::Waypoint::GetJunctionId)
    .add_property("lane_change", &cc::Waypoint::GetLaneChange)
    .add_property("lane_type", &cc::Waypoint::GetType)
    .add_property("right_lane_marking", CALL_RETURNING_OPTIONAL(cc::Waypoint, GetRightLaneMarking))
    .add_property("left_lane_marking", CALL_RETURNING_OPTIONAL(cc::Waypoint, GetLeftLaneMarking))
    .def("next", CALL_RETURNING_LIST_1(cc::Waypoint, GetNext, double), (args("distance")))
    .def("previous", CALL_RETURNING_LIST_1(cc::Waypoint, GetPrevious, double), (args("distance")))
    .def("next_until_lane_end", CALL_RETURNING_LIST_1(cc::Waypoint, GetNextUntilLaneEnd, double), (args("distance")))
    .def("previous_until_lane_start", CALL_RETURNING_LIST_1(cc::Waypoint, GetPreviousUntilLaneStart, double), (args("distance", "stop_at_junction")))
    .def("get_right_lane", &cc::Waypoint::GetRight)
    .def("get_left_lane", &cc::Waypoint::GetLeft)
    .def("get_junction", &cc::Waypoint::GetJunction, (args("lane_type")))
    .def("get_landmarks", CALL_RETURNING_LIST_2(cc::Waypoint, GetAllLandmakrsInDistance, double, bool), (arg("distance"), arg("stop_at_junction")=false))
    .def("get_landmarks_of_type", CALL_RETURNING_LIST_3(cc::Waypoint, GetLandmakrsOfTypeInDistance, double, std::string, bool), (arg("distance"), arg("type"), arg("stop_at_junction")=false))
    .def(self_ns::str(self_ns::self))
  ;

  class_<cc::Junction, boost::noncopyable, boost::shared_ptr<cc::Junction>>("Junction", no_init)
    .add_property("id", &cc::Junction::GetId)
    .add_property("bounding_box", &cc::Junction::GetBoundingBox)
    .def("get_waypoints", &GetJunctionWaypoints)
  ;

  class_<cc::LandmarkType>("LandmarkType", no_init)
    .add_static_property("Danger", &cc::LandmarkType::Danger)
    .add_static_property("LanesMerging", &cc::LandmarkType::LanesMerging)
    .add_static_property("CautionPedestrian", &cc::LandmarkType::CautionPedestrian)
    .add_static_property("CautionBicycle", &cc::LandmarkType::CautionBicycle)
    .add_static_property("LevelCrossing", &cc::LandmarkType::LevelCrossing)
    .add_static_property("StopSign", &cc::LandmarkType::StopSign)
    .add_static_property("YieldSign", &cc::LandmarkType::YieldSign)
    .add_static_property("MandatoryTurnDirection", &cc::LandmarkType::MandatoryTurnDirection)
    .add_static_property("MandatoryLeftRightDirection", &cc::LandmarkType::MandatoryLeftRightDirection)
    .add_static_property("TwoChoiceTurnDirection", &cc::LandmarkType::TwoChoiceTurnDirection)
    .add_static_property("Roundabout", &cc::LandmarkType::Roundabout)
    .add_static_property("PassRightLeft", &cc::LandmarkType::PassRightLeft)
    .add_static_property("AccessForbidden", &cc::LandmarkType::AccessForbidden)
    .add_static_property("AccessForbiddenMotorvehicles", &cc::LandmarkType::AccessForbiddenMotorvehicles)
    .add_static_property("AccessForbiddenTrucks", &cc::LandmarkType::AccessForbiddenTrucks)
    .add_static_property("AccessForbiddenBicycle", &cc::LandmarkType::AccessForbiddenBicycle)
    .add_static_property("AccessForbiddenWeight", &cc::LandmarkType::AccessForbiddenWeight)
    .add_static_property("AccessForbiddenWidth", &cc::LandmarkType::AccessForbiddenWidth)
    .add_static_property("AccessForbiddenHeight", &cc::LandmarkType::AccessForbiddenHeight)
    .add_static_property("AccessForbiddenWrongDirection", &cc::LandmarkType::AccessForbiddenWrongDirection)
    .add_static_property("ForbiddenUTurn", &cc::LandmarkType::ForbiddenUTurn)
    .add_static_property("MaximumSpeed", &cc::LandmarkType::MaximumSpeed)
    .add_static_property("ForbiddenOvertakingMotorvehicles", &cc::LandmarkType::ForbiddenOvertakingMotorvehicles)
    .add_static_property("ForbiddenOvertakingTrucks", &cc::LandmarkType::ForbiddenOvertakingTrucks)
    .add_static_property("AbsoluteNoStop", &cc::LandmarkType::AbsoluteNoStop)
    .add_static_property("RestrictedStop", &cc::LandmarkType::RestrictedStop)
    .add_static_property("HasWayNextIntersection", &cc::LandmarkType::HasWayNextIntersection)
    .add_static_property("PriorityWay", &cc::LandmarkType::PriorityWay)
    .add_static_property("PriorityWayEnd", &cc::LandmarkType::PriorityWayEnd)
    .add_static_property("CityBegin", &cc::LandmarkType::CityBegin)
    .add_static_property("CityEnd", &cc::LandmarkType::CityEnd)
    .add_static_property("Highway", &cc::LandmarkType::Highway)
    .add_static_property("DeadEnd", &cc::LandmarkType::DeadEnd)
    .add_static_property("RecomendedSpeed", &cc::LandmarkType::RecomendedSpeed)
    .add_static_property("RecomendedSpeedEnd", &cc::LandmarkType::RecomendedSpeedEnd)
  ;

  class_<cc::Landmark, boost::noncopyable, boost::shared_ptr<cc::Landmark>>("Landmark", no_init)
    .add_property("road_id", &cc::Landmark::GetRoadId)
    .add_property("distance", &cc::Landmark::GetDistance)
    .add_property("s", &cc::Landmark::GetS)
    .add_property("t", &cc::Landmark::GetT)
    .add_property("id", &cc::Landmark::GetId)
    .add_property("name", &cc::Landmark::GetName)
    .add_property("is_dynamic", &cc::Landmark::IsDynamic)
    .add_property("orientation", &cc::Landmark::GetOrientation)
    .add_property("z_offset", &cc::Landmark::GetZOffset)
    .add_property("country", &cc::Landmark::GetCountry)
    .add_property("type", &cc::Landmark::GetType)
    .add_property("sub_type", &cc::Landmark::GetSubType)
    .add_property("value", &cc::Landmark::GetValue)
    .add_property("unit", &cc::Landmark::GetUnit)
    .add_property("height", &cc::Landmark::GetHeight)
    .add_property("width", &cc::Landmark::GetWidth)
    .add_property("text", &cc::Landmark::GetText)
    .add_property("h_offset", &cc::Landmark::GethOffset)
    .add_property("pitch", &cc::Landmark::GetPitch)
    .add_property("roll", &cc::Landmark::GetRoll)
    .add_property("waypoint", &cc::Landmark::GetWaypoint)
    .add_property("transform", CALL_RETURNING_COPY(cc::Landmark, GetTransform))
    .def("get_lane_validities", &GetLaneValidities)
  ;
}
