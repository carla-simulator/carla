#pragma once

#include "carla/MsgPack.h"

#include <cstdint>

namespace carla {
namespace rpc {

  enum class GimbalMode : uint8_t {
    VehicleRelative,
    WorldRelative
  };

} // namespace rpc
} // namespace carla

MSGPACK_ADD_ENUM(carla::rpc::GimbalMode);