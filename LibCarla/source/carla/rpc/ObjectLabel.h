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

  enum class CityObjectLabel : uint8_t {
    None         =   0u,
    Static       =   1u,
    Dynamic      =   2u,
    Other        =   3u,
    Water        =   4u,
    RoadLines    =   6u,
    Roads        =   7u,
    Sidewalks    =   8u,
    Buildings    =  11u,
    Walls        =  12u,
    Fences       =  13u,
    Ground       =  14u,
    Bridge       =  15u,
    RailTrack    =  16u,
    Poles        =  17u,
    GuardRail    =  18u,
    TrafficLight =  19u,
    TrafficSigns =  20u,
    Vegetation   =  21u,
    Terrain      =  22u,
    Sky          =  23u,
    Pedestrians  =  24u,
    Rider        =  25u,
    Car          =  26u,
    Truck        =  27u,
    Bus          =  28u,
    Train        =  31u,
    Motorcycle   =  32u,
    Bicycle      =  33u,
    Any          =  0xFF
  };

} // namespace rpc
} // namespace carla

MSGPACK_ADD_ENUM(carla::rpc::CityObjectLabel);
