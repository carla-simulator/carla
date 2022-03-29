// Copyright (c) 2020 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "MapLayer.h"

namespace carla {
namespace rpc {

std::string MapLayerToString(MapLayer MapLayerValue)
{
  switch(MapLayerValue)
  {
    case MapLayer::None:            return "None";
    case MapLayer::Buildings:       return "Buildings";
    case MapLayer::Decals:          return "Decals";
    case MapLayer::Foliage:         return "Foliage";
    case MapLayer::Ground:          return "Ground";
    case MapLayer::ParkedVehicles:  return "Parked_Vehicles";
    case MapLayer::Particles:       return "Particles";
    case MapLayer::Props:           return "Props";
    case MapLayer::StreetLights:    return "StreetLights";
    case MapLayer::Walls:           return "Walls";
    case MapLayer::All:             return "All";
    default:                        return "Invalid";
  }
}


} // namespace rpc
} // namespace carla
