// Copyright (c) 2024 Computer Vision Center (CVC) at the Universitat Autonoma
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
#include "carla/road/element/RoadInfoSignal.h"
#include "carla/road/element/RoadInfoVisitor.h"
#include "carla/road/element/RoadInfoCrosswalk.h"
#include "carla/road/InformationSet.h"
#include "carla/road/Signal.h"
#include "carla/road/SignalType.h"

#include <iterator>
#include <memory>
#include <algorithm>

using namespace carla::road::element;

namespace carla {
namespace road {

  std::optional<Map> MapBuilder::Build() {

    CreatePointersBetweenRoadSegments();
    RemoveZeroLaneValiditySignalReferences();

    for (auto &&info : _temp_road_info_container) {
      DEBUG_ASSERT(info.first != nullptr);
      info.first->_info = InformationSet(std::move(info.second));
    }

    for (auto &&info : _temp_lane_info_container) {
      DEBUG_ASSERT(info.first != nullptr);
      info.first->_info = InformationSet(std::move(info.second));
    }

    // compute transform requires the roads to have the RoadInfo
    SolveSignalReferencesAndTransforms();

    SolveControllerAndJuntionReferences();

    // remove temporal already used information
    _temp_road_info_container.clear();
    _temp_lane_info_container.clear();

    // _map_data is a memeber of MapBuilder so you must especify if
    // you want to keep it (will return copy -> Map(const Map &))
    // or move it (will return move -> Map(Map &&))
    Map map(std::move(_map_data));
    CreateJunctionBoundingBoxes(map);
    ComputeJunctionRoadConflicts(map);
    CheckSignalsOnRoads(map);

    return map;
  }

  // called from profiles parser
  void MapBuilder::AddRoadElevationProfile(
      Road *road,
      const double s,
      const double a,
      const double b,
      const double c,
      const double d) {
    DEBUG_ASSERT(road != nullptr);
    auto elevation = std::make_unique<RoadInfoElevation>(s, a, b, c, d);
    _temp_road_info_container[road].emplace_back(std::move(elevation));
  }

  void MapBuilder::AddRoadObjectCrosswalk(
      Road *road,
      const std::string name,
      const double s,
      const double t,
      const double zOffset,
      const double hdg,
      const double pitch,
      const double roll,
      const std::string orientation,
      const double width,
      const double length,
      const std::vector<road::element::CrosswalkPoint> points) {
    DEBUG_ASSERT(road != nullptr);
    auto cross = std::make_unique<RoadInfoCrosswalk>(s, name, t, zOffset, hdg, pitch, roll, std::move(orientation), width, length, std::move(points));
    _temp_road_info_container[road].emplace_back(std::move(cross));
  }

  // called from lane parser
  void MapBuilder::CreateLaneAccess(
      Lane *lane,
      const double s,
      const std::string restriction) {
    DEBUG_ASSERT(lane != nullptr);
    _temp_lane_info_container[lane].emplace_back(std::make_unique<RoadInfoLaneAccess>(s, restriction));
  }

  void MapBuilder::CreateLaneBorder(
      Lane *lane,
      const double s,
      const double a,
      const double b,
      const double c,
      const double d) {
    DEBUG_ASSERT(lane != nullptr);
    _temp_lane_info_container[lane].emplace_back(std::make_unique<RoadInfoLaneBorder>(s, a, b, c, d));
  }

  void MapBuilder::CreateLaneHeight(
      Lane *lane,
      const double s,
      const double inner,
      const double outer) {
    DEBUG_ASSERT(lane != nullptr);
    _temp_lane_info_container[lane].emplace_back(std::make_unique<RoadInfoLaneHeight>(s, inner, outer));
  }

  void MapBuilder::CreateLaneMaterial(
      Lane *lane,
      const double s,
      const std::string surface,
      const double friction,
      const double roughness) {
    DEBUG_ASSERT(lane != nullptr);
    _temp_lane_info_container[lane].emplace_back(std::make_unique<RoadInfoLaneMaterial>(s, surface, friction,
        roughness));
  }

  void MapBuilder::CreateLaneRule(
      Lane *lane,
      const double s,
      const std::string value) {
    DEBUG_ASSERT(lane != nullptr);
    _temp_lane_info_container[lane].emplace_back(std::make_unique<RoadInfoLaneRule>(s, value));
  }

  void MapBuilder::CreateLaneVisibility(
      Lane *lane,
      const double s,
      const double forward,
      const double back,
      const double left,
      const double right) {
    DEBUG_ASSERT(lane != nullptr);
    _temp_lane_info_container[lane].emplace_back(std::make_unique<RoadInfoLaneVisibility>(s, forward, back,
        left, right));
  }

  void MapBuilder::CreateLaneWidth(
      Lane *lane,
      const double s,
      const double a,
      const double b,
      const double c,
      const double d) {
    DEBUG_ASSERT(lane != nullptr);
    _temp_lane_info_container[lane].emplace_back(std::make_unique<RoadInfoLaneWidth>(s, a, b, c, d));
  }

  void MapBuilder::CreateRoadMark(
      Lane *lane,
      const int road_mark_id,
      const double s,
      const std::string type,
      const std::string weight,
      const std::string color,
      const std::string material,
      const double width,
      std::string lane_change,
      const double height,
      const std::string type_name,
      const double type_width) {
    DEBUG_ASSERT(lane != nullptr);
    RoadInfoMarkRecord::LaneChange lc;

    StringUtil::ToLower(lane_change);

    if (lane_change == "increase") {
      lc = RoadInfoMarkRecord::LaneChange::Increase;
    } else if (lane_change == "decrease") {
      lc = RoadInfoMarkRecord::LaneChange::Decrease;
    } else if (lane_change == "none") {
      lc = RoadInfoMarkRecord::LaneChange::None;
    } else {
      lc = RoadInfoMarkRecord::LaneChange::Both;
    }
    _temp_lane_info_container[lane].emplace_back(std::make_unique<RoadInfoMarkRecord>(s, road_mark_id, type,
        weight, color,
        material, width, lc, height, type_name, type_width));
  }

  void MapBuilder::CreateRoadMarkTypeLine(
      Lane *lane,
      const int road_mark_id,
      const double length,
      const double space,
      const double tOffset,
      const double s,
      const std::string rule,
      const double width) {
    DEBUG_ASSERT(lane != nullptr);
    auto it = MakeRoadInfoIterator<RoadInfoMarkRecord>(_temp_lane_info_container[lane]);
    for (; !it.IsAtEnd(); ++it) {
      if (it->GetRoadMarkId() == road_mark_id) {
        it->GetLines().emplace_back(std::make_unique<RoadInfoMarkTypeLine>(s, road_mark_id, length, space,
            tOffset, rule, width));
        break;
      }
    }

  }

  void MapBuilder::CreateLaneSpeed(
      Lane *lane,
      const double s,
      const double max,
      const std::string /*unit*/) {
    DEBUG_ASSERT(lane != nullptr);
    _temp_lane_info_container[lane].emplace_back(std::make_unique<RoadInfoSpeed>(s, max));
  }


  element::RoadInfoSignal* MapBuilder::AddSignal(
      Road* road,
      const SignId signal_id,
      const double s,
      const double t,
      const std::string name,
      const std::string dynamic,
      const std::string orientation,
      const double zOffset,
      const std::string country,
      const std::string type,
      const std::string subtype,
      const double value,
      const std::string unit,
      const double height,
      const double width,
      const std::string text,
      const double hOffset,
      const double pitch,
      const double roll) {
    _temp_signal_container[signal_id] = std::make_unique<Signal>(
        road->GetId(),
        signal_id,
        s,
        t,
        name,
        dynamic,
        orientation,
        zOffset,
        country,
        type,
        subtype,
        value,
        unit,
        height,
        width,
        text,
        hOffset,
        pitch,
        roll);

    return AddSignalReference(road, signal_id, s, t, orientation);
  }

  void MapBuilder::AddSignalPositionInertial(
      const SignId signal_id,
      const double x,
      const double y,
      const double z,
      const double hdg,
      const double pitch,
      const double roll) {
    std::unique_ptr<Signal> &signal = _temp_signal_container[signal_id];
    signal->_using_inertial_position = true;
    geom::Location location = geom::Location(x, -y, z);
    signal->_transform = geom::Transform (location, geom::Rotation(
        geom::Math::ToDegrees(static_cast<float>(pitch)),
        geom::Math::ToDegrees(static_cast<float>(-hdg)),
        geom::Math::ToDegrees(static_cast<float>(roll))));
  }

  void MapBuilder::AddSignalPositionRoad(
      const SignId signal_id,
      const RoadId road_id,
      const double s,
      const double t,
      const double zOffset,
      const double hOffset,
      const double pitch,
      const double roll) {
    std::unique_ptr<Signal> &signal = _temp_signal_container[signal_id];
    signal->_road_id = road_id;
    signal->_s = s;
    signal->_t = t;
    signal->_zOffset = zOffset;
    signal->_hOffset = hOffset;
    signal->_pitch = pitch;
    signal->_roll = roll;
  }

    element::RoadInfoSignal* MapBuilder::AddSignalReference(
        Road* road,
        const SignId signal_id,
        const double s_position,
        const double t_position,
        const std::string signal_reference_orientation) {

      const double epsilon = 0.00001;
      RELEASE_ASSERT(s_position >= 0.0);
      // Prevent s_position from being equal to the road length
      double fixed_s = geom::Math::Clamp(s_position, 0.0, road->GetLength() - epsilon);
      _temp_road_info_container[road].emplace_back(std::make_unique<element::RoadInfoSignal>(
          signal_id, road->GetId(), fixed_s, t_position, signal_reference_orientation));
      auto road_info_signal = static_cast<element::RoadInfoSignal*>(
          _temp_road_info_container[road].back().get());
      _temp_signal_reference_container.emplace_back(road_info_signal);
      return road_info_signal;
    }

    void MapBuilder::AddValidityToSignalReference(
        element::RoadInfoSignal* signal_reference,
        const LaneId from_lane,
        const LaneId to_lane) {
      signal_reference->_validities.emplace_back(LaneValidity(from_lane, to_lane));
    }

    void MapBuilder::AddDependencyToSignal(
        const SignId signal_id,
        const std::string dependency_id,
        const std::string dependency_type) {
      _temp_signal_container[signal_id]->_dependencies.emplace_back(
          SignalDependency(dependency_id, dependency_type));
    }

    // build road objects
    carla::road::Road *MapBuilder::AddRoad(
        const RoadId road_id,
        const std::string name,
        const double length,
        const JuncId junction_id,
        const RoadId predecessor,
        const RoadId successor)
    {

      // add it
      auto road = &(_map_data._roads.emplace(road_id, Road()).first->second);

      // set road data
      road->_map_data = &_map_data;
      road->_id = road_id;
      road->_name = name;
      road->_length = length;
      road->_junction_id = junction_id;
      (junction_id != -1) ? road->_is_junction = true : road->_is_junction = false;
      road->_successor = successor;
      road->_predecessor = predecessor;

      return road;
  }

  carla::road::LaneSection *MapBuilder::AddRoadSection(
      Road *road,
      const SectionId id,
      const double s) {
    DEBUG_ASSERT(road != nullptr);
    carla::road::LaneSection &sec = road->_lane_sections.Emplace(id, s);
    sec._road = road;
    return &sec;
  }

  carla::road::Lane *MapBuilder::AddRoadSectionLane(
      carla::road::LaneSection *section,
      const int32_t lane_id,
      const uint32_t lane_type,
      const bool lane_level,
      const int32_t predecessor,
      const int32_t successor) {
    DEBUG_ASSERT(section != nullptr);

    // add the lane
    auto *lane = &((section->_lanes.emplace(lane_id, Lane()).first)->second);

    // set lane data
    lane->_id = lane_id;
    lane->_lane_section = section;
    lane->_level = lane_level;
    lane->_type = static_cast<carla::road::Lane::LaneType>(lane_type);
    lane->_successor = successor;
    lane->_predecessor = predecessor;

    return lane;
  }

  void MapBuilder::AddRoadGeometryLine(
      Road *road,
      const double s,
      const double x,
      const double y,
      const double hdg,
      const double length) {
    DEBUG_ASSERT(road != nullptr);
    const geom::Location location(static_cast<float>(x), static_cast<float>(y), 0.0f);
    auto line_geometry = std::make_unique<GeometryLine>(
        s,
        length,
        hdg,
        location);

    _temp_road_info_container[road].emplace_back(std::unique_ptr<RoadInfo>(new RoadInfoGeometry(s,
        std::move(line_geometry))));
  }

  void MapBuilder::CreateRoadSpeed(
      Road *road,
      const double s,
      const std::string /*type*/,
      const double max,
      const std::string /*unit*/) {
    DEBUG_ASSERT(road != nullptr);
    _temp_road_info_container[road].emplace_back(std::make_unique<RoadInfoSpeed>(s, max));
  }

  void MapBuilder::CreateSectionOffset(
      Road *road,
      const double s,
      const double a,
      const double b,
      const double c,
      const double d) {
    DEBUG_ASSERT(road != nullptr);
    _temp_road_info_container[road].emplace_back(std::make_unique<RoadInfoLaneOffset>(s, a, b, c, d));
  }

  void MapBuilder::AddRoadGeometryArc(
      Road *road,
      const double s,
      const double x,
      const double y,
      const double hdg,
      const double length,
      const double curvature) {
    DEBUG_ASSERT(road != nullptr);
    const geom::Location location(static_cast<float>(x), static_cast<float>(y), 0.0f);
    auto arc_geometry = std::make_unique<GeometryArc>(
        s,
        length,
        hdg,
        location,
        curvature);

    _temp_road_info_container[road].emplace_back(std::unique_ptr<RoadInfo>(new RoadInfoGeometry(s,
        std::move(arc_geometry))));
  }

  void MapBuilder::AddRoadGeometrySpiral(
      Road * road,
      const double s,
      const double x,
      const double y,
      const double hdg,
      const double length,
      const double curvStart,
      const double curvEnd) {
    //throw_exception(std::runtime_error("geometry spiral not supported"));
    DEBUG_ASSERT(road != nullptr);
    const geom::Location location(static_cast<float>(x), static_cast<float>(y), 0.0f);
    auto spiral_geometry = std::make_unique<GeometrySpiral>(
        s,
        length,
        hdg,
        location,
        curvStart,
        curvEnd);

      _temp_road_info_container[road].emplace_back(std::unique_ptr<RoadInfo>(new RoadInfoGeometry(s,
        std::move(spiral_geometry))));
  }

  void MapBuilder::AddRoadGeometryPoly3(
      Road * road,
      const double s,
      const double x,
      const double y,
      const double hdg,
      const double length,
      const double a,
      const double b,
      const double c,
      const double d) {
    //throw_exception(std::runtime_error("geometry poly3 not supported"));
    DEBUG_ASSERT(road != nullptr);
    const geom::Location location(static_cast<float>(x), static_cast<float>(y), 0.0f);
    auto poly3_geometry = std::make_unique<GeometryPoly3>(
        s,
        length,
        hdg,
        location,
        a,
        b,
        c,
        d);
    _temp_road_info_container[road].emplace_back(std::unique_ptr<RoadInfo>(new RoadInfoGeometry(s,
        std::move(poly3_geometry))));
  }

  void MapBuilder::AddRoadGeometryParamPoly3(
      Road * road,
      const double s,
      const double x,
      const double y,
      const double hdg,
      const double length,
      const double aU,
      const double bU,
      const double cU,
      const double dU,
      const double aV,
      const double bV,
      const double cV,
      const double dV,
      const std::string p_range) {
    //throw_exception(std::runtime_error("geometry poly3 not supported"));
    bool arcLength;
    if(p_range == "arcLength"){
      arcLength = true;
    }else{
      arcLength = false;
    }
    DEBUG_ASSERT(road != nullptr);
    const geom::Location location(static_cast<float>(x), static_cast<float>(y), 0.0f);
    auto parampoly3_geometry = std::make_unique<GeometryParamPoly3>(
        s,
        length,
        hdg,
        location,
        aU,
        bU,
        cU,
        dU,
        aV,
        bV,
        cV,
        dV,
        arcLength);
    _temp_road_info_container[road].emplace_back(std::unique_ptr<RoadInfo>(new RoadInfoGeometry(s,
        std::move(parampoly3_geometry))));
  }

  void MapBuilder::AddJunction(const int32_t id, const std::string name) {
    _map_data.GetJunctions().emplace(id, Junction(id, name));
  }

  void MapBuilder::AddConnection(
      const JuncId junction_id,
      const ConId connection_id,
      const RoadId incoming_road,
      const RoadId connecting_road) {
    DEBUG_ASSERT(_map_data.GetJunction(junction_id) != nullptr);
    _map_data.GetJunction(junction_id)->GetConnections().emplace(connection_id,
        Junction::Connection(connection_id, incoming_road, connecting_road));
  }

  void MapBuilder::AddLaneLink(
      const JuncId junction_id,
      const ConId connection_id,
      const LaneId from,
      const LaneId to) {
    DEBUG_ASSERT(_map_data.GetJunction(junction_id) != nullptr);
    _map_data.GetJunction(junction_id)->GetConnection(connection_id)->AddLaneLink(from, to);
  }

  void MapBuilder::AddJunctionController(
      const JuncId junction_id,
      std::set<road::ContId>&& controllers) {
    DEBUG_ASSERT(_map_data.GetJunction(junction_id) != nullptr);
    _map_data.GetJunction(junction_id)->_controllers = std::move(controllers);
  }

  Lane *MapBuilder::GetLane(
      const RoadId road_id,
      const LaneId lane_id,
      const double s) {
    return &_map_data.GetRoad(road_id).GetLaneByDistance(s, lane_id);
  }

  Road *MapBuilder::GetRoad(
      const RoadId road_id) {
    return &_map_data.GetRoad(road_id);
  }

  // return the pointer to a lane object
  Lane *MapBuilder::GetEdgeLanePointer(RoadId road_id, bool from_start, LaneId lane_id) {

    if (!_map_data.ContainsRoad(road_id)) {
      return nullptr;
    }
    Road &road = _map_data.GetRoad(road_id);

    // get the lane section
    LaneSection *section;
    if (from_start) {
      section = road.GetStartSection(lane_id);
    } else {
      section = road.GetEndSection(lane_id);
    }

    // get the lane
    DEBUG_ASSERT(section != nullptr);
    return section->GetLane(lane_id);
  }

  // return a list of pointers to all lanes from a lane (using road and junction
  // info)
  std::vector<Lane *> MapBuilder::GetLaneNext(
      RoadId road_id,
      SectionId section_id,
      LaneId lane_id) {
    std::vector<Lane *> result;

    if (!_map_data.ContainsRoad(road_id)) {
      return result;
    }
    Road &road = _map_data.GetRoad(road_id);

    // get the section
    LaneSection &section = road._lane_sections.GetById(section_id);

    // get the lane
    Lane *lane = section.GetLane(lane_id);
    DEBUG_ASSERT(lane != nullptr);

    // successor and predecessor (road and lane)
    LaneId next;
    RoadId next_road;
    if (lane_id <= 0) {
      next_road = road.GetSuccessor();
      next = lane->GetSuccessor();
    } else {
      next_road = road.GetPredecessor();
      next = lane->GetPredecessor();
    }

    // check to see if next is a road or a junction
    bool next_is_junction = !_map_data.ContainsRoad(next_road);
    double s = section.GetDistance();

    // check if we are in a lane section in the middle
    if ((lane_id > 0 && s > 0) ||
        (lane_id <= 0 && road._lane_sections.upper_bound(s) != road._lane_sections.end())) {
      // check if lane has a next link (if not, it deads in the middle section)
      if (next != 0 || (lane_id == 0 && next == 0)) {
        // change to next / prev section
        if (lane_id <= 0) {
          result.push_back(road.GetNextLane(s, next));
        } else {
          result.push_back(road.GetPrevLane(s, next));
        }
      }
    } else if (!next_is_junction) {
      // change to another road / junction
      if (next != 0 || (lane_id == 0 && next == 0)) {
        // single road
        result.push_back(GetEdgeLanePointer(next_road, (next <= 0), next));
      }
    } else {
      // several roads (junction)

      /// @todo Is it correct to use a road id as section id? (NS: I just added
      /// this cast to avoid compiler warnings).
      auto next_road_as_junction = static_cast<JuncId>(next_road);
      auto options = GetJunctionLanes(next_road_as_junction, road_id, lane_id);
      for (auto opt : options) {
        result.push_back(GetEdgeLanePointer(opt.first, (opt.second <= 0), opt.second));
      }
    }

    return result;
  }

  std::vector<std::pair<RoadId, LaneId>> MapBuilder::GetJunctionLanes(
      JuncId junction_id,
      RoadId road_id,
      LaneId lane_id) {
    std::vector<std::pair<RoadId, LaneId>> result;

    // get the junction
    Junction *junction = _map_data.GetJunction(junction_id);
    if (junction == nullptr) {
      return result;
    }

    // check all connections
    for (auto con : junction->_connections) {
      // only connections for our road
      if (con.second.incoming_road == road_id) {
        // for center lane it is always next lane id 0, we don't need to search
        // because it is not in the junction
        if (lane_id == 0) {
          result.push_back(std::make_pair(con.second.connecting_road, 0));
        } else {
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
    }

    return result;
  }

  // assign pointers to the next lanes
  void MapBuilder::CreatePointersBetweenRoadSegments(void) {
    // process each lane to define its nexts
    for (auto &road : _map_data._roads) {
      for (auto &section : road.second._lane_sections) {
        for (auto &lane : section.second._lanes) {

          // assign the next lane pointers
          lane.second._next_lanes = GetLaneNext(road.first, section.second._id, lane.first);

          // add to each lane found, this as its predecessor
          for (auto next_lane : lane.second._next_lanes) {
            // add as previous
            DEBUG_ASSERT(next_lane != nullptr);
            next_lane->_prev_lanes.push_back(&lane.second);
          }

        }
      }
    }

    // process each lane to define its nexts
    for (auto &road : _map_data._roads) {
      for (auto &section : road.second._lane_sections) {
        for (auto &lane : section.second._lanes) {

          // add next roads
          for (auto next_lane : lane.second._next_lanes) {
            DEBUG_ASSERT(next_lane != nullptr);
            // avoid same road
            if (next_lane->GetRoad() != &road.second) {
              if (std::find(road.second._nexts.begin(), road.second._nexts.end(),
                  next_lane->GetRoad()) == road.second._nexts.end()) {
                road.second._nexts.push_back(next_lane->GetRoad());
              }
            }
          }

          // add prev roads
          for (auto prev_lane : lane.second._prev_lanes) {
            DEBUG_ASSERT(prev_lane != nullptr);
            // avoid same road
            if (prev_lane->GetRoad() != &road.second) {
              if (std::find(road.second._prevs.begin(), road.second._prevs.end(),
                  prev_lane->GetRoad()) == road.second._prevs.end()) {
                road.second._prevs.push_back(prev_lane->GetRoad());
              }
            }
          }

        }
      }
    }
  }

  geom::Transform MapBuilder::ComputeSignalTransform(std::unique_ptr<Signal> &signal, MapData &data) {
    DirectedPoint point = data.GetRoad(signal->_road_id).GetDirectedPointInNoLaneOffset(signal->_s);
    point.ApplyLateralOffset(static_cast<float>(-signal->_t));
    point.location.y *= -1; // Unreal Y axis hack
    point.location.z += static_cast<float>(signal->_zOffset);
    geom::Transform transform(point.location, geom::Rotation(
        geom::Math::ToDegrees(static_cast<float>(signal->_pitch)),
        geom::Math::ToDegrees(static_cast<float>(-(point.tangent + signal->_hOffset))),
        geom::Math::ToDegrees(static_cast<float>(signal->_roll))));
    return transform;
  }

  void MapBuilder::SolveSignalReferencesAndTransforms() {
    for(auto signal_reference : _temp_signal_reference_container){
      signal_reference->_signal =
          _temp_signal_container[signal_reference->_signal_id].get();
    }

    for(auto& signal_pair : _temp_signal_container) {
      auto& signal = signal_pair.second;
      if (signal->_using_inertial_position) {
        continue;
      }
      auto transform = ComputeSignalTransform(signal, _map_data);
      if (SignalType::IsTrafficLight(signal->GetType())) {
        transform.location = transform.location +
            geom::Location(transform.GetForwardVector()*0.25);
      }
      signal->_transform = transform;
    }

    _map_data._signals = std::move(_temp_signal_container);

    GenerateDefaultValiditiesForSignalReferences();
  }

  void MapBuilder::SolveControllerAndJuntionReferences() {
    for(const auto& junction : _map_data._junctions) {
      for(const auto& controller : junction.second._controllers) {
        auto it = _map_data._controllers.find(controller);
        if(it != _map_data._controllers.end()){
          if( it->second != nullptr ){
            it->second->_junctions.insert(junction.first);
            for(const auto & signal : it->second->_signals) {
              auto signal_it = _map_data._signals.find(signal);
              if( signal_it->second != nullptr ){
                signal_it->second->_controllers.insert(controller);
              }
            }
          }
        }
      }
    }
  }

  void MapBuilder::CreateJunctionBoundingBoxes(Map &map) {
    for (auto &junctionpair : map._data.GetJunctions()) {
      auto* junction = map.GetJunction(junctionpair.first);
      auto waypoints = map.GetJunctionWaypoints(junction->GetId(), Lane::LaneType::Any);
      const int number_intervals = 10;

      float minx = std::numeric_limits<float>::max();
      float miny = std::numeric_limits<float>::max();
      float minz = std::numeric_limits<float>::max();
      float maxx = -std::numeric_limits<float>::max();
      float maxy = -std::numeric_limits<float>::max();
      float maxz = -std::numeric_limits<float>::max();

      auto get_min_max = [&](geom::Location position) {
        if (position.x < minx) {
          minx = position.x;
        }
        if (position.y < miny) {
          miny = position.y;
        }
        if (position.z < minz) {
          minz = position.z;
        }

        if (position.x > maxx) {
          maxx = position.x;
        }
        if (position.y > maxy) {
          maxy = position.y;
        }
        if (position.z > maxz) {
          maxz = position.z;
        }
      };

      for (auto &waypoint_p : waypoints) {
        auto &waypoint_start = waypoint_p.first;
        auto &waypoint_end = waypoint_p.second;
        double interval = (waypoint_end.s - waypoint_start.s) / static_cast<double>(number_intervals);
        auto next_wp = waypoint_end;
        auto location = map.ComputeTransform(next_wp).location;

        get_min_max(location);

        next_wp = waypoint_start;
        location = map.ComputeTransform(next_wp).location;

        get_min_max(location);

        for (int i = 0; i < number_intervals; ++i) {
          if (interval < std::numeric_limits<double>::epsilon())
            break;
          auto next = map.GetNext(next_wp, interval);
          if(next.size()){
            next_wp = next.back();
          }

          location = map.ComputeTransform(next_wp).location;
          get_min_max(location);
        }
      }
      carla::geom::Location location(0.5f * (maxx + minx), 0.5f * (maxy + miny), 0.5f * (maxz + minz));
      carla::geom::Vector3D extent(0.5f * (maxx - minx), 0.5f * (maxy - miny), 0.5f * (maxz - minz));

      junction->_bounding_box = carla::geom::BoundingBox(location, extent);
    }
  }

void MapBuilder::CreateController(
  const ContId controller_id,
  const std::string controller_name,
  const uint32_t controller_sequence,
  const std::set<road::SignId>&& signals) {

    // Add the Controller to MapData
    auto controller_pair = _map_data._controllers.emplace(
      std::make_pair(
          controller_id,
          std::make_unique<Controller>(controller_id, controller_name, controller_sequence)));

    DEBUG_ASSERT(controller_pair.first != _map_data._controllers.end());
    DEBUG_ASSERT(controller_pair.first->second);

    // Add the signals owned by the controller
    controller_pair.first->second->_signals = std::move(signals);

    // Add ContId to the signal owned by this Controller
    auto& signals_map = _map_data._signals;
    for(auto signal: signals) {
      auto it = signals_map.find(signal);
      if(it != signals_map.end()) {
        it->second->_controllers.insert(signal);
      }
    }
}

  void MapBuilder::ComputeJunctionRoadConflicts(Map &map) {
    for (auto &junctionpair : map._data.GetJunctions()) {
      auto& junction = junctionpair.second;
      junction._road_conflicts = (map.ComputeJunctionConflicts(junction.GetId()));
    }
  }

  void MapBuilder::GenerateDefaultValiditiesForSignalReferences() {
    for (auto * signal_reference : _temp_signal_reference_container) {
      if (signal_reference->_validities.size() == 0) {
        Road* road = GetRoad(signal_reference->GetRoadId());
        auto lanes = road->GetLanesByDistance(signal_reference->GetS());
        switch (signal_reference->GetOrientation()) {
          case SignalOrientation::Positive: {
            LaneId min_lane = 1;
            LaneId max_lane = 0;
            for (const auto* lane : lanes) {
              auto lane_id = lane->GetId();
              if(lane_id > max_lane) {
                max_lane = lane_id;
              }
            }
            if(min_lane <= max_lane) {
              AddValidityToSignalReference(signal_reference, min_lane, max_lane);
            }
            break;
          }
          case SignalOrientation::Negative: {
            LaneId min_lane = 0;
            LaneId max_lane = -1;
            for (const auto* lane : lanes) {
              auto lane_id = lane->GetId();
              if(lane_id < min_lane) {
                min_lane = lane_id;
              }
            }
            if(min_lane <= max_lane) {
              AddValidityToSignalReference(signal_reference, min_lane, max_lane);
            }
            break;
          }
          case SignalOrientation::Both: {
            // Get positive lanes
            LaneId min_lane = 1;
            LaneId max_lane = 0;
            for (const auto* lane : lanes) {
              auto lane_id = lane->GetId();
              if(lane_id > max_lane) {
                max_lane = lane_id;
              }
            }
            if(min_lane <= max_lane) {
              AddValidityToSignalReference(signal_reference, min_lane, max_lane);
            }

            // get negative lanes
            min_lane = 0;
            max_lane = -1;
            for (const auto* lane : lanes) {
              auto lane_id = lane->GetId();
              if(lane_id < min_lane) {
                min_lane = lane_id;
              }
            }
            if(min_lane <= max_lane) {
              AddValidityToSignalReference(signal_reference, min_lane, max_lane);
            }
            break;
          }
        }
      }
    }
  }

  void MapBuilder::RemoveZeroLaneValiditySignalReferences() {
    std::vector<element::RoadInfoSignal*> elements_to_remove;
    for (auto * signal_reference : _temp_signal_reference_container) {
      bool should_remove = true;
      for (auto & lane_validity : signal_reference->_validities) {
        if ( (lane_validity._from_lane != 0) ||
             (lane_validity._to_lane != 0)) {
          should_remove = false;
          break;
        }
      }
      if (signal_reference->_validities.size() == 0) {
        should_remove = false;
      }
      if (should_remove) {
        elements_to_remove.push_back(signal_reference);
      }
    }
    for (auto* element : elements_to_remove) {
      auto road_id = element->GetRoadId();
      auto& road_info = _temp_road_info_container[GetRoad(road_id)];
      road_info.erase(std::remove_if(road_info.begin(), road_info.end(),
          [=] (auto& info_ptr) {
            return (info_ptr.get() == element);
          }), road_info.end());
      _temp_signal_reference_container.erase(std::remove(_temp_signal_reference_container.begin(),
          _temp_signal_reference_container.end(), element),
          _temp_signal_reference_container.end());
    }
  }

  void MapBuilder::CheckSignalsOnRoads(Map &map) {
    for (auto& signal_pair : map._data._signals) {
      auto& signal = signal_pair.second;
      auto signal_position = signal->GetTransform().location;
      auto signal_rotation = signal->GetTransform().rotation;
      auto closest_waypoint_to_signal =
          map.GetClosestWaypointOnRoad(signal_position,
          static_cast<int32_t>(carla::road::Lane::LaneType::Shoulder) |  static_cast<int32_t>(carla::road::Lane::LaneType::Driving));
      // workarround to not move stencil stop
      if (
          signal->GetName().find("Stencil_STOP") != std::string::npos ||
          signal->GetName().find("STATIC") != std::string::npos ||
          signal->_using_inertial_position) {
        continue;
      }
      if(closest_waypoint_to_signal) {
        auto road_transform = map.ComputeTransform(closest_waypoint_to_signal.value());
        auto distance_to_road = (road_transform.location -signal_position).Length();
        double lane_width = map.GetLaneWidth(closest_waypoint_to_signal.value());
        int displacement_direction = 1;
        int iter = 0;
        int MaxIter = 10;
        // Displaces signal until it finds a suitable spot
        while(distance_to_road < (lane_width * 0.7) && iter < MaxIter && displacement_direction != 0) {
          if(iter == 0) {
            log_debug("Traffic sign",
                signal->GetSignalId(),
                "overlaps a driving lane. Moving out of the road...");
          }

          auto right_waypoint = map.GetRight(closest_waypoint_to_signal.value());
          auto right_lane_type = (right_waypoint) ? map.GetLaneType(right_waypoint.value()) : carla::road::Lane::LaneType::None;

          auto left_waypoint = map.GetLeft(closest_waypoint_to_signal.value());
          auto left_lane_type = (left_waypoint) ? map.GetLaneType(left_waypoint.value()) : carla::road::Lane::LaneType::None;

          if (right_lane_type != carla::road::Lane::LaneType::Driving) {
            displacement_direction = 1;
          } else if (left_lane_type != carla::road::Lane::LaneType::Driving) {
            displacement_direction = -1;
          } else {
            displacement_direction = 0;
          }

          geom::Vector3D displacement = 1.f*(road_transform.GetRightVector()) *
              static_cast<float>(abs(lane_width))*0.2f;
          signal_position += (displacement * displacement_direction);
          signal_rotation = road_transform.rotation;
          closest_waypoint_to_signal =
              map.GetClosestWaypointOnRoad(signal_position,
              static_cast<int32_t>(carla::road::Lane::LaneType::Shoulder) |  static_cast<int32_t>(carla::road::Lane::LaneType::Driving));
          distance_to_road =
              (map.ComputeTransform(closest_waypoint_to_signal.value()).location -
              signal_position).Length();
          lane_width = map.GetLaneWidth(closest_waypoint_to_signal.value());
          iter++;
        }
        if(iter == MaxIter) {
          log_debug("Failed to find suitable place for signal.");
        } else {
          // Only perform the displacement if a good location has been found
          signal->_transform.location = signal_position;
          signal->_transform.rotation = signal_rotation;
        }
      }
    }
  }

} // namespace road
} // namespace carla
