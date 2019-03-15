// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "carla/road/Lane.h"
#include "carla/road/Road.h"

namespace carla {
namespace road {

  const LaneSection *Lane::GetLaneSection() const {
    return _lane_section;
  }

  const Road *Lane::GetRoad() const {
    return _lane_section->GetRoad();
  }

  LaneId Lane::GetId() const {
    return _id;
  }

  std::string Lane::GetType() const {
    return _type;
  }

  bool Lane::GetLevel() const {
    return _level;
  }

} // road
} // carla
