#include "TrafficDistributor.h"

namespace traffic_manager {

  const float MINIMUM_LANE_CHANGE_DISTANCE = 5.0;
  const float LATERAL_DETECTION_CONE = 135.0;
  const float LANE_CHANGE_OBSTACLE_DISTANCE = 20.0;

  TrafficDistributor::TrafficDistributor() {}

  TrafficDistributor::~TrafficDistributor() {}

  void TrafficDistributor::SetVehicleId(
      uint vehicle_id,
      GeoIds ids) {

    std::unique_lock<std::shared_timed_mutex> lock(distributor_mutex);
    if (road_to_vehicle_id_map.find(ids) != road_to_vehicle_id_map.end()) {
      road_to_vehicle_id_map.at(ids).insert(vehicle_id);
    } else {
      road_to_vehicle_id_map.insert({ids, std::unordered_set<uint>()});
      road_to_vehicle_id_map.at(ids).insert(vehicle_id);
    }
  }

  void TrafficDistributor::EraseVehicleId(
      uint vehicle_id,
      GeoIds ids) {

    std::unique_lock<std::shared_timed_mutex> lock(distributor_mutex);
    if (road_to_vehicle_id_map.find(ids) != road_to_vehicle_id_map.end()) {
      road_to_vehicle_id_map.at(ids).erase(vehicle_id);
    }
  }

  void TrafficDistributor::SetRoadIds(
      uint vehicle_id,
      GeoIds ids) {

    std::unique_lock<std::shared_timed_mutex> lock(distributor_mutex);
    if (vehicle_id_to_road_map.find(vehicle_id) != vehicle_id_to_road_map.end()) {
      vehicle_id_to_road_map.at(vehicle_id) = ids;
    } else {
      vehicle_id_to_road_map.insert({vehicle_id, ids});
    }
  }

  GeoIds TrafficDistributor::GetRoadIds(uint vehicle_id) const {

    std::shared_lock<std::shared_timed_mutex> lock(distributor_mutex);
    if (vehicle_id_to_road_map.find(vehicle_id) != vehicle_id_to_road_map.end()) {
      return vehicle_id_to_road_map.at(vehicle_id);
    } else {
      return {0, 0, 0};
    }
  }

  std::unordered_set<uint> TrafficDistributor::GetVehicleIds(GeoIds ids) const {

    std::shared_lock<std::shared_timed_mutex> lock(distributor_mutex);
    if (road_to_vehicle_id_map.find(ids) != road_to_vehicle_id_map.end()) {
      return road_to_vehicle_id_map.at(ids);
    } else {
      return std::unordered_set<uint>();
    }

  }

  void TrafficDistributor::UpdateVehicleRoadPosition(
      uint actor_id,
      GeoIds road_ids) {

    auto old_ids = GetRoadIds(actor_id);

    if (
      road_ids.road_id != old_ids.road_id ||
      road_ids.section_id != old_ids.section_id ||
      road_ids.lane_id != old_ids.lane_id) {

      EraseVehicleId(actor_id, old_ids);
      SetVehicleId(actor_id, road_ids);
      SetRoadIds(actor_id, road_ids);
    }
  }

  std::shared_ptr<SimpleWaypoint>
  TrafficDistributor::AssignLaneChange(
      carla::SharedPtr<carla::client::Actor> vehicle,
      std::shared_ptr<SimpleWaypoint> current_waypoint,
      GeoIds current_road_ids,
      std::shared_ptr<BufferList> buffer_list,
      std::unordered_map<uint, int> &vehicle_id_to_index,
      std::vector<carla::SharedPtr<carla::client::Actor>> &actor_list,
      carla::client::DebugHelper &debug_helper) {

    auto actor_id = vehicle->GetId();
    auto vehicle_location = vehicle->GetLocation();
    auto vehicle_velocity = vehicle->GetVelocity().Length();
    auto co_lane_vehicles = GetVehicleIds(current_road_ids);

    bool need_to_change_lane = false;
    bool lane_change_direction; // true -> left, false -> right

    auto left_waypoint = current_waypoint->GetLeftWaypoint();
    auto right_waypoint = current_waypoint->GetRightWaypoint();

    if (co_lane_vehicles.size() >= 2) {
      for (auto &same_lane_vehicle_id: co_lane_vehicles) {

        auto &other_vehicle_buffer = buffer_list->at(
            vehicle_id_to_index.at(same_lane_vehicle_id));

        std::shared_ptr<traffic_manager::SimpleWaypoint> same_lane_vehicle_waypoint = nullptr;
        if (!other_vehicle_buffer.empty()) {
          same_lane_vehicle_waypoint = buffer_list->at(
              vehicle_id_to_index.at(same_lane_vehicle_id)).front();
        }

        if (
          same_lane_vehicle_id != actor_id
          &&
          same_lane_vehicle_waypoint != nullptr
          &&
          !same_lane_vehicle_waypoint->CheckJunction()
          &&
          DeviationDotProduct(
          vehicle,
          same_lane_vehicle_waypoint->GetLocation()) > 0 // check other vehicle
                                                         // is ahead
          &&
          same_lane_vehicle_waypoint->GetLocation().Distance(vehicle_location)
          < LANE_CHANGE_OBSTACLE_DISTANCE
          ) {

          if (left_waypoint != nullptr) {
            auto left_lane_vehicles = GetVehicleIds(
                {current_road_ids.road_id,
                 current_road_ids.section_id,
                 left_waypoint->GetWaypoint()->GetLaneId()});
            if (co_lane_vehicles.size() - left_lane_vehicles.size() > 1) {
              need_to_change_lane = true;
              lane_change_direction = true;
              break;
            }
          } else if (right_waypoint != nullptr) {
            auto right_lane_vehicles = GetVehicleIds(
                {current_road_ids.road_id,
                 current_road_ids.section_id,
                 right_waypoint->GetWaypoint()->GetLaneId()});
            if (co_lane_vehicles.size() - right_lane_vehicles.size() > 1) {
              need_to_change_lane = true;
              lane_change_direction = false;
              break;
            }
          }
        }
      }
    }

    int change_over_distance = static_cast<int>(
      std::max(std::ceil(0.5f * vehicle_velocity),
      MINIMUM_LANE_CHANGE_DISTANCE)
      );

    bool possible_to_lane_change = false;
    std::shared_ptr<traffic_manager::SimpleWaypoint> change_over_point;
    if (need_to_change_lane) {

      if (lane_change_direction) {
        change_over_point = left_waypoint;
      } else {
        change_over_point = right_waypoint;
      }

      if (change_over_point != nullptr) {
        auto lane_change_id = change_over_point->GetWaypoint()->GetLaneId();
        auto target_lane_vehicles = GetVehicleIds(
            {current_road_ids.road_id,
             current_road_ids.section_id,
             lane_change_id});

        if (target_lane_vehicles.size() > 0) {
          bool found_hazard = false;
          for (auto other_vehicle_id: target_lane_vehicles) {

            auto &other_vehicle_buffer = buffer_list->at(
                vehicle_id_to_index.at(other_vehicle_id));

            if (
              !other_vehicle_buffer.empty()
              &&
              other_vehicle_buffer.front()->GetWaypoint()->GetLaneId()
              == lane_change_id) {

              auto other_vehicle = actor_list.at(vehicle_id_to_index.at(other_vehicle_id));
              auto other_vehicle_location = other_vehicle_buffer.front()->GetLocation();
              auto relative_deviation = DeviationDotProduct(vehicle, other_vehicle_location);

              if (relative_deviation < 0) {

                auto time_to_reach_other =
                    (change_over_point->Distance(other_vehicle_location) + change_over_distance) /
                    other_vehicle->GetVelocity().Length();

                auto time_to_reach_reference =
                    (change_over_point->Distance(vehicle_location) + change_over_distance) /
                    vehicle->GetVelocity().Length();

                if (
                  relative_deviation > std::cos(M_PI * LATERAL_DETECTION_CONE / 180)
                  || time_to_reach_other > time_to_reach_reference) {
                  
                  found_hazard = true;
                  break;
                }

              } else {

                auto vehicle_reference = boost::static_pointer_cast<carla::client::Vehicle>(vehicle);
                if (
                  change_over_point->Distance(other_vehicle_location)
                  < (1.0 + change_over_distance + vehicle_reference->GetBoundingBox().extent.x * 2)) {
                  found_hazard = true;
                  break;
                }

              }
            }
          }

          if (!found_hazard) {
            possible_to_lane_change = true;
          }

        } else {
          possible_to_lane_change = true;
        }
      }
    }

    if (need_to_change_lane && possible_to_lane_change) {
      for (int i = change_over_distance; i >= 0; i--) {
        change_over_point = change_over_point->GetNextWaypoint()[0];
      }
      return change_over_point;
    } else {
      return nullptr;
    }
  }

  float DeviationCrossProduct(
      carla::SharedPtr<carla::client::Actor> actor,
      const carla::geom::Location &target_location) {

    auto heading_vector = actor->GetTransform().GetForwardVector();
    heading_vector.z = 0;
    heading_vector = heading_vector.MakeUnitVector();
    auto next_vector = target_location - actor->GetLocation();
    next_vector.z = 0;
    if (next_vector.Length() > 2.0f * std::numeric_limits<float>::epsilon()) {
      next_vector = next_vector.MakeUnitVector();
      float cross_z = heading_vector.x * next_vector.y - heading_vector.y * next_vector.x;
      return cross_z;
    } else {
      return 0;
    }
  }

  float DeviationDotProduct(
      carla::SharedPtr<carla::client::Actor> actor,
      const carla::geom::Location &target_location) {

    auto heading_vector = actor->GetTransform().GetForwardVector();
    heading_vector.z = 0;
    heading_vector = heading_vector.MakeUnitVector();
    auto next_vector = target_location - actor->GetLocation();
    next_vector.z = 0;
    if (next_vector.Length() > 2.0f * std::numeric_limits<float>::epsilon()) {
      next_vector = next_vector.MakeUnitVector();
      auto dot_product = carla::geom::Math::Dot(next_vector, heading_vector);
      return dot_product;
    } else {
      return 0;
    }
  }

}
