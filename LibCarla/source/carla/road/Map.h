// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/road/element/RoadSegment.h"

#include <map>

namespace carla {
namespace road {

  class Map {
  public:

    Map(const Map &) = delete;
    Map &operator=(const Map &) = delete;

    Map(Map &&) = default;
    Map &operator=(Map &&) = default;

    bool ExistId(element::id_type id) const;

    const element::RoadSegment *GetRoad(element::id_type id) const;

    std::vector<element::id_type> GetAllIds() const;

    uint32_t GetRoadCount() const;

    const element::RoadSegment &NearestRoad(const geom::Location &loc);

  private:

    friend class MapBuilder;

    Map() {}

    std::map<element::id_type, std::unique_ptr<element::RoadSegment>> _elements;
  };

} // namespace road
} // namespace carla
