// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/Buffer.h"

namespace carla {
namespace sensor {
namespace s11n {

  class ImageSerializer {
  public:

    template <typename Sensor>
    static Buffer Serialize(const Sensor &, Buffer bitmap) {
      return bitmap; /// @todo
    }
  };

} // namespace s11n
} // namespace sensor
} // namespace carla
