// Copyright (c) 2024 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/MsgPack.h"

#include <cstdint>

#ifdef LIBCARLA_INCLUDED_FROM_UE4
#include <util/enable-ue4-macros.h>
#include "Math/Color.h"
#include <util/disable-ue4-macros.h>
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

    Color(const FLinearColor &color)
      : Color(color.R * 255.0f, color.G  * 255.0f, color.B  * 255.0f) {}

    operator FColor() const {
      return FColor{r, g, b};
    }

    operator FLinearColor() const {
      return FLinearColor{
        static_cast<float>(r)/255.0f,
        static_cast<float>(g)/255.0f,
        static_cast<float>(b)/255.0f,
        1.0f
      };
    }

#endif // LIBCARLA_INCLUDED_FROM_UE4

    MSGPACK_DEFINE_ARRAY(r, g, b);
  };

} // namespace rpc
} // namespace carla
