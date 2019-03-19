// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "carla/StringUtil.h"
#include "carla/road/MapBuilder.h"
#include "carla/road/element/RoadInfoElevation.h"
#include "carla/road/element/RoadInfoGeometry.h"
#include "carla/road/element/RoadInfoLaneAccess.h"
#include "carla/road/element/RoadInfoLaneBorder.h"
#include "carla/road/element/RoadInfoLaneHeight.h"
#include "carla/road/element/RoadInfoLaneMaterial.h"
#include "carla/road/element/RoadInfoLaneOffset.h"
#include "carla/road/element/RoadInfoLaneRule.h"
#include "carla/road/element/RoadInfoLaneVisibility.h"
#include "carla/road/element/RoadInfoLaneWidth.h"
#include "carla/road/element/RoadInfoMarkRecord.h"
#include "carla/road/element/RoadInfoMarkTypeLine.h"
#include "carla/road/element/RoadInfoSpeed.h"
#include "carla/road/element/RoadInfoVisitor.h"
#include "carla/road/general/Validity.h"
#include "carla/road/signal/Signal.h"
#include "carla/road/signal/SignalReference.h"
#include "carla/road/signal/SignalDependency.h"

#include <iterator>
#include <memory>

using namespace carla::road::element;

namespace carla {
namespace road {

  boost::optional<Map> MapBuilder::Build() {

    CreatePointersBetweenRoadSegments();

    // Delete all the unneeded temporal data once used
    _temp_road_info_container.empty();
    _temp_lane_info_container.empty();

    // _map_data is a memeber of MapBuilder so you must especify if
    // you want to keep it (will return copy -> Map(const Map &))
    // or move it (will return move -> Map(Map &&))
    return Map{std::move(_map_data)};
  }

  // called from profiles parser
  void MapBuilder::AddRoadElevationProfile(
      const Road* road,
      const float s,
      const float a,
      const float b,
      const float c,
      const float d) {

    auto elevation = std::make_unique<RoadInfoElevation>(s, a, b, c, d);
    _temp_road_info_container[road].emplace_back(std::move(elevation));
  }

  // void MapBuilder::AddRoadLateralSuperElevation(
  //     const Road* road,
  //     const float s,
  //     const float a,
  //     const float b,
  //     const float c,
  //     const float d) {}

  // void MapBuilder::AddRoadLateralCrossfall(
  //     const Road* road,
  //     const float s,
  //     const float a,
  //     const float b,
  //     const float c,
  //     const float d,
  //     const std::string side) {}

  // called from lane parser
  void MapBuilder::CreateLaneAccess(
      const Lane *lane,
      const float s,
      const std::string restriction) {
    _temp_lane_info_container[lane].emplace_back(std::make_unique<RoadInfoLaneAccess>(s, restriction));
  }

  void MapBuilder::CreateLaneBorder(
      const Lane *lane,
      const float s,
      const float a,
      const float b,
      const float c,
      const float d) {
    _temp_lane_info_container[lane].emplace_back(std::make_unique<RoadInfoLaneBorder>(s, a, b, c, d));
  }

  void MapBuilder::CreateLaneHeight(
      const Lane *lane,
      const float s,
      const float inner,
      const float outer) {
    _temp_lane_info_container[lane].emplace_back(std::make_unique<RoadInfoLaneHeight>(s, inner, outer));
  }

  void MapBuilder::CreateLaneMaterial(
      const Lane *lane,
      const float s,
      const std::string surface,
      const float friction,
      const float roughness) {
    _temp_lane_info_container[lane].emplace_back(std::make_unique<RoadInfoLaneMaterial>(s, surface, friction,
        roughness));
  }

  void MapBuilder::CreateSectionOffset(
      const Road *road,
      const float s,
      const float a,
      const float b,
      const float c,
      const float d) {
    _temp_road_info_container[road].emplace_back(std::make_unique<RoadInfoLaneOffset>(s, a, b, c, d));
  }

  void MapBuilder::CreateLaneRule(
      const Lane *lane,
      const float s,
      const std::string value) {

    _temp_lane_info_container[lane].emplace_back(std::make_unique<RoadInfoLaneRule>(s, value));
  }

  void MapBuilder::CreateLaneVisibility(
      const Lane *lane,
      const float s,
      const float forward,
      const float back,
      const float left,
      const float right) {
    _temp_lane_info_container[lane].emplace_back(std::make_unique<RoadInfoLaneVisibility>(s, forward, back,
        left, right));
  }

  void MapBuilder::CreateLaneWidth(
      const Lane *lane,
      const float s,
      const float a,
      const float b,
      const float c,
      const float d) {
    _temp_lane_info_container[lane].emplace_back(std::make_unique<RoadInfoLaneWidth>(s, a, b, c, d));
  }

  void MapBuilder::CreateRoadMark(
      const Lane *lane,
      const int road_mark_id,
      const float s,
      const std::string type,
      const std::string weight,
      const std::string color,
      const std::string material,
      const float width,
      const std::string lane_change,
      const float height,
      const std::string type_name,
      const float type_width) {
    RoadInfoMarkRecord::LaneChange lc;

    auto ToLower = [](auto str) { return StringUtil::ToLowerCopy(str); };

    if (ToLower(lane_change) == "increase") {
      lc = RoadInfoMarkRecord::LaneChange::Increase;
    } else if (ToLower(lane_change) == "decrease") {
      lc = RoadInfoMarkRecord::LaneChange::Decrease;
    } else if (ToLower(lane_change) == "both") {
      lc = RoadInfoMarkRecord::LaneChange::Both;
    } else {
      lc = RoadInfoMarkRecord::LaneChange::None;
    }
    _temp_lane_info_container[lane].emplace_back(std::make_unique<RoadInfoMarkRecord>(s, road_mark_id, type,
        weight, color,
        material, width, lc, height, type_name, type_width));
  }

  void MapBuilder::CreateRoadMarkTypeLine(
      const Lane *lane,
      const int road_mark_id,
      const float length,
      const float space,
      const float tOffset,
      const float s,
      const std::string rule,
      const float width) {

    auto it = MakeRoadInfoIterator<RoadInfoMarkRecord>(_temp_lane_info_container[lane]);
    for (; !it.IsAtEnd(); ++it) {
      if(it->GetRoadMarkId() == road_mark_id) {
        it->GetLines().emplace_back(std::make_unique<RoadInfoMarkTypeLine>(s, road_mark_id, length, space,
        tOffset, rule, width));
        break;
      }
    }

  }

  void MapBuilder::CreateRoadSpeed(
      Road *road,
      const float s,
      const std::string /*type*/,
      const float max,
      const std::string /*unit*/) {
    _temp_road_info_container[road].emplace_back(std::make_unique<RoadInfoSpeed>(s, max));
  }

  void MapBuilder::CreateLaneSpeed(
      const Lane *lane,
      const float s,
      const float max,
      const std::string /*unit*/) {
    _temp_lane_info_container[lane].emplace_back(std::make_unique<RoadInfoSpeed>(s, max));
  }

  void MapBuilder::AddSignal(
      const uint32_t road_id,
      const uint32_t signal_id,
      const float s,
      const float t,
      const std::string name,
      const std::string dynamic,
      const std::string orientation,
      const float zOffset,
      const std::string country,
      const std::string type,
      const std::string subtype,
      const float value,
      const std::string unit,
      const float height,
      const float width,
      const std::string text,
      const float hOffset,
      const float pitch,
      const float roll) {

    _map_data.GetRoad(road_id)->getSignals()->emplace(signal_id,
        signal::Signal(road_id, signal_id, s, t, name, dynamic,
        orientation, zOffset, country, type, subtype, value, unit, height, width,
        text, hOffset, pitch, roll));
  }

  void MapBuilder::AddValidityToLastAddedSignal(
      const uint32_t road_id,
      const uint32_t signal_id,
      const int32_t from_lane,
      const int32_t to_lane) {
    _map_data.GetRoad(road_id)->GetSignal(signal_id)->AddValidity(general::Validity(signal_id, from_lane, to_lane));
  }

  // build road objects
  carla::road::Road *MapBuilder::AddRoad(
      const uint32_t road_id,
      const std::string name,
      const float length,
      const int32_t junction_id,
      const int32_t predecessor,
      const int32_t successor) {

    // add it
    auto road = &(_map_data._roads.emplace(road_id, Road()).first->second);

    // set road data
    road->_map_data = &_map_data;
    road->_id = road_id;
    road->_name = name;
    road->_length = length;
    road->_junction_id = junction_id;
    (junction_id != -1) ? road->_is_junction = true : road->_is_junction = false;
    road->_nexts.push_back(successor);
    road->_prevs.push_back(predecessor);

    return road;
  }

  carla::road::LaneSection *MapBuilder::AddRoadSection(
      carla::road::Road *road,
      const float s) {

    // add it
    carla::road::LaneSection *sec = &(road->_lane_sections.emplace(std::make_pair(float(s), LaneSection())))->second;

    // set section data
    sec->_road = const_cast<Road *>(road);
    sec->_s = s;

    return sec;
  }

  carla::road::Lane *MapBuilder::AddRoadSectionLane(
      carla::road::LaneSection *section,
      const int32_t lane_id,
      const std::string lane_type,
      const bool lane_level,
      const int32_t predecessor,
      const int32_t successor) {

    // add the lane
    auto *lane = &((section->_lanes.emplace(lane_id, Lane()).first)->second);

    // set lane data
    lane->_id = lane_id;
    lane->_lane_section = section;
    lane->_level = lane_level;
    lane->_type = lane_type;
    // we save id as pointers temporally, later will be processed in the right
    // way
    lane->_next_lanes.emplace_back(reinterpret_cast<Lane *>(successor));
    lane->_prev_lanes.emplace_back(reinterpret_cast<Lane *>(predecessor));

    return lane;
  }

  void MapBuilder::AddRoadGeometryLine(
      carla::road::Road *road,
      const float s,
      const float x,
      const float y,
      const float hdg,
      const float length) {

    auto line_geometry = std::make_unique<GeometryLine>(s,
        length,
        hdg,
        geom::Location(x, y, 0.0f));

    _temp_road_info_container[road].emplace_back(std::unique_ptr<RoadInfo>(new RoadInfoGeometry(s,
        std::move(line_geometry))));
  }

  void MapBuilder::AddRoadGeometryArc(
      carla::road::Road *road,
      const float s,
      const float x,
      const float y,
      const float hdg,
      const float length,
      const float curvature) {

    auto arc_geometry = std::make_unique<GeometryArc>(s,
        length,
        hdg,
        geom::Location(x, y, 0.0f),
        curvature);

    _temp_road_info_container[road].emplace_back(std::unique_ptr<RoadInfo>(new RoadInfoGeometry(s,
        std::move(arc_geometry))));
  }

  void MapBuilder::AddRoadGeometrySpiral(
      carla::road::Road * /*road*/,
      const float /*s*/,
      const float /*x*/,
      const float /*y*/,
      const float /*hdg*/,
      const float /*length*/,
      const float /*curvStart*/,
      const float /*curvEnd*/) {}

  void MapBuilder::AddRoadGeometryPoly3(
      carla::road::Road * /*road*/,
      const float /*s*/,
      const float /*x*/,
      const float /*y*/,
      const float /*hdg*/,
      const float /*length*/,
      const float /*a*/,
      const float /*b*/,
      const float /*c*/,
      const float /*d*/) {}

  void MapBuilder::AddRoadGeometryParamPoly3(
      carla::road::Road * /*road*/,
      const float /*s*/,
      const float /*x*/,
      const float /*y*/,
      const float /*hdg*/,
      const float /*length*/,
      const float /*aU*/,
      const float /*bU*/,
      const float /*cU*/,
      const float /*dU*/,
      const float /*aV*/,
      const float /*bV*/,
      const float /*cV*/,
      const float /*dV*/,
      const std::string /*p_range*/) {}

  void MapBuilder::AddJunction(const int32_t id, const std::string name) {
    _map_data.GetJunctions().emplace(id, Junction(id, name));
  }

  void MapBuilder::AddConnection(
      const int32_t junction_id,
      const int32_t connection_id,
      const int32_t incoming_road,
      const int32_t connecting_road) {
    _map_data.GetJunction(junction_id)->GetConnections().emplace(connection_id,
        Junction::Connection(connection_id, incoming_road, connecting_road));
  }

  void MapBuilder::AddLaneLink(
      const int32_t junction_id,
      const int32_t connection_id,
      const int32_t from,
      const int32_t to) {
    _map_data.GetJunction(junction_id)->GetConnection(connection_id)->AddLaneLink(from, to);
  }

  void MapBuilder::AddValidityToSignal(
      const uint32_t road_id,
      const uint32_t signal_id,
      const int32_t from_lane,
      const int32_t to_lane) {
    _map_data.GetRoad(road_id)->GetSignal(signal_id)->AddValidity(general::Validity(signal_id, from_lane,
        to_lane));
  }

  void MapBuilder::AddValidityToSignalReference(
      const uint32_t road_id,
      const uint32_t signal_reference_id,
      const int32_t from_lane,
      const int32_t to_lane) {
    _map_data.GetRoad(road_id)->GetSignalRef(signal_reference_id)->AddValidity(general::Validity(
        signal_reference_id, from_lane, to_lane));
  }

  void MapBuilder::AddSignalReference(
      const uint32_t road_id,
      const uint32_t signal_reference_id,
      const float s_position,
      const float t_position,
      const std::string signal_reference_orientation) {
    _map_data.GetRoad(road_id)->getSignalReferences()->emplace(signal_reference_id,
        signal::SignalReference(road_id, signal_reference_id, s_position, t_position,
        signal_reference_orientation));
  }

  void MapBuilder::AddDependencyToSignal(
      const uint32_t road_id,
      const uint32_t signal_id,
      const uint32_t dependency_id,
      const std::string dependency_type) {
    _map_data.GetRoad(road_id)->GetSignal(signal_id)->AddDependency(signal::SignalDependency(
        road_id,
        signal_id,
        dependency_id,
        dependency_type));
  }

  Lane *MapBuilder::GetLane(
      const RoadId road_id,
      const LaneId lane_id,
      const float s) {
    return _map_data.GetLane(road_id, lane_id, s);
  }

  Road *MapBuilder::GetRoad(
      const RoadId road_id) {
    return _map_data.GetRoad(road_id);
  }
  // return the pointer to a lane object
  Lane *MapBuilder::GetEdgeLanePointer(RoadId road_id, bool from_start, LaneId lane_id) {

    // get the road
    Road *road = _map_data.GetRoad(road_id);
    if (road == nullptr)
      return nullptr;

    // get the lane section
    LaneSection *section;
    if (from_start)
      section = road->GetStartSection(lane_id);
    else
      section = road->GetEndSection(lane_id);

    // get the lane
    return section->GetLane(lane_id);
  }

  // return a list of pointers to all lanes from a lane (using road and junction info)
  std::vector<Lane *> MapBuilder::GetLaneNext(RoadId road_id, float s, LaneId lane_id) {
    std::vector<Lane *> result;

    // get the road
    Road *road = _map_data.GetRoad(road_id);
    if (road == nullptr)
      return result;

    // get the lane
    Lane *lane = road->GetLane(lane_id, s);

    // successor and predecessor
    int64_t next = reinterpret_cast<int64_t>(lane->_next_lanes[0]);
    // int32_t prev = reinterpret_cast<int32_t>(lane->_prev_lanes[0]);

    if (s > 0 and road->_lane_sections.upper_bound(s) != road->_lane_sections.end()) {
      // change to next / prev section
      if (lane->_id < 0)
        result.push_back(road->GetNextLane(s, next));
      else
        result.push_back(road->GetPrevLane(s, next));
    }
    else {
      // change to another road / junction
      if (next != 0) {
        // single road
        result.push_back(GetEdgeLanePointer(road->_nexts[0], next < 0, next));
      }
      else {
        // several roads (junction)
        auto options = GetJunctionLanes(road->_nexts[0], road_id, lane_id);
        for (auto opt : options) {
          result.push_back(GetEdgeLanePointer(opt.first, opt.second < 0, opt.second));
        }
      }
    }

    return result;
  }

  std::vector<std::pair<RoadId, LaneId>> MapBuilder::GetJunctionLanes(RoadId junction_id, RoadId road_id, LaneId lane_id) {
    std::vector<std::pair<RoadId, LaneId>> result;

    // get the junction
    Junction *junction = _map_data.GetJunction(junction_id);
    if (junction == nullptr)
      return result;

    // check all connections
    for (auto con : junction->_connections) {
      // only connections for our road
      if (con.second.incoming_road == road_id) {
        // check all lane links
        for (auto link : con.second.lane_links) {
          // is our lane id ?
          if (link.from == lane_id) {
            // add as option
            result.push_back(std::make_pair(con.second.connecting_road, link.to));
          }
        }
      }
    }

    return result;
  }

  // assign pointers to the next lanes
  void MapBuilder::CreatePointersBetweenRoadSegments(void) {
    // for all roads
    for (auto &road : _map_data._roads) {
      // for each section
      for (auto &section : road.second._lane_sections) {
        // for each lane
        for (auto &lane : section.second._lanes) {
          // assign the next lane pointers
          lane.second._next_lanes = GetLaneNext(road.first, section.second._s, lane.second._id);
        }
      }
    }
  }
} // namespace road
} // namespace carla
