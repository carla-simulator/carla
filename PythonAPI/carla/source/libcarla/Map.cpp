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
#include <carla/road/SignalType.h>

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
    .def(init<std::string, std::string>((arg("name"), arg("xodr_content")), "@DocString(Map.__init__)"))
    .add_property("name", CALL_RETURNING_COPY(cc::Map, GetName), "@DocString(Map.name)")
    .def("get_spawn_points", CALL_RETURNING_LIST(cc::Map, GetRecommendedSpawnPoints), "@DocString(Map.get_spawn_points)")
    .def("get_waypoint", &cc::Map::GetWaypoint, (arg("location"), arg("project_to_road")=true, arg("lane_type")=cr::Lane::LaneType::Driving), "@DocString(Map.get_waypoint)")
    .def("get_waypoint_xodr", &cc::Map::GetWaypointXODR, (arg("road_id"), arg("lane_id"), arg("s")), "@DocString(Map.get_waypoint_xodr)")
    .def("get_topology", &GetTopology, "@DocString(Map.get_topology)")
    .def("generate_waypoints", CALL_RETURNING_LIST_1(cc::Map, GenerateWaypoints, double), (args("distance")), "@DocString(Map.generate_waypoints)")
    .def("transform_to_geolocation", &ToGeolocation, (arg("location")), "@DocString(Map.transform_to_geolocation)")
    .def("to_opendrive", CALL_RETURNING_COPY(cc::Map, GetOpenDrive), "@DocString(Map.to_opendrive)")
    .def("save_to_disk", &SaveOpenDriveToDisk, (arg("path")=""), "@DocString(Map.save_to_disk)")
    .def("get_crosswalks", CALL_RETURNING_LIST(cc::Map, GetAllCrosswalkZones), "@DocString(Map.get_crosswalks)")
    .def("get_all_landmarks", CALL_RETURNING_LIST(cc::Map, GetAllLandmarks), "@DocString(Map.get_all_landmarks)")
    .def("get_all_landmarks_from_id", CALL_RETURNING_LIST_1(cc::Map, GetLandmarksFromId, std::string), (args("opendrive_id")), "@DocString(Map.get_all_landmarks_from_id)")
    .def("get_all_landmarks_of_type", CALL_RETURNING_LIST_1(cc::Map, GetAllLandmarksOfType, std::string), (args("type")), "@DocString(Map.get_all_landmarks_of_type)")
    .def("get_landmark_group", CALL_RETURNING_LIST_1(cc::Map, GetLandmarkGroup, cc::Landmark), args("landmark"), "@DocString(Map.get_landmark_group)")
    .def(self_ns::str(self_ns::self))
  ;

  // ===========================================================================
  // -- Helper objects ---------------------------------------------------------
  // ===========================================================================

  class_<cre::LaneMarking>("LaneMarking", no_init)
    .add_property("type", &cre::LaneMarking::type, "@DocString(LaneMarking.type)")
    .add_property("color", &cre::LaneMarking::color, "@DocString(LaneMarking.color)")
    .add_property("lane_change", &cre::LaneMarking::lane_change, "@DocString(LaneMarking.lane_change)")
    .add_property("width", &cre::LaneMarking::width, "@DocString(LaneMarking.width)")
  ;

  class_<cc::Waypoint, boost::noncopyable, boost::shared_ptr<cc::Waypoint>>("Waypoint", no_init)
    .add_property("id", &cc::Waypoint::GetId, "@DocString(Waypoint.id)")
    .add_property("transform", CALL_RETURNING_COPY(cc::Waypoint, GetTransform), "@DocString(Waypoint.transform)")
    .add_property("is_intersection", &cc::Waypoint::IsJunction, "@DocString(Waypoint.is_intersection)") // deprecated
    .add_property("is_junction", &cc::Waypoint::IsJunction, "@DocString(Waypoint.is_junction)")
    .add_property("lane_width", &cc::Waypoint::GetLaneWidth, "@DocString(Waypoint.lane_width)")
    .add_property("road_id", &cc::Waypoint::GetRoadId, "@DocString(Waypoint.road_id)")
    .add_property("section_id", &cc::Waypoint::GetSectionId, "@DocString(Waypoint.section_id)")
    .add_property("lane_id", &cc::Waypoint::GetLaneId, "@DocString(Waypoint.lane_id)")
    .add_property("s", &cc::Waypoint::GetDistance, "@DocString(Waypoint.s)")
    .add_property("junction_id", &cc::Waypoint::GetJunctionId, "@DocString(Waypoint.junction_id)")
    .add_property("lane_change", &cc::Waypoint::GetLaneChange, "@DocString(Waypoint.lane_change)")
    .add_property("lane_type", &cc::Waypoint::GetType, "@DocString(Waypoint.lane_type)")
    .add_property("right_lane_marking", CALL_RETURNING_OPTIONAL(cc::Waypoint, GetRightLaneMarking), "@DocString(Waypoint.right_lane_marking)")
    .add_property("left_lane_marking", CALL_RETURNING_OPTIONAL(cc::Waypoint, GetLeftLaneMarking), "@DocString(Waypoint.left_lane_marking)")
    .def("next", CALL_RETURNING_LIST_1(cc::Waypoint, GetNext, double), (args("distance")), "@DocString(Waypoint.next)")
    .def("previous", CALL_RETURNING_LIST_1(cc::Waypoint, GetPrevious, double), (args("distance")), "@DocString(Waypoint.previous)")
    .def("next_until_lane_end", CALL_RETURNING_LIST_1(cc::Waypoint, GetNextUntilLaneEnd, double), (args("distance")), "@DocString(Waypoint.next_until_lane_end)")
    .def("previous_until_lane_start", CALL_RETURNING_LIST_1(cc::Waypoint, GetPreviousUntilLaneStart, double), (args("distance", "stop_at_junction")), "@DocString(Waypoint.previous_until_lane_start)")
    .def("get_right_lane", &cc::Waypoint::GetRight, "@DocString(Waypoint.get_right_lane)")
    .def("get_left_lane", &cc::Waypoint::GetLeft, "@DocString(Waypoint.get_left_lane)")
    .def("get_junction", &cc::Waypoint::GetJunction, "@DocString(Waypoint.get_junction)")
    .def("get_landmarks", CALL_RETURNING_LIST_2(cc::Waypoint, GetAllLandmakrsInDistance, double, bool), (arg("distance"), arg("stop_at_junction")=false), "@DocString(Waypoint.get_landmarks)")
    .def("get_landmarks_of_type", CALL_RETURNING_LIST_3(cc::Waypoint, GetLandmakrsOfTypeInDistance, double, std::string, bool), (arg("distance"), arg("type"), arg("stop_at_junction")=false), "@DocString(Waypoint.get_landmarks_of_type)")
    .def(self_ns::str(self_ns::self))
  ;

  class_<cc::Junction, boost::noncopyable, boost::shared_ptr<cc::Junction>>("Junction", no_init)
    .add_property("id", &cc::Junction::GetId, "@DocString(Junction.id)")
    .add_property("bounding_box", &cc::Junction::GetBoundingBox, "@DocString(Junction.bounding_box)")
    .def("get_waypoints", &GetJunctionWaypoints, "@DocString(Junction.get_waypoints)")
  ;

  class_<cr::SignalType>("LandmarkType", no_init)
    .add_static_property("Danger", &cr::SignalType::Danger, "@DocString(LandmarkType.Danger)")
    .add_static_property("LanesMerging", &cr::SignalType::LanesMerging, "@DocString(LandmarkType.LanesMerging)")
    .add_static_property("CautionPedestrian", &cr::SignalType::CautionPedestrian, "@DocString(LandmarkType.CautionPedestrian)")
    .add_static_property("CautionBicycle", &cr::SignalType::CautionBicycle, "@DocString(LandmarkType.CautionBicycle)")
    .add_static_property("LevelCrossing", &cr::SignalType::LevelCrossing, "@DocString(LandmarkType.LevelCrossing)")
    .add_static_property("StopSign", &cr::SignalType::StopSign, "@DocString(LandmarkType.StopSign)")
    .add_static_property("YieldSign", &cr::SignalType::YieldSign, "@DocString(LandmarkType.YieldSign)")
    .add_static_property("MandatoryTurnDirection", &cr::SignalType::MandatoryTurnDirection, "@DocString(LandmarkType.MandatoryTurnDirection)")
    .add_static_property("MandatoryLeftRightDirection", &cr::SignalType::MandatoryLeftRightDirection, "@DocString(LandmarkType.MandatoryLeftRightDirection)")
    .add_static_property("TwoChoiceTurnDirection", &cr::SignalType::TwoChoiceTurnDirection, "@DocString(LandmarkType.TwoChoiceTurnDirection)")
    .add_static_property("Roundabout", &cr::SignalType::Roundabout, "@DocString(LandmarkType.Roundabout)")
    .add_static_property("PassRightLeft", &cr::SignalType::PassRightLeft, "@DocString(LandmarkType.PassRightLeft)")
    .add_static_property("AccessForbidden", &cr::SignalType::AccessForbidden, "@DocString(LandmarkType.AccessForbidden)")
    .add_static_property("AccessForbiddenMotorvehicles", &cr::SignalType::AccessForbiddenMotorvehicles, "@DocString(LandmarkType.AccessForbiddenMotorvehicles)")
    .add_static_property("AccessForbiddenTrucks", &cr::SignalType::AccessForbiddenTrucks, "@DocString(LandmarkType.AccessForbiddenTrucks)")
    .add_static_property("AccessForbiddenBicycle", &cr::SignalType::AccessForbiddenBicycle, "@DocString(LandmarkType.AccessForbiddenBicycle)")
    .add_static_property("AccessForbiddenWeight", &cr::SignalType::AccessForbiddenWeight, "@DocString(LandmarkType.AccessForbiddenWeight)")
    .add_static_property("AccessForbiddenWidth", &cr::SignalType::AccessForbiddenWidth, "@DocString(LandmarkType.AccessForbiddenWidth)")
    .add_static_property("AccessForbiddenHeight", &cr::SignalType::AccessForbiddenHeight, "@DocString(LandmarkType.AccessForbiddenHeight)")
    .add_static_property("AccessForbiddenWrongDirection", &cr::SignalType::AccessForbiddenWrongDirection, "@DocString(LandmarkType.AccessForbiddenWrongDirection)")
    .add_static_property("ForbiddenUTurn", &cr::SignalType::ForbiddenUTurn, "@DocString(LandmarkType.ForbiddenUTurn)")
    .add_static_property("MaximumSpeed", &cr::SignalType::MaximumSpeed, "@DocString(LandmarkType.MaximumSpeed)")
    .add_static_property("ForbiddenOvertakingMotorvehicles", &cr::SignalType::ForbiddenOvertakingMotorvehicles, "@DocString(LandmarkType.ForbiddenOvertakingMotorvehicles)")
    .add_static_property("ForbiddenOvertakingTrucks", &cr::SignalType::ForbiddenOvertakingTrucks, "@DocString(LandmarkType.ForbiddenOvertakingTrucks)")
    .add_static_property("AbsoluteNoStop", &cr::SignalType::AbsoluteNoStop, "@DocString(LandmarkType.AbsoluteNoStop)")
    .add_static_property("RestrictedStop", &cr::SignalType::RestrictedStop, "@DocString(LandmarkType.RestrictedStop)")
    .add_static_property("HasWayNextIntersection", &cr::SignalType::HasWayNextIntersection, "@DocString(LandmarkType.HasWayNextIntersection)")
    .add_static_property("PriorityWay", &cr::SignalType::PriorityWay, "@DocString(LandmarkType.PriorityWay)")
    .add_static_property("PriorityWayEnd", &cr::SignalType::PriorityWayEnd, "@DocString(LandmarkType.PriorityWayEnd)")
    .add_static_property("CityBegin", &cr::SignalType::CityBegin, "@DocString(LandmarkType.CityBegin)")
    .add_static_property("CityEnd", &cr::SignalType::CityEnd, "@DocString(LandmarkType.CityEnd)")
    .add_static_property("Highway", &cr::SignalType::Highway, "@DocString(LandmarkType.Highway)")
    .add_static_property("DeadEnd", &cr::SignalType::DeadEnd, "@DocString(LandmarkType.DeadEnd)")
    .add_static_property("RecomendedSpeed", &cr::SignalType::RecomendedSpeed, "@DocString(LandmarkType.RecomendedSpeed)")
    .add_static_property("RecomendedSpeedEnd", &cr::SignalType::RecomendedSpeedEnd, "@DocString(LandmarkType.RecomendedSpeedEnd)")
  ;

  class_<cc::Landmark, boost::noncopyable, boost::shared_ptr<cc::Landmark>>("Landmark", no_init)
    .add_property("road_id", &cc::Landmark::GetRoadId, "@DocString(Landmark.road_id)")
    .add_property("distance", &cc::Landmark::GetDistance, "@DocString(Landmark.distance)")
    .add_property("s", &cc::Landmark::GetS, "@DocString(Landmark.s)")
    .add_property("t", &cc::Landmark::GetT, "@DocString(Landmark.t)")
    .add_property("id", &cc::Landmark::GetId, "@DocString(Landmark.id)")
    .add_property("name", &cc::Landmark::GetName, "@DocString(Landmark.name)")
    .add_property("is_dynamic", &cc::Landmark::IsDynamic, "@DocString(Landmark.is_dynamic)")
    .add_property("orientation", &cc::Landmark::GetOrientation, "@DocString(Landmark.orientation)")
    .add_property("z_offset", &cc::Landmark::GetZOffset, "@DocString(Landmark.z_offset)")
    .add_property("country", &cc::Landmark::GetCountry, "@DocString(Landmark.country)")
    .add_property("type", &cc::Landmark::GetType, "@DocString(Landmark.type)")
    .add_property("sub_type", &cc::Landmark::GetSubType, "@DocString(Landmark.sub_type)")
    .add_property("value", &cc::Landmark::GetValue, "@DocString(Landmark.value)")
    .add_property("unit", &cc::Landmark::GetUnit, "@DocString(Landmark.unit)")
    .add_property("height", &cc::Landmark::GetHeight, "@DocString(Landmark.height)")
    .add_property("width", &cc::Landmark::GetWidth, "@DocString(Landmark.width)")
    .add_property("text", &cc::Landmark::GetText, "@DocString(Landmark.text)")
    .add_property("h_offset", &cc::Landmark::GethOffset, "@DocString(Landmark.h_offset)")
    .add_property("pitch", &cc::Landmark::GetPitch, "@DocString(Landmark.pitch)")
    .add_property("roll", &cc::Landmark::GetRoll, "@DocString(Landmark.roll)")
    .add_property("waypoint", &cc::Landmark::GetWaypoint, "@DocString(Landmark.waypoint)")
    .add_property("transform", CALL_RETURNING_COPY(cc::Landmark, GetTransform), "@DocString(Landmark.transform)")
    .def("get_lane_validities", &GetLaneValidities, "@DocString(Landmark.get_lane_validities)")
  ;
}
