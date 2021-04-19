// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/rpc/Color.h"

#include <cstdint>

namespace carla {
namespace sensor {
namespace data {

#pragma pack(push, 1)
  /// A 32-bit BGRA color.
  struct Color {
    Color() = default;
    Color(const Color &) = default;

    Color(uint8_t r, uint8_t g, uint8_t b, uint8_t a = 255u)
      : b(b), g(g), r(r), a(a) {}

    Color &operator=(const Color &) = default;

    bool operator==(const Color &rhs) const  {
      return (r == rhs.r) && (g == rhs.g) && (b == rhs.b);
    }

    bool operator!=(const Color &rhs) const  {
      return !(*this == rhs);
    }

    operator rpc::Color() const {
      return {r, g, b};
    }

    uint8_t b = 0u;
    uint8_t g = 0u;
    uint8_t r = 0u;
    uint8_t a = 0u;
  };
#pragma pack(pop)

  static_assert(sizeof(Color) == sizeof(uint32_t), "Invalid color size!");

#pragma pack(push, 1)
  /// A 64-bit PF_FloatRGBA color [16bit / channel]. (still uses uint16 types to transport 16bit floats)
  struct Color16bit {
    Color16bit() = default;
    Color16bit(const Color16bit &) = default;

    Color16bit(uint16_t r, uint16_t g, uint16_t b, uint16_t a = 255u)
      : b(b), g(g), r(r), a(a) {}

    Color16bit &operator=(const Color16bit &) = default;

    bool operator==(const Color16bit &rhs) const  {
      return (r == rhs.r) && (g == rhs.g) && (b == rhs.b) && (a == rhs.a);
    }

    bool operator!=(const Color16bit &rhs) const  {
      return !(*this == rhs);
    }

//    operator rpc::Color16bit() const {
//      return {r, g, b};
//    }

    uint16_t b = 0u;
    uint16_t g = 0u;
    uint16_t r = 0u;
    uint16_t a = 0u;
  };
#pragma pack(pop)

  static_assert(sizeof(Color16bit) == sizeof(uint64_t), "Invalid 16bit color size!");

} // namespace data
} // namespace sensor
} // namespace carla
