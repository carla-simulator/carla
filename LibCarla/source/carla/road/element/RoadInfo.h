// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "RoadSegment.h"

namespace carla {
namespace road {
namespace element {
  class RoadInfo {
  public:

    RoadInfo() {}
    RoadInfo(double distance) : d(distance) {}
    virtual ~RoadInfo() = default;

    // distance from Road's start location
    double d = 0; // [meters]
  };

} // namespace element
} // namespace road
} // namespace carla
