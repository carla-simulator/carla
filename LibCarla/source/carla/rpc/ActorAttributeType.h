// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include <cstdint>

namespace carla {
namespace rpc {

  enum class ActorAttributeType : uint8_t {
    Bool,
    Int,
    Float,
    String,
    RGBColor,

    SIZE,
    INVALID
  };

  // Serialization of this class is in ActorAttribute.h, to reduce dependencies
  // since this file is directly included in UE4 code.

} // namespace rpc
} // namespace carla
