// Copyright (c) 2024 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/MsgPack.h"

#include <cstdint>

namespace carla {
namespace rpc {

using MapLayerType = uint16_t;

enum class MapLayer : MapLayerType
{
  None            =  0,
  Buildings       =  0x1,
  Decals          =  0x1 << 1,
  Foliage         =  0x1 << 2,
  Ground          =  0x1 << 3,
  ParkedVehicles  =  0x1 << 4,
  Particles       =  0x1 << 5,
  Props           =  0x1 << 6,
  StreetLights    =  0x1 << 7,
  Walls           =  0x1 << 8,
  All             =  0xFFFF,
};

std::string MapLayerToString(MapLayer MapLayerValue);

} // namespace rpc
} // namespace carla

MSGPACK_ADD_ENUM(carla::rpc::MapLayer);
