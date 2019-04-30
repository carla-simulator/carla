// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "carla/Debug.h"
#include "carla/road/Lane.h"
#include "carla/road/LaneSection.h"
#include "carla/road/Road.h"

#include <limits>

namespace carla {
namespace road {

  const LaneSection *Lane::GetLaneSection() const {
    return _lane_section;
  }

  Road *Lane::GetRoad() const {
    DEBUG_ASSERT(_lane_section != nullptr);
    return _lane_section->GetRoad();
  }

  LaneId Lane::GetId() const {
    return _id;
  }

  Lane::LaneType Lane::GetType() const {
    return _type;
  }

  bool Lane::GetLevel() const {
    return _level;
  }

  double Lane::GetDistance() const {
    DEBUG_ASSERT(_lane_section != nullptr);
    return _lane_section->GetDistance();
  }

  double Lane::GetLength() const {
    const auto *road = GetRoad();
    DEBUG_ASSERT(road != nullptr);
    const auto s = GetDistance();
    return road->UpperBound(s) - s;
  }

} // road
} // carla
