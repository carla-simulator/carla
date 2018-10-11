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

  using temp_section_type = std::multimap<id_type, const RoadSegmentDefinition &>;

  class MapBuilder {
  public:

    bool AddRoadSegmentDefinition(const RoadSegmentDefinition &seg);

    const Map &BuildMap();

  private:
    temp_section_type _temp_sections;
  };

} // namespace road
} // namespace carla
