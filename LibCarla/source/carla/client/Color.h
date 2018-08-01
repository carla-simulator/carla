// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include <cstdint>

namespace carla {
namespace client {

  struct Color {
    Color() = default;
    Color(uint8_t r, uint8_t g, uint8_t b) : r(r), g(g), b(b) {}

    bool operator==(const Color &rhs) const  {
      return (r == rhs.r) && (g == rhs.g) && (b == rhs.b);
    }

    bool operator!=(const Color &rhs) const  {
      return !(*this == rhs);
    }

    uint8_t r = 0u;
    uint8_t g = 0u;
    uint8_t b = 0u;
  };

} // namespace client
} // namespace carla
