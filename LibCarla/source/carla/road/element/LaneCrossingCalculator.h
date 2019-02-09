// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/road/element/LaneMarking.h"

#include <vector>

namespace carla {
namespace geom { class Location; }
namespace road {

  class Map;

namespace element {

  class LaneCrossingCalculator {
  public:

    static std::vector<LaneMarking> Calculate(
        const Map &map,
        const geom::Location &origin,
        const geom::Location &destination);
  };

} // namespace element
} // namespace road
} // namespace carla
