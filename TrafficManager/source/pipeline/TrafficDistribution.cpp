#include "TrafficDistribution.h"

namespace traffic_manager {

  TrafficDistribution::TrafficDistribution() {}

  TrafficDistribution::~TrafficDistribution() {}

  std::string TrafficDistribution::MakeRoadKey(GeoIds ids) {
    return std::to_string(ids.road_id) + std::to_string(ids.section_id) + std::to_string(ids.lane_id);
  }

  void TrafficDistribution::SetVehicleId(
      uint vehicle_id,
      GeoIds ids
    ) {

    std::unique_lock<std::shared_timed_mutex> lock(distributor_mutex);
    auto road_key = MakeRoadKey(ids);
    if (road_to_vehicle_id_map.find(road_key) != road_to_vehicle_id_map.end()) {
      road_to_vehicle_id_map.at(road_key).insert(vehicle_id);
    } else {
      road_to_vehicle_id_map.insert({road_key, std::unordered_set<uint>()});
      road_to_vehicle_id_map.at(road_key).insert(vehicle_id);
    }
  }

  void TrafficDistribution::EraseVehicleId(
    uint vehicle_id,
    GeoIds ids
  ) {

    std::unique_lock<std::shared_timed_mutex> lock(distributor_mutex);
    auto road_key = MakeRoadKey(ids);
    if (road_to_vehicle_id_map.find(road_key) != road_to_vehicle_id_map.end()) {
      road_to_vehicle_id_map.at(road_key).erase(vehicle_id);
    }
  }

  void TrafficDistribution::SetRoadIds(
    uint vehicle_id,
    GeoIds ids
  ) {

    std::unique_lock<std::shared_timed_mutex> lock(distributor_mutex);
    vehicle_id_to_road_map.insert({vehicle_id, ids});
  }

  GeoIds TrafficDistribution::GetRoadIds(uint vehicle_id) const {

    std::shared_lock<std::shared_timed_mutex> lock(distributor_mutex);
    if (vehicle_id_to_road_map.find(vehicle_id) != vehicle_id_to_road_map.end()) {
      return vehicle_id_to_road_map.at(vehicle_id);
    } else {
      return {0, 0, 0};
    }
  }

  std::unordered_set<uint> TrafficDistribution::GetVehicleIds(std::string road_key) const {

    std::shared_lock<std::shared_timed_mutex> lock(distributor_mutex);
    if (road_to_vehicle_id_map.find(road_key) != road_to_vehicle_id_map.end()) {
      return road_to_vehicle_id_map.at(road_key);
    } else {
      return std::unordered_set<uint>();
    }

  }

  void TrafficDistribution::UpdateVehicleRoadPosition(
    uint actor_id,
    GeoIds road_ids
  ) {

    auto old_ids = GetRoadIds(actor_id);

    if (
      road_ids.road_id != old_ids.road_id
      or road_ids.section_id != old_ids.section_id
      or road_ids.lane_id != old_ids.lane_id
    ) {

      EraseVehicleId(actor_id, old_ids);
      SetVehicleId(actor_id, road_ids);
      SetRoadIds(actor_id, road_ids);
    }
  }
}
