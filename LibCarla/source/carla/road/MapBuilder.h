// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/geom/Location.h"
#include "carla/road/Map.h"

#include <map>

namespace carla {
namespace road {

  class MapBuilder {
  public:

    bool AddRoadSegmentDefinition(element::RoadSegmentDefinition &seg);

    void SetJunctionInformation(const std::vector<carla::road::lane_junction_t> &junctionInfo)
    {
      _map_data.SetJunctionInformation(junctionInfo);
    }

    SharedPtr<Map> Build();

  private:

    template <typename T, typename ... Args>
    T &MakeElement(element::id_type id, Args && ... args) {
      auto inst = std::make_unique<T>(std::forward<Args>(args) ...);
      T &r = *inst;
      _map_data._elements.emplace(id, std::move(inst));
      return r;
    }

  private:

    MapData _map_data;
    std::map<element::id_type, element::RoadSegmentDefinition> _temp_sections;
  };

} // namespace road
} // namespace carla
