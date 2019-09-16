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

    bool operator==(const GeoIds &other) const{
      return (road_id == other.road_id
              && section_id == other.section_id
              && lane_id == other.lane_id);
    }
  };
}

namespace std {

  template <>
  struct hash<traffic_manager::GeoIds>
  {
    std::size_t operator()(const traffic_manager::GeoIds& k) const
    {
      using std::size_t;
      using std::hash;

      return ((hash<uint>()(k.road_id)
              ^ (hash<uint>()(k.section_id) << 1)) >> 1)
              ^ (hash<int>()(k.lane_id) << 1);
    }
  };

}

namespace traffic_manager {

  typedef std::unordered_map<int, std::unordered_set<uint>> LaneMap;
  typedef std::unordered_map<uint, LaneMap> SectionMap;

  class TrafficDistribution {

  private:

    mutable std::shared_timed_mutex distributor_mutex;
    std::unordered_map<GeoIds, std::unordered_set<uint>> road_to_vehicle_id_map;
    std::unordered_map<uint, GeoIds> vehicle_id_to_road_map;



    void SetVehicleId(uint, GeoIds);
    void EraseVehicleId(uint, GeoIds);
    void SetRoadIds(uint, GeoIds);

  public:

    TrafficDistribution();
    ~TrafficDistribution();

    void UpdateVehicleRoadPosition(
      uint actor_id,
      GeoIds road_ids
    );

    std::unordered_set<uint> GetVehicleIds(GeoIds ids) const;
    GeoIds GetRoadIds(uint vehicle_id) const;

  };

}
