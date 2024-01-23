// Copyright (c) 2024 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/NonCopyable.h"
#include <string>
#include <vector>
#include "carla/road/RoadTypes.h"

namespace carla {
namespace road {
namespace object {

  class RepeatRecord : private MovableNonCopyable {
  public:

    RepeatRecord() {}

  private:

    ObjId _id;
  };

} // object
} // road
} // carla
