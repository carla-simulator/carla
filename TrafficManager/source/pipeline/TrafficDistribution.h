#pragma once

#include <vector>
#include <memory>
#include <unordered_map>
#include <unordered_set>
#include <mutex>
#include <shared_mutex>
#include <string>

#include "SimpleWaypoint.h"

namespace traffic_manager {

  struct GeoIds {
    uint road_id = 0;
    uint section_id = 0;
    int lane_id = 0;
  };

  typedef std::unordered_map<int, std::unordered_set<uint>> LaneMap;
  typedef std::unordered_map<uint, LaneMap> SectionMap;

  class TrafficDistribution {

  private:

    mutable std::shared_timed_mutex distributor_mutex;

    std::unordered_map<std::string, std::unordered_set<uint>> road_to_vehicle_id_map;

    std::unordered_map<uint, GeoIds> vehicle_id_to_road_map;

    void SetVehicleId(uint, GeoIds);
    void EraseVehicleId(uint, GeoIds);
    void SetRoadIds(uint, GeoIds);

  public:

    TrafficDistribution();
    ~TrafficDistribution();

    std::string MakeRoadKey (GeoIds);

    void UpdateVehicleRoadPosition(
      uint actor_id,
      GeoIds road_ids
    );

    std::unordered_set<uint> GetVehicleIds(std::string) const;
    GeoIds GetRoadIds(uint) const;

  };

}
