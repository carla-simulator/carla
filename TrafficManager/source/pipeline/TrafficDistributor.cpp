#include "TrafficDistributor.h"

namespace traffic_manager {

  TrafficDistributor::TrafficDistributor() {}

  TrafficDistributor::~TrafficDistributor() {}

  void TrafficDistributor::setVehicleId(
      int vehicle_id,
      GeoIds ids) {
    if (road_to_vehicle_id_map.find(ids.road_id) != road_to_vehicle_id_map.end()) {
      if (
        road_to_vehicle_id_map[ids.road_id].find(ids.section_id)
        !=
        road_to_vehicle_id_map[ids.road_id].end()) {
        if (
          road_to_vehicle_id_map[ids.road_id][ids.section_id].find(ids.lane_id)
          !=
          road_to_vehicle_id_map[ids.road_id][ids.section_id].end()) {
          road_to_vehicle_id_map
          [ids.road_id]
          [ids.section_id]
          [ids.lane_id]
          .insert(std::pair<int, int>(vehicle_id, 0));
        } else {

          road_to_vehicle_id_map.at(ids.road_id).at(ids.section_id).insert(
            std::pair<int, std::map<int, int>>(
              ids.lane_id, std::map<int, int>()
            )
          );

          road_to_vehicle_id_map.at(ids.road_id).at(ids.section_id).at(ids.lane_id)
            .insert(std::pair<int, int>(vehicle_id, 0));
        }
      } else {
        road_to_vehicle_id_map.at(ids.road_id).insert(
          std::pair<int, std::map<int, std::map<int, int>>>(
            ids.section_id, std::map<int, std::map<int, int>>()
          )
        );

        road_to_vehicle_id_map.at(ids.road_id).at(ids.section_id).insert(
          std::pair<int, std::map<int, int>>(
            ids.lane_id, std::map<int, int>()
          )
        );

        road_to_vehicle_id_map.at(ids.road_id).at(ids.section_id).at(ids.lane_id)
          .insert(std::pair<int, int>(vehicle_id, 0));
      }
    } else {
      road_to_vehicle_id_map.insert(
        std::pair<int, std::map<int, std::map<int, std::map<int, int>>>>(
          ids.road_id, std::map<int, std::map<int, std::map<int, int>>>()
        )
      );

      road_to_vehicle_id_map.at(ids.road_id).insert(
          std::pair<int, std::map<int, std::map<int, int>>>(
            ids.section_id, std::map<int, std::map<int, int>>()
          )
        );

        road_to_vehicle_id_map.at(ids.road_id).at(ids.section_id).insert(
          std::pair<int, std::map<int, int>>(
            ids.lane_id, std::map<int, int>()
          )
        );

        road_to_vehicle_id_map.at(ids.road_id).at(ids.section_id).at(ids.lane_id)
          .insert(std::pair<int, int>(vehicle_id, 0));
    }
  }

  void TrafficDistributor::eraseVehicleId(
      int vehicle_id,
      GeoIds ids) {

    road_to_vehicle_id_map
      .at(ids.road_id)
      .at(ids.section_id)
      .at(ids.lane_id).erase(vehicle_id);
  }

  void TrafficDistributor::setRoadIds(
      int vehicle_id,
      GeoIds ids) {
    vehicle_id_to_road_map[vehicle_id] = ids;
  }

  GeoIds TrafficDistributor::getRoadIds(int vehicle_id) const {
    if (vehicle_id_to_road_map.find(vehicle_id) != vehicle_id_to_road_map.end()) {
      return vehicle_id_to_road_map.at(vehicle_id);
    } else {
      GeoIds ids = {0, 0, 0};
      return ids;
    }
  }

  std::map<int, std::map<int, int>> TrafficDistributor::getVehicleIds(GeoIds ids) const {

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
      std::map<int, std::map<int, int>> vehicle_id_map;
      for (auto lane: road_to_vehicle_id_map.at(ids.road_id).at(ids.section_id)) {
        if (lane.first * ids.lane_id > 0) {
          vehicle_id_map.insert(lane);
        }
      }
      return vehicle_id_map;
    } else {
      return std::map<int, std::map<int, int>>();
    }
  }

  std::vector<
      std::shared_ptr<SimpleWaypoint>
      > TrafficDistributor::assignDistribution(
      int actor_id,
      uint32_t road_id,
      uint32_t section_id,
      int lane_id,
      std::shared_ptr<traffic_manager::SimpleWaypoint> waypoint) {
    int LANE_CHANGE_DISTANCE = 5;
    std::vector<std::shared_ptr<SimpleWaypoint>> lane_change_waypoints;
    GeoIds new_ids = {road_id, section_id, lane_id};
    auto old_ids = getRoadIds(actor_id);
    if (old_ids.lane_id != 0) {
      if (
        road_id != old_ids.road_id
        or section_id != old_ids.section_id
        or lane_id != old_ids.lane_id) {
        if (!(waypoint->checkJunction())) {
          auto vehicle_map = getVehicleIds(new_ids);
          int minimum_vehicle_count = 1000;           // Arbitrary high value
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
              }
              lane_change_waypoints.push_back(change_point);
            }
          }

          if (lane_change_waypoints.size() > 0) {

            auto end_wp = lane_change_waypoints.back()->getWaypoint();
            road_id = end_wp->GetRoadId();
            section_id = end_wp->GetSectionId();
            lane_id = end_wp->GetLaneId();
            eraseVehicleId(actor_id, old_ids);
            setVehicleId(actor_id, {road_id, section_id, lane_id});
            setRoadIds(actor_id, {road_id, section_id, lane_id});
          }
          eraseVehicleId(actor_id, old_ids);
          setVehicleId(actor_id, new_ids);
          setRoadIds(actor_id, new_ids);
        }
        eraseVehicleId(actor_id, old_ids);
        setVehicleId(actor_id, new_ids);
        setRoadIds(actor_id, new_ids);
      }
    } else if (!(waypoint->checkJunction())) {
      std::lock_guard<std::mutex> lock(vehicle_add_mutex);
      if (getRoadIds(actor_id).lane_id == 0) {
        setVehicleId(actor_id, new_ids);
        setRoadIds(actor_id, new_ids);
      }
    }
    return lane_change_waypoints;
  }

}
