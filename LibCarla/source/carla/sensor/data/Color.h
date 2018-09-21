// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include <cstdint>

namespace carla {
namespace sensor {
namespace data {

#pragma pack(push, 1)
    struct Color {
      uint8_t b;
      uint8_t g;
      uint8_t r;
      uint8_t a;
    };
#pragma pack(pop)

    static_assert(sizeof(Color) == sizeof(uint32_t), "Invalid color size!");

} // namespace data
} // namespace sensor
} // namespace carla
