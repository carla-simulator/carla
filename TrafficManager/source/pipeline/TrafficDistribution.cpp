#include "TrafficDistribution.h"

namespace traffic_manager {

  TrafficDistribution::TrafficDistribution() {}

  TrafficDistribution::~TrafficDistribution() {}

  void TrafficDistribution::SetVehicleId(
      uint vehicle_id,
      GeoIds ids
    ) {

    std::unique_lock<std::shared_timed_mutex> lock(distributor_mutex);
    if (road_to_vehicle_id_map.find(ids.road_id) != road_to_vehicle_id_map.end()) {
      if (
        road_to_vehicle_id_map.at(ids.road_id).find(ids.section_id)
        !=
        road_to_vehicle_id_map.at(ids.road_id).end()
      ) {
        if (
          road_to_vehicle_id_map.at(ids.road_id).at(ids.section_id).find(ids.lane_id)
          !=
          road_to_vehicle_id_map.at(ids.road_id).at(ids.section_id).end()
        ) {

          road_to_vehicle_id_map.at(ids.road_id).at(ids.section_id).at(ids.lane_id).insert(vehicle_id);

        } else {

          road_to_vehicle_id_map.at(ids.road_id).at(ids.section_id).insert(
            std::pair<int, std::unordered_set<uint>>(ids.lane_id, std::unordered_set<uint>())
          );

          road_to_vehicle_id_map.at(ids.road_id).at(ids.section_id).at(ids.lane_id).insert(vehicle_id);

          }
      } else {

        road_to_vehicle_id_map.at(ids.road_id).insert(
          std::pair<uint, LaneMap>(ids.section_id, LaneMap())
        );

        road_to_vehicle_id_map.at(ids.road_id).at(ids.section_id).insert(
          std::pair<int, std::unordered_set<uint>>(ids.lane_id, std::unordered_set<uint>())
        );

        road_to_vehicle_id_map.at(ids.road_id).at(ids.section_id).at(ids.lane_id).insert(vehicle_id);
      }
    } else {

      road_to_vehicle_id_map.insert(
        std::pair<uint, SectionMap>(ids.road_id, SectionMap())
      );

      road_to_vehicle_id_map.at(ids.road_id).insert(
        std::pair<uint, LaneMap>(ids.section_id, LaneMap())
      );

      road_to_vehicle_id_map.at(ids.road_id).at(ids.section_id).insert(
        std::pair<int, std::unordered_set<uint>>(ids.lane_id, std::unordered_set<uint>())
      );

      road_to_vehicle_id_map.at(ids.road_id).at(ids.section_id).at(ids.lane_id).insert(vehicle_id);
    }
  }

  void TrafficDistribution::EraseVehicleId(
    uint vehicle_id,
    GeoIds ids
  ) {

    std::unique_lock<std::shared_timed_mutex> lock(distributor_mutex);
    road_to_vehicle_id_map.at(ids.road_id).at(ids.section_id).at(ids.lane_id).erase(vehicle_id);
  }

  void TrafficDistribution::SetRoadIds(
    uint vehicle_id,
    GeoIds ids
  ) {

    std::unique_lock<std::shared_timed_mutex> lock(distributor_mutex);
    vehicle_id_to_road_map.insert(std::pair<uint, GeoIds>(vehicle_id, ids));
  }

  GeoIds TrafficDistribution::GetRoadIds(uint vehicle_id) const {

    std::shared_lock<std::shared_timed_mutex> lock(distributor_mutex);
    if (vehicle_id_to_road_map.find(vehicle_id) != vehicle_id_to_road_map.end()) {
      return vehicle_id_to_road_map.at(vehicle_id);
    } else {
      GeoIds ids = {0, 0, 0};
      return ids;
    }
  }

  LaneMap TrafficDistribution::GetVehicleIds(GeoIds ids) const {

    std::shared_lock<std::shared_timed_mutex> lock(distributor_mutex);
    bool entry_found = false;
    if (road_to_vehicle_id_map.find(ids.road_id) != road_to_vehicle_id_map.end()) {
      if (
        road_to_vehicle_id_map.at(ids.road_id).find(ids.section_id)
        !=
        road_to_vehicle_id_map.at(ids.road_id).end()) {
        if (
          road_to_vehicle_id_map.at(ids.road_id).at(ids.section_id).find(ids.lane_id)
          !=
          road_to_vehicle_id_map.at(ids.road_id).at(ids.section_id).end()) {
          entry_found = true;
        }
      }
    }

    if (entry_found) {
      LaneMap vehicle_id_map;
      for (auto lane: road_to_vehicle_id_map.at(ids.road_id).at(ids.section_id)) {
        if (lane.first * ids.lane_id > 0) {
          vehicle_id_map.insert(lane);
        }
      }
      return vehicle_id_map;
    } else {
      return LaneMap();
    }
  }

  void TrafficDistribution::UpdateVehicleRoadPosition(
    uint actor_id,
    uint road_id,
    uint section_id,
    int lane_id
  ) {

    GeoIds new_ids = {road_id, section_id, lane_id};
    auto old_ids = GetRoadIds(actor_id);

    if (old_ids.lane_id != 0) {
      if (
        road_id != old_ids.road_id
        or section_id != old_ids.section_id
        or lane_id != old_ids.lane_id
      ) {

        EraseVehicleId(actor_id, old_ids);
        SetVehicleId(actor_id, new_ids);
        SetRoadIds(actor_id, new_ids);
      }
    } else {
      SetVehicleId(actor_id, new_ids);
      SetRoadIds(actor_id, new_ids);
    }
  }

}
