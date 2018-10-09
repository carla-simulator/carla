// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "Map.h"
#include "carla/geom/Location.h"

#include <map>
#include <vector>

namespace carla {
namespace road {

class MapBuilder {
public:
  bool AddRoadSegment(const RoadSegmentDefinition &seg);

private:
  Map map;
};

} // namespace road
} // namespace  carla