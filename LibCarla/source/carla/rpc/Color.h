// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/MsgPack.h"

#include <cstdint>

#ifdef LIBCARLA_INCLUDED_FROM_UE4
#  include "Math/Color.h"
#endif // LIBCARLA_INCLUDED_FROM_UE4

namespace carla {
namespace rpc {

  class Color {
  public:

    uint8_t r = 0u;
    uint8_t g = 0u;
    uint8_t b = 0u;

    Color() = default;
    Color(const Color &) = default;

    Color(uint8_t r, uint8_t g, uint8_t b)
      : r(r), g(g), b(b) {}

    Color &operator=(const Color &) = default;

#ifdef LIBCARLA_INCLUDED_FROM_UE4

    Color(const FColor &color)
      : Color(color.R, color.G, color.B) {}

    operator FColor() const {
      return FColor{r, g, b};
    }

#endif // LIBCARLA_INCLUDED_FROM_UE4

    MSGPACK_DEFINE_ARRAY(r, g, b);
  };

} // namespace rpc
} // namespace carla
