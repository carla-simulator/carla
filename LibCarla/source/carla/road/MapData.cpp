// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "carla/road/MapData.h"
#include "carla/road/Lane.h"

namespace carla {
namespace road {

  std::unordered_map<RoadId, Road> &MapData::GetRoads() {
    return _roads;
  }

  std::unordered_map<JuncId, Junction> &MapData::GetJunctions() {
    return _junctions;
  }

  Road *MapData::GetRoad(const RoadId id) {
    const auto search = _roads.find(id);
    if (search != _roads.end()) {
      return &search->second;
    }
    return nullptr;
  }

  Junction *MapData::GetJunction(JuncId id) {
    const auto search = _junctions.find(id);
    if (search != _junctions.end()) {
      return &search->second;
    }
    return nullptr;
  }

  Lane *MapData::GetLane(
      const RoadId road_id,
      const LaneId lane_id,
      const float s) {
    auto road = GetRoad(road_id);
    if (road != nullptr) {
      return road->GetLane(lane_id, s);
    }
    return nullptr;
  }

  const Lane *MapData::GetLane(
      const RoadId road_id,
      const LaneId lane_id,
      const float s) const {
    return const_cast<MapData *>(this)->GetLane(road_id, lane_id, s);
  }

} // namespace road
} // namespace carla
