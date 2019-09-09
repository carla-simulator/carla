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

  class TrafficDistributor {

  private:

    mutable std::shared_timed_mutex distributor_mutex;

    RoadMap road_to_vehicle_id_map;

    std::unordered_map<uint, GeoIds> vehicle_id_to_road_map;

    void setVehicleId(uint, GeoIds);
    void eraseVehicleId(uint, GeoIds);
    void setRoadIds(uint, GeoIds);

    LaneMap getVehicleIds(GeoIds) const;
    GeoIds getRoadIds(uint) const;

  public:

    TrafficDistributor();
    ~TrafficDistributor();

    std::vector<
        std::shared_ptr<SimpleWaypoint>
        > assignDistribution(
        uint actor_id,
        uint road_id,
        uint section_id,
        int lane_id,
        std::shared_ptr<traffic_manager::SimpleWaypoint> waypoint);

  };

}
