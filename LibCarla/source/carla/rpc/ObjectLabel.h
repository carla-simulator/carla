// Copyright (c) 2025 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/MsgPack.h"

#include <cstdint>
#include <string>

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

    Any          =  0xFF
  };

} // namespace rpc
} // namespace carla

MSGPACK_ADD_ENUM(carla::rpc::CityObjectLabel)

namespace std {
  inline std::string to_string(carla::rpc::CityObjectLabel label) {
    switch (label)
    {
      case carla::rpc::CityObjectLabel::None: return "None";
      case carla::rpc::CityObjectLabel::Roads: return "Roads";
      case carla::rpc::CityObjectLabel::Sidewalks: return "Sidewalks";
      case carla::rpc::CityObjectLabel::Buildings: return "Buildings";
      case carla::rpc::CityObjectLabel::Walls: return "Walls";
      case carla::rpc::CityObjectLabel::Fences: return "Fences";
      case carla::rpc::CityObjectLabel::Poles: return "Poles";
      case carla::rpc::CityObjectLabel::TrafficLight: return "TrafficLight";
      case carla::rpc::CityObjectLabel::TrafficSigns: return "TrafficSigns";
      case carla::rpc::CityObjectLabel::Vegetation: return "Vegetation";
      case carla::rpc::CityObjectLabel::Terrain: return "Terrain";
      case carla::rpc::CityObjectLabel::Sky: return "Sky";
      case carla::rpc::CityObjectLabel::Pedestrians: return "Pedestrians";
      case carla::rpc::CityObjectLabel::Rider: return "Rider";
      case carla::rpc::CityObjectLabel::Car: return "Car";
      case carla::rpc::CityObjectLabel::Truck: return "Truck";
      case carla::rpc::CityObjectLabel::Bus: return "Bus";
      case carla::rpc::CityObjectLabel::Train: return "Train";
      case carla::rpc::CityObjectLabel::Motorcycle: return "Motorcycle";
      case carla::rpc::CityObjectLabel::Bicycle: return "Bicycle";
      case carla::rpc::CityObjectLabel::Static: return "Static";
      case carla::rpc::CityObjectLabel::Dynamic: return "Dynamic";
      case carla::rpc::CityObjectLabel::Other: return "Other";
      case carla::rpc::CityObjectLabel::Water: return "Water";
      case carla::rpc::CityObjectLabel::RoadLines: return "RoadLines";
      case carla::rpc::CityObjectLabel::Ground: return "Ground";
      case carla::rpc::CityObjectLabel::Bridge: return "Bridge";
      case carla::rpc::CityObjectLabel::RailTrack: return "RailTrack";
      case carla::rpc::CityObjectLabel::GuardRail: return "GuardRail";
      case carla::rpc::CityObjectLabel::Any: return "Any";
      default: return "Unknown";
    }
  };
} // namespace std
