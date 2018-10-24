// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "carla/road/element/Waypoint.h"

namespace carla {
namespace road {
namespace element {

  RoadInfoList Waypoint::GetRoadInfo() const {
    return RoadInfoList(_map->GetData().GetRoad(_road_id)->GetInfos(_dist));
  }

} // namespace element
} // namespace road
} // namespace carla
