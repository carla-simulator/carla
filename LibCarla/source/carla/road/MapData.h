// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/geom/GeoLocation.h"
#include "carla/Iterator.h"
#include "carla/road/Junction.h"
#include "carla/ListView.h"
#include "carla/NonCopyable.h"
#include "carla/road/Road.h"
#include "carla/road/RoadTypes.h"
#include "carla/road/element/RoadInfo.h"

#include <boost/iterator/transform_iterator.hpp>

#include <unordered_map>

namespace carla {
namespace road {

  class Lane;

  class MapData : private MovableNonCopyable {
  public:

    const geom::GeoLocation &GetGeoReference() const {
      return _geo_reference;
    }

    std::unordered_map<RoadId, Road> &GetRoads();

    const std::unordered_map<RoadId, Road> &GetRoads() const {
      return _roads;
    }

    std::unordered_map<JuncId, Junction> &GetJunctions();

    Road *GetRoad(const RoadId id);

    Junction *GetJunction(JuncId id);

    Lane *GetLane(const RoadId road_id, LaneId lane_id, float s);

    template <typename T>
    const std::shared_ptr<const T> GetRoadInfo(
        const RoadId id,
        const float s) {
      auto road = GetRoad(id);
      if (road != nullptr) {
        return road->GetInfo<T>(s);
      }
      return nullptr;
    }

    template <typename T>
    const std::shared_ptr<const T> GetLaneInfo(
        const RoadId road_id,
        const LaneId lane_id,
        const float s) {
      auto road = GetRoad(road_id);
      if (road != nullptr) {
        auto lane = road->GetLane(lane_id, s);
        if (lane != nullptr) {
          return lane->GetInfo<T>(s);
        }
      }
      return nullptr;
    }

  private:

    friend class MapBuilder;

    MapData() = default;

    geom::GeoLocation _geo_reference;

    std::unordered_map<RoadId, Road> _roads;

    std::unordered_map<JuncId, Junction> _junctions;
  };

} // namespace road
} // namespace carla
