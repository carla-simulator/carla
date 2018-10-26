// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/NonCopyable.h"
#include "carla/road/element/RoadSegment.h"

#include <map>

namespace carla {
namespace road {
namespace element {
  class Waypoint;
}

  struct lane_junction_t {
    std::string contact_point = "start";
    int junction_id = -1;

    int connection_road = -1;
    int incomming_road = -1;

    std::vector<int> from_lane;
    std::vector<int> to_lane;
  };

  class MapData
    : private MovableNonCopyable {
  public:

    bool ExistId(element::id_type id) const;

    const element::RoadSegment *GetRoad(element::id_type id) const;

    std::vector<element::id_type> GetAllIds() const;

    uint32_t GetRoadCount() const;

    const element::RoadSegment &NearestRoad(const geom::Location &loc);

    void SetJunctionInformation(const std::vector<lane_junction_t> &junctionInfo) {
      _junction_information = junctionInfo;
    }

    std::vector<lane_junction_t> GetJunctionInformation() const {
      return _junction_information;
    }

  private:

    friend class MapBuilder;
    friend class Map;
    friend element::Waypoint;

    MapData() = default;

    std::vector<lane_junction_t> _junction_information;

    std::map<element::id_type, std::unique_ptr<element::RoadSegment>> _elements;
  };

} // namespace road
} // namespace carla
