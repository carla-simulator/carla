#include "TrafficDistributor.h"

namespace traffic_manager {

  TrafficDistributor::TrafficDistributor() {}

  TrafficDistributor::~TrafficDistributor() {}

  void TrafficDistributor::setVehicleId(
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

  void TrafficDistributor::eraseVehicleId(
    uint vehicle_id,
    GeoIds ids
  ) {

    std::unique_lock<std::shared_timed_mutex> lock(distributor_mutex);
    road_to_vehicle_id_map.at(ids.road_id).at(ids.section_id).at(ids.lane_id).erase(vehicle_id);
  }

  void TrafficDistributor::setRoadIds(
    uint vehicle_id,
    GeoIds ids
  ) {

    std::unique_lock<std::shared_timed_mutex> lock(distributor_mutex);
    vehicle_id_to_road_map.insert(std::pair<uint, GeoIds>(vehicle_id, ids));
  }

  GeoIds TrafficDistributor::getRoadIds(uint vehicle_id) const {

    std::shared_lock<std::shared_timed_mutex> lock(distributor_mutex);
    if (vehicle_id_to_road_map.find(vehicle_id) != vehicle_id_to_road_map.end()) {
      return vehicle_id_to_road_map.at(vehicle_id);
    } else {
      GeoIds ids = {0, 0, 0};
      return ids;
    }
  }

  LaneMap TrafficDistributor::getVehicleIds(GeoIds ids) const {

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

  std::vector<std::shared_ptr<SimpleWaypoint>> TrafficDistributor::assignDistribution(
    uint actor_id,
    uint road_id,
    uint section_id,
    int lane_id,
    std::shared_ptr<traffic_manager::SimpleWaypoint> waypoint
  ) {

    int LANE_CHANGE_DISTANCE = 5;
    std::vector<std::shared_ptr<SimpleWaypoint>> lane_change_waypoints;
    GeoIds new_ids = {road_id, section_id, lane_id};
    auto old_ids = getRoadIds(actor_id);

    if (old_ids.lane_id != 0) {

      if (
        road_id != old_ids.road_id
        or section_id != old_ids.section_id
        or lane_id != old_ids.lane_id
      ) {

        if (!(waypoint->checkJunction())) {

          auto vehicle_map = getVehicleIds(new_ids);
          int minimum_vehicle_count = 1000;     // Arbitrary high value
          int target_lane_id = 0;
          for (auto lane: vehicle_map) {
            auto new_count = lane.second.size();
            if (new_count < minimum_vehicle_count) {
              target_lane_id = lane.first;
              minimum_vehicle_count = new_count;
            }
          }

          for (int i = 0; i < abs(target_lane_id - lane_id); i++) {
            std::shared_ptr<SimpleWaypoint> change_point;

            auto right_change_point = waypoint->getRightWaypoint();
            auto left_change_point = waypoint->getLeftWaypoint();
            if (right_change_point == nullptr) {
              change_point = left_change_point;
            } else if (left_change_point == nullptr) {
              change_point = right_change_point;
            }

            if (right_change_point != nullptr and left_change_point != nullptr) {
              int right_lane_id = right_change_point->getWaypoint()->GetLaneId();
              int left_lane_id = left_change_point->getWaypoint()->GetLaneId();

              if (abs(right_lane_id - target_lane_id) < abs(left_lane_id - target_lane_id)) {
                change_point = right_change_point;
              } else {
                change_point = left_change_point;
              }
            }

            if (change_point != nullptr) {
              for (int i = 0; i < LANE_CHANGE_DISTANCE; i++) {
                change_point = change_point->getNextWaypoint()[0];
                if (change_point->checkJunction()) {
                  break;
                }
              }
              if (!change_point->checkJunction()) {
                lane_change_waypoints.push_back(change_point);
              }
            }
          }

          if (lane_change_waypoints.size() > 0) {

            auto end_wp = lane_change_waypoints.back()->getWaypoint();
            road_id = end_wp->GetRoadId();
            section_id = end_wp->GetSectionId();
            lane_id = end_wp->GetLaneId();

          }
        }
      }

      eraseVehicleId(actor_id, old_ids);
      setVehicleId(actor_id, new_ids);
      setRoadIds(actor_id, new_ids);

    } else if (!(waypoint->checkJunction())) {
      if (getRoadIds(actor_id).lane_id == 0) {
        setVehicleId(actor_id, new_ids);
        setRoadIds(actor_id, new_ids);
      }
    }

    return lane_change_waypoints;
  }

}
