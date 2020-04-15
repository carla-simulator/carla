// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "carla/road/LaneSection.h"
#include "carla/road/Road.h"

namespace carla {
namespace road {

  double LaneSection::GetDistance() const {
    return _s;
  }

  double LaneSection::GetLength() const {
    const auto *road = GetRoad();
    DEBUG_ASSERT(road != nullptr);
    return road->UpperBound(_s) - _s;
  }

  Road *LaneSection::GetRoad() const {
    return _road;
  }

  SectionId LaneSection::GetId() const {
    return _id;
  }

  Lane *LaneSection::GetLane(const LaneId id) {
    auto search = _lanes.find(id);
    if (search != _lanes.end()) {
      return &search->second;
    }
    return nullptr;
  }

  const Lane *LaneSection::GetLane(const LaneId id) const {
    auto search = _lanes.find(id);
    if (search != _lanes.end()) {
      return &search->second;
    }
    return nullptr;
  }

  std::map<LaneId, Lane> &LaneSection::GetLanes() {
    return _lanes;
  }

  const std::map<LaneId, Lane> &LaneSection::GetLanes() const {
    return _lanes;
  }

  std::vector<Lane *> LaneSection::GetLanesOfType(Lane::LaneType lane_type) {
    std::vector<Lane *> drivable_lanes;
    for (auto &&lane : _lanes) {
      if ((static_cast<uint32_t>(lane.second.GetType()) & static_cast<uint32_t>(lane_type)) > 0) {
        drivable_lanes.emplace_back(&lane.second);
      }
    }
    return drivable_lanes;
  }

} // namespace road
} // namespace carla
