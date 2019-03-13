// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/NonCopyable.h"

namespace carla {
namespace road {

  class LaneSection : private NonCopyable {
  public:

    LaneSection() {}

  private:

    float _s;

  };

} // road
} // carla
