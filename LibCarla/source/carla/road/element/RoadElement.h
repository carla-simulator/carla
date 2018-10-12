// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/geom/Location.h"

namespace carla {
namespace road {
namespace element {

  struct Direction {
    double angle = 0.0; // [radians]
  };

  class RoadElement {
  public:

    geom::Location GetLocation() const {
      return _loc;
    }
    Direction GetDirection() const {
      return _dir;
    }

  private:

    geom::Location _loc;
    Direction _dir;
  };

} // namespace element
} // namespace road
} // namespace carla
