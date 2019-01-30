// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/MsgPack.h"

#include <cstdint>

namespace carla {
namespace rpc {

  enum class TrafficLightState : uint8_t {
    Red,
    Yellow,
    Green,
    Off,
    Unknown,
    SIZE
  };

} // namespace rpc
} // namespace carla

MSGPACK_ADD_ENUM(carla::rpc::TrafficLightState);
