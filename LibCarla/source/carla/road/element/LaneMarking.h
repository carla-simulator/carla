// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

namespace carla {
namespace road {
namespace element {

  enum class LaneMarking {
    Other,
    Broken,
    Solid,
    SolidSolid, // (for double solid line)
    SolidBroken, // (from inside to outside, exception: center lane -from left to right)
    BrokenSolid, // (from inside to outside, exception: center lane -from left to right)
    BrokenBroken, // (from inside to outside, exception: center lane -from left to right)
    BottsDots,
    Grass, // (meaning a grass edge)
    Curb,
    None
  };

} // namespace element
} // namespace road
} // namespace carla
