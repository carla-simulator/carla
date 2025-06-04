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

  enum class CityObjectLabel : uint8_t {
    None         =    0u,
    // cityscape labels
    Roads        =    1u,
    Sidewalks    =    2u,
    Buildings    =    3u,
    Walls        =    4u,
    Fences       =    5u,
    Poles        =    6u,
    TrafficLight =    7u,
    TrafficSigns =    8u,
    Vegetation   =    9u,
    Terrain      =   10u,
    Sky          =   11u,
    Pedestrians  =   12u,
    Rider        =   13u,
    Car          =   14u,
    Truck        =   15u,
    Bus          =   16u,
    Train        =   17u,
    Motorcycle   =   18u,
    Bicycle      =   19u,
    // custom
    Static       =   20u,
    Dynamic      =   21u,
    Other        =   22u,
    Water        =   23u,
    RoadLines    =   24u,
    Ground       =   25u,
    Bridge       =   26u,
    RailTrack    =   27u,
    GuardRail    =   28u,
    Rock         =   29u,

    Any          =  0xFF
  };

} // namespace rpc
} // namespace carla

MSGPACK_ADD_ENUM(carla::rpc::CityObjectLabel);
