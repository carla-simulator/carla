// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "carla/road/MapBuilder.h"
#include "carla/road/element/RoadInfoVisitor.h"

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

} // namespace road
} // namespace carla
