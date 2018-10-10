// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "Map.h"

namespace carla {
namespace road {

  bool Map::AddRoadSegment(const RoadSegmentDefinition &def) {
    _graph.AddNode(def.GetId(), def);
    return true;
  }

} // namespace road
} // namespace carla
