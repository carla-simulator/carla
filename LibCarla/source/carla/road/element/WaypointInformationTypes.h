// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/road/element/LaneMarking.h"
#include <cstdint>

namespace carla {
namespace road {
namespace element {

  class RoadInfoMarkRecord;

  struct WaypointInfoRoadMark {

    // WaypointInfoRoadMark() {}

    WaypointInfoRoadMark(const RoadInfoMarkRecord &info);

    LaneMarking type = LaneMarking::None;

    enum class Color : uint8_t {
      Standard, // (equivalent to "white")
      Blue,
      Green,
      Red,
      White = Standard,
      Yellow,
      Other
    } color = Color::Standard;

    enum class LaneChange : uint8_t {
      Increase,
      Decrease,
      Both,
      None
    } lane_change = LaneChange::None;

    float width = 0.0f;
  };

} // namespace element
} // namespace road
} // namespace carla
