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

    WaypointInfoRoadMark(const RoadInfoMarkRecord &info);

    LaneMarking type = LaneMarking::None;

    enum class Color : uint8_t {
      Standard = 0u, // (equivalent to "white")
      Blue     = 1u,
      Green    = 2u,
      Red      = 3u,
      White    = Standard,
      Yellow   = 4u,
      Other    = 5u
    } color = Color::Standard;

    /// Can be used as flags
    enum class LaneChange : uint8_t {
      None  = 0x00, // 00
      Right = 0x01, // 01
      Left  = 0x02, // 10
      Both  = 0x03  // 11
    } lane_change = LaneChange::None;

    double width = 0.0;
  };

} // namespace element
} // namespace road
} // namespace carla
