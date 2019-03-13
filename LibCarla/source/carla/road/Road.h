// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/NonCopyable.h"
#include "carla/road/LaneSection.h"
#include "carla/road/Junction.h"

#include <vector>

namespace carla {
namespace road {

  class Road : private NonCopyable {
  public:

    using IdType = uint32_t;

    Road() {}

  private:

    const IdType _id;
    const Junction::IdType _junction_id;
    std::vector<const LaneSection *>
  };

} // road
} // carla
