// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "MapBuilder.h"

namespace carla {
namespace road {

  bool MapBuilder::AddRoadSegmentDefinition(RoadSegmentDefinition &seg) {
    _temp_sections.emplace(seg.GetId(), seg);
    return true;
  }

  const Map &MapBuilder::Build() {

    for (auto &&seg : _temp_sections) {
      map.MakeElement<RoadSegment>(seg.first, seg.second);
    }

    return map;
  }

} // namespace road
} // namespace carla
