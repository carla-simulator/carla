// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/Buffer.h"

#include <cstdint>
#include <cstring>

namespace carla {
namespace sensor {
namespace s11n {

  class ImageSerializer {

#pragma pack(push, 1)
    struct ImageHeader {
      uint32_t width;
      uint32_t height;
      uint32_t type;
      float fov;
    };
#pragma pack(pop)

  public:

    constexpr static auto offset_size = sizeof(ImageHeader);

    template <typename Sensor>
    static Buffer Serialize(const Sensor &sensor, Buffer bitmap) {
      DEBUG_ASSERT(bitmap.size() > sizeof(ImageHeader));
      ImageHeader header = {
        sensor.GetImageWidth(),
        sensor.GetImageHeight(),
        0u, /// @todo
        sensor.GetFOVAngle()
      };
      std::memcpy(bitmap.data(), reinterpret_cast<const void *>(&header), sizeof(header));
      return bitmap;
    }
  };

} // namespace s11n
} // namespace sensor
} // namespace carla
