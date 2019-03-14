// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "carla/road/Lane.h"

namespace carla {
namespace road {

  LaneSection *Lane::GetLaneSection() const {
    return _lane_section;
  }

  LaneId Lane::GetId() const {
    return _id;
  }

} // road
} // carla
