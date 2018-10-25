// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/road/element/RoadSegment.h"
#include "carla/NonCopyable.h"

#include <map>

namespace carla {
namespace road {

  using namespace carla::road::element;

  struct lane_junction_t {
    std::string contact_point = "start";
    int junction_id = -1;

    int connection_road = -1;
    int incomming_road = -1;

    std::vector<int> from_lane;
    std::vector<int> to_lane;
  };

  class Map {
  public:

    Map(const Map &) = delete;
    Map &operator=(const Map &) = delete;

    Map(Map &&) = default;
    Map &operator=(Map &&) = default;

    bool ExistId(id_type id) const;

    const RoadSegment *GetRoad(id_type id) const;

    std::vector<id_type> GetAllIds() const;

    uint32_t GetRoadCount() const;

    const RoadSegment &NearestRoad(const geom::Location &loc);

    void SetJunctionInformation(const std::vector<lane_junction_t> &junctionInfo) { _junction_information = junctionInfo; }
    std::vector<lane_junction_t> GetJunctionInformation() const { return _junction_information; }

  private:

    friend class MapBuilder;
    std::vector<lane_junction_t> _junction_information;

    Map() {}

    std::map<id_type, std::unique_ptr<RoadSegment>> _elements;
  };

} // namespace road
} // namespace carla
