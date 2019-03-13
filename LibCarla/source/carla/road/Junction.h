// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/NonCopyable.h"

namespace carla {
namespace road {

  class Junction : private NonCopyable {
  public:

    using IdType = uint32_t;

    Junction() {}

  private:

    IdType _id;

  };

} // road
} // carla
