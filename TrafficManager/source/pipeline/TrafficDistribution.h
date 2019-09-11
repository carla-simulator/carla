#pragma once

#include <vector>
#include <memory>
#include <unordered_map>
#include <unordered_set>
#include <mutex>
#include <shared_mutex>

#include "SimpleWaypoint.h"

namespace traffic_manager {

  struct GeoIds {
    uint road_id = 0;
    uint section_id = 0;
    int lane_id = 0;
  };

  typedef std::unordered_map<int, std::unordered_set<uint>> LaneMap;
  typedef std::unordered_map<uint, LaneMap> SectionMap;
  typedef std::unordered_map<uint, SectionMap> RoadMap;

  class TrafficDistribution {

  private:

    mutable std::shared_timed_mutex distributor_mutex;

    RoadMap road_to_vehicle_id_map;

    std::unordered_map<uint, GeoIds> vehicle_id_to_road_map;

    void SetVehicleId(uint, GeoIds);
    void EraseVehicleId(uint, GeoIds);
    void SetRoadIds(uint, GeoIds);

  public:

    TrafficDistribution();
    ~TrafficDistribution();

    void UpdateVehicleRoadPosition(
      uint actor_id,
      uint road_id,
      uint section_id,
      int lane_id
    );

    LaneMap GetVehicleIds(GeoIds) const;
    GeoIds GetRoadIds(uint) const;

  };

}
