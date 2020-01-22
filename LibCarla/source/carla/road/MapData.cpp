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

  Road &MapData::GetRoad(const RoadId id) {
    return _roads.at(id);
  }

  const Road &MapData::GetRoad(const RoadId id) const {
    return const_cast<MapData *>(this)->GetRoad(id);
  }

  Junction *MapData::GetJunction(JuncId id) {
    const auto search = _junctions.find(id);
    if (search != _junctions.end()) {
      return &search->second;
    }
    return nullptr;
  }

  const Junction *MapData::GetJunction(JuncId id) const {
    const auto search = _junctions.find(id);
    if (search != _junctions.end()) {
      return &search->second;
    }
    return nullptr;
  }
} // namespace road
} // namespace carla
