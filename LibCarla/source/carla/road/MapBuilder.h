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

    void SetJunctionInformation(const std::vector<carla::road::lane_junction_t> &junctionInfo) {
      _map_data.SetJunctionInformation(junctionInfo);
    }

    void SetGeoReference(const std::string &geoReference) {
      _map_data.SetGeoReference(geoReference);
    }

    void SetTrafficGroupData(const std::vector<opendrive::types::TrafficLightGroup> &trafficLightData) {
      _map_data.SetTrafficLightData(trafficLightData);
    }

    void SetTrafficSignData(const std::vector<opendrive::types::TrafficSign> &trafficSignData) {
      _map_data.SetTrafficSignData(trafficSignData);
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

    /// Set the total length of each road based on the geometries
    void SetTotalRoadSegmentLength();

    /// Create the pointers between RoadSegments based on the ids
    void CreatePointersBetweenRoadSegments();

    /// Set the _lane_center_offset of all the lanes
    void ComputeLaneCenterOffset();

  private:

    MapData _map_data;
    std::map<element::id_type, element::RoadSegmentDefinition> _temp_sections;
  };

} // namespace road
} // namespace carla
