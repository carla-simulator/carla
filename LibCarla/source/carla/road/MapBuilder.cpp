// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "carla/StringUtil.h"
#include "carla/road/MapBuilder.h"
#include "carla/road/element/RoadInfoElevation.h"
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
#include "carla/road/element/RoadInfoVelocity.h"
#include "carla/road/element/RoadInfoVisitor.h"

#include <iterator>
#include <memory>
#include <vector>

using namespace carla::road::element;

namespace carla {
namespace road {

  boost::optional<Map> MapBuilder::Build() {

    SetTotalRoadSegmentLength();

    CreatePointersBetweenRoadSegments();

    // _map_data is a member of MapBuilder so you must especify if
    // you want to keep it (will return copy -> Map(const Map &))
    // or move it (will return move -> Map(Map &&))
    return Map{std::move(_map_data)};
  }

  // called from profiles parser
  void MapBuilder::AddRoadElevationProfile(
      const RoadId road_id,
      const double s,
      const double a,
      const double b,
      const double c,
      const double d) {

    Road* road = _map_data.GetRoad(road_id);
    if (road != nullptr)
    {
      auto data = std::unique_ptr<RoadInfoElevation>(new RoadInfoElevation(s, a, b, c, d));
    }
  }

  // called from lane parser
  void MapBuilder::CreateLaneAccess(
      const RoadId road_id,
      const float lane_section_id,
      const LaneId lane_id,
      const float s,
      const std::string restriction) {

    Lane* lane = _map_data.GetLane(road_id, lane_section_id, lane_id);
    auto data = std::unique_ptr<RoadInfoLaneAccess>(new RoadInfoLaneAccess(s, restriction));
    lane->_road_info_cache.push_back(std::move(data));
  }

  void MapBuilder::CreateLaneBorder(
      const RoadId road_id,
      const float lane_section_id,
      const LaneId lane_id,
      const float s,
      const float a,
      const float b,
      const float c,
      const float d) {

    Lane* lane = _map_data.GetLane(road_id, lane_section_id, lane_id);
    auto data = std::unique_ptr<RoadInfoLaneBorder>(new RoadInfoLaneBorder(s, a, b, c, d));
    lane->_road_info_cache.push_back(std::move(data));
  }

  void MapBuilder::CreateLaneHeight(
      const RoadId road_id,
      const float lane_section_id,
      const LaneId lane_id,
      const float s,
      const float inner,
      const float outer) {

    Lane* lane = _map_data.GetLane(road_id, lane_section_id, lane_id);
    auto data = std::unique_ptr<RoadInfoLaneHeight>(new RoadInfoLaneHeight(s, inner, outer));
    lane->_road_info_cache.push_back(std::move(data));
  }

  void MapBuilder::CreateLaneMaterial(
      const RoadId road_id,
      const float lane_section_id,
      const LaneId lane_id,
      const float s,
      const std::string surface,
      const float friction,
      const float roughness) {

    Lane* lane = _map_data.GetLane(road_id, lane_section_id, lane_id);
    auto data = std::unique_ptr<RoadInfoLaneMaterial>(new RoadInfoLaneMaterial(s, surface, friction, roughness));
    lane->_road_info_cache.push_back(std::move(data));
  }

  void MapBuilder::CreateLaneOffset(
      const RoadId road_id,
      const float lane_section_id,
      const LaneId lane_id,
      const float s,
      const float a,
      const float b,
      const float c,
      const float d) {

    Lane* lane = _map_data.GetLane(road_id, lane_section_id, lane_id);
    auto data = std::unique_ptr<RoadInfoLaneOffset>(new RoadInfoLaneOffset(s, a, b, c, d));
    lane->_road_info_cache.push_back(std::move(data));
  }

  void MapBuilder::CreateLaneRule(
      const RoadId road_id,
      const float lane_section_id,
      const LaneId lane_id,
      const float s,
      const std::string value) {

    Lane* lane = _map_data.GetLane(road_id, lane_section_id, lane_id);
    auto data = std::unique_ptr<RoadInfoLaneRule>(new RoadInfoLaneRule(s, value));
    lane->_road_info_cache.push_back(std::move(data));
  }

  void MapBuilder::CreateLaneVisibility(
      const RoadId road_id,
      const float lane_section_id,
      const LaneId lane_id,
      const float s,
      const float forward,
      const float back,
      const float left,
      const float right) {

    Lane* lane = _map_data.GetLane(road_id, lane_section_id, lane_id);
    auto data = std::unique_ptr<RoadInfoLaneVisibility>(new RoadInfoLaneVisibility(s, forward, back, left, right));
    lane->_road_info_cache.push_back(std::move(data));
  }

  void MapBuilder::CreateLaneWidth(
      const RoadId road_id,
      const float lane_section_id,
      const LaneId lane_id,
      const float s,
      const float a,
      const float b,
      const float c,
      const float d) {

    Lane* lane = _map_data.GetLane(road_id, lane_section_id, lane_id);
    auto data = std::unique_ptr<RoadInfoLaneWidth>(new RoadInfoLaneWidth(s, a, b, c, d));
    lane->_road_info_cache.push_back(std::move(data));
  }

  void MapBuilder::CreateRoadMark(
      const RoadId road_id,
      const float lane_section_id,
      const LaneId lane_id,
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

    Lane* lane = _map_data.GetLane(road_id, lane_section_id, lane_id);

    RoadInfoMarkRecord::LaneChange lc;

    auto ToLower = [](auto str) {
      return StringUtil::ToLowerCopy(str);
    };

    if (ToLower(lane_change) == "increase") {
      lc = RoadInfoMarkRecord::LaneChange::Increase;
    } else if (ToLower(lane_change) == "decrease") {
      lc = RoadInfoMarkRecord::LaneChange::Decrease;
    } else if (ToLower(lane_change) == "both") {
      lc = RoadInfoMarkRecord::LaneChange::Both;
    } else {
      lc = RoadInfoMarkRecord::LaneChange::None;
    }
    auto data = std::unique_ptr<RoadInfoMarkRecord>(new RoadInfoMarkRecord(s, road_mark_id, type, weight, color,
        material, width, lc, height, type_name, type_width));
    lane->_road_info_cache.push_back(std::move(data));
  }

  void MapBuilder::CreateRoadMarkTypeLine(
      const RoadId road_id,
      const float lane_section_id,
      const LaneId lane_id,
      const int road_mark_id,
      const float length,
      const float space,
      const float tOffset,
      const float s,
      const std::string rule,
      const float width) {

    Lane* lane = _map_data.GetLane(road_id, lane_section_id, lane_id);
    auto it = MakeRoadInfoIterator<RoadInfoMarkRecord>(lane->_road_info_cache);
    for (; !it.IsAtEnd(); ++it) {
      if(it->GetRoadMarkId() == road_mark_id) {
        auto data = std::unique_ptr<RoadInfoMarkTypeLine>(new RoadInfoMarkTypeLine(s, road_mark_id, length, space,
        tOffset, rule, width));
        it->GetLines().push_back(std::move(data));
        break;
      }
    }
  }

  void MapBuilder::CreateLaneSpeed(
      const RoadId road_id,
      const float lane_section_id,
      const LaneId lane_id,
      const float s,
      const float max,
      const std::string unit) {

    Lane* lane = _map_data.GetLane(road_id, lane_section_id, lane_id);
    auto data = std::unique_ptr<RoadInfoVelocity>(new RoadInfoVelocity(s, max, unit));
    lane->_road_info_cache.push_back(std::move(data));
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

    _map_data.GetRoad(road_id)->getSignals().emplace(signal_id,
        signal::Signal(road_id, signal_id, s, t, name, dynamic,
        orientation, zOffset, country, type, subtype, value, unit, height, width,
        text, hOffset, pitch, roll));
  }

  void MapBuilder::AddValidityToLastAddedSignal(
      uint32_t road_id,
      uint32_t signal_id,
      int32_t from_lane,
      int32_t to_lane) {
    _map_data.GetRoad(road_id)->GetSignal(signal_id).AddValidity(general::Validity(signal_id, from_lane,
        to_lane));
  }

  // build road objects
  void MapBuilder::AddRoad(
      const uint32_t road_id,
      const std::string name,
      const double length,
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
  }

  void MapBuilder::AddRoadSection(
      const uint32_t road_id,
      geom::CubicPolynomial cubic) {

    // get the road
    Road *road = _map_data.GetRoad(road_id);
    if (road == nullptr) {
      log_warning("Road %d not found (Mapbuilder adding section)", road_id);
      return;
    }

    // add it
    LaneSection *sec = &(road->_lane_sections.emplace(cubic.GetS(), LaneSection()))->second;

    // set section data
    sec->_road = road;
    sec->_s = cubic.GetS();
    sec->_lane_offset = cubic;
  }

  void MapBuilder::AddRoadSectionLane(
      const uint32_t road_id,
      const uint32_t section_index,
      const int32_t lane_id,
      const std::string lane_type,
      const bool lane_level,
      const int32_t predecessor,
      const int32_t successor) {

    // get the road
    Road *road = _map_data.GetRoad(road_id);
    if (road == nullptr) {
      log_warning("Road %d not found (Mapbuilder adding lane)", road_id);
      return;
    }

    // get the section
    if (section_index >= road->_lane_sections.size()) {
      log_warning("LaneSection %d not found (Mapbuilder adding lane)", section_index);
      return;
    }
    auto it = road->_lane_sections.begin();
    std::advance(it, section_index);
    LaneSection *section = &(it->second);

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
  }

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

} // namespace road
} // namespace carla
