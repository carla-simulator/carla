// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "carla/road/MapBuilder.h"
#include "carla/road/element/RoadInfoVisitor.h"
#include <iterator>

using namespace carla::road::element;

namespace carla {
namespace road {

  boost::optional<Map> MapBuilder::Build() {

    SetTotalRoadSegmentLength();

    CreatePointersBetweenRoadSegments();

    // _map_data is a memeber of MapBuilder so you must especify if
    // you want to keep it (will return copy -> Map(const Map &))
    // or move it (will return move -> Map(Map &&))
    return Map{std::move(_map_data)};
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

    _map_data.GetRoad(road_id)->getSignals().emplace(signal_id, signal::Signal(road_id, signal_id, s, t, name, dynamic,
        orientation, zOffset, country, type, subtype, value, unit, height, width,
        text, hOffset, pitch, roll));
  }

  void MapBuilder::AddValidityToLastAddedSignal(uint32_t road_id, uint32_t signal_id, int32_t from_lane, int32_t to_lane) {
    _map_data.GetRoad(road_id)->GetSignal(signal_id).AddValidity(general::Validity(signal_id, from_lane, to_lane));
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
    // we save id as pointers temporally, later will be processed in the right way
    lane->_next_lanes.emplace_back(reinterpret_cast<Lane *>(successor));
    lane->_prev_lanes.emplace_back(reinterpret_cast<Lane *>(predecessor));
  }

} // namespace road
} // namespace carla
