// Copyright (c) 2021 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/MsgPack.h"

#include <cstdint>

namespace carla {
namespace rpc {

  enum class VehicleDoor : uint8_t {
    FL = 0,
    FR = 1,
    RL = 2,
    RR = 3,
    Hood = 4,
    Trunk = 5,
    All = 6
  };

} // namespace rpc
} // namespace carla

MSGPACK_ADD_ENUM(carla::rpc::VehicleDoor);
