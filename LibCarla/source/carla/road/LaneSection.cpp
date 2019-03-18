// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "carla/road/LaneSection.h"

namespace carla {
namespace road {

  float LaneSection::GetDistance() const {
    return _s;
  }

  Road *LaneSection::GetRoad() {
    return _road;
  }

  Lane *LaneSection::GetLane(const LaneId id) {
    const auto search = _lanes.find(id);
    if (search != _lanes.end()) {
      return &search->second;
    }
    return nullptr;
  }

  std::unordered_map<LaneId, Lane> &LaneSection::GetLanes() {
    return _lanes;
  }

  std::vector<Lane *> LaneSection::GetLanesOfType(const std::string &type) {
    std::vector<Lane *> drivable_lanes;
    for (auto &&lane : _lanes) {
      if (lane.second.GetType() == type) {
        drivable_lanes.emplace_back(&lane.second);
      }
    }
    return drivable_lanes;
  }

} // namespace road
} // namespace carla
