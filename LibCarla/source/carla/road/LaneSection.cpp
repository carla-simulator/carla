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

  const Road *LaneSection::GetRoad() const {
    return _road;
  }

  const Lane *LaneSection::GetLane(const LaneId id) const {
    const auto search = _lanes.find(id);
    if (search != _lanes.end()) {
      return &search->second;
    }
    return nullptr;
  }

  std::unordered_map<LaneId, Lane> &LaneSection::GetLanes() {
    return _lanes;
  }

} // namespace road
} // namespace carla
