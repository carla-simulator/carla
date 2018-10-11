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

  Map MapBuilder::Build() {

    for (auto &&seg : _temp_sections) {
      MakeElement<RoadSegment>(seg.first, std::move(seg.second));
    }

    // _map is a memeber of MapBuilder so you must especify if
    // you want to keep it (will return copy -> Map(const Map &))
    // or move it (will return move -> Map(Map &&))
    return std::move(_map);
  }

} // namespace road
} // namespace carla
