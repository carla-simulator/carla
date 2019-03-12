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

    SharedPtr<Map> Build();

  private:

    /// Set the total length of each road based on the geometries
    void SetTotalRoadSegmentLength();

    /// Create the pointers between RoadSegments based on the ids
    void CreatePointersBetweenRoadSegments();

    MapData _map_data;
  };

} // namespace road
} // namespace carla
