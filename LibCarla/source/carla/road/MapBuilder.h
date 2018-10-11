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

    Map Build();

  private:

    template <typename T, typename ... Args>
    T &MakeElement(id_type id, Args && ... args) {
      auto inst = std::make_unique<T>(std::forward<Args>(args) ...);
      T &r = *inst;
      _map._elements.emplace(id, std::move(inst));
      return r;
    }

  private:
    Map _map;
    temp_section_type _temp_sections;
  };

} // namespace road
} // namespace carla
