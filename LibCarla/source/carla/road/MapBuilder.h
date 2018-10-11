// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "Map.h"
#include "carla/geom/Location.h"

#include <map>

namespace carla {
namespace road {

  using temp_section_type = std::map<id_type, RoadSegmentDefinition &>;

  class MapBuilder {
  public:

    bool AddRoadSegmentDefinition(RoadSegmentDefinition &seg);

    const Map &Build();

  private:

    temp_section_type _temp_sections;
    Map map;
  };

} // namespace road
} // namespace carla
