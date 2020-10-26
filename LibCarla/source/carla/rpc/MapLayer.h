// Copyright (c) 2020 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/MsgPack.h"

#include <cstdint>

namespace carla {
namespace rpc {

enum class MapLayer : uint8_t
{
  None            =  0,
  Buildings       =  0x1,
  Decals          =  0x1 << 1,
  Foliage         =  0x1 << 2,
  Ground          =  0x1 << 3,
  ParkedVehicles  =  0x1 << 4,
  Props           =  0x1 << 5,
  StreetLights    =  0x1 << 6,
  Walls           =  0x1 << 7,
  All             =  0xFF
};


} // namespace rpc
} // namespace carla

MSGPACK_ADD_ENUM(carla::rpc::MapLayer);
