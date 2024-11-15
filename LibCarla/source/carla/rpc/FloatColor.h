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

#pragma pack(push, 1)
  struct FloatColor {
  public:

    float r = 0.f;
    float g = 0.f;
    float b = 0.f;
    float a = 1.f;

    FloatColor() = default;
    FloatColor(const FloatColor &) = default;

    FloatColor(float r, float g, float b, float a = 1.f)
      : r(r), g(g), b(b), a(a) {}

    FloatColor &operator=(const FloatColor &) = default;

    bool operator==(const FloatColor &rhs) const  {
      return (r == rhs.r) && (g == rhs.g) && (b == rhs.b) && (a == rhs.a);
    }

    bool operator!=(const FloatColor &rhs) const  {
      return !(*this == rhs);
    }

#ifdef LIBCARLA_INCLUDED_FROM_UE4

    FloatColor(const FColor &color)
      : FloatColor(color.R / 255.f, color.G / 255.f, color.B / 255.f, color.A / 255.f) {}

    FloatColor(const FLinearColor &color)
      : FloatColor(color.R, color.G, color.B, color.A) {}

    FColor ToFColor() const {
      return FColor{
        static_cast<uint8>(r * 255),
        static_cast<uint8>(g * 255),
        static_cast<uint8>(b * 255),
        static_cast<uint8>(a * 255)};
    }

    operator FLinearColor() const {
      return FLinearColor{ r, g, b, a };
    }

#endif // LIBCARLA_INCLUDED_FROM_UE4

    MSGPACK_DEFINE_ARRAY(r, g, b, a);
  };
#pragma pack(pop)

} // namespace rpc
} // namespace carla
