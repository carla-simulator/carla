#include "TrafficDistributor.h"

namespace traffic_manager {

namespace TrafficDistributorConstants {
  static const float MINIMUM_LANE_CHANGE_DISTANCE = 10.0f;
  static const float LATERAL_DETECTION_CONE = 135.0f;
  static const float LANE_CHANGE_OBSTACLE_DISTANCE = 20.0f;
  static const float LANE_OBSTACLE_MINIMUM_DISTANCE = 10.0f;
  static const float APPROACHING_VEHICLE_TIME_MARGIN = 2.0f;
}
  using namespace TrafficDistributorConstants;

  TrafficDistributor::TrafficDistributor() {}

  TrafficDistributor::~TrafficDistributor() {}

  void TrafficDistributor::SetVehicleId(ActorId vehicle_id, GeoIds ids) {

    std::unique_lock<std::shared_timed_mutex> lock(distributor_mutex);
    if (road_to_vehicle_id_map.find(ids) != road_to_vehicle_id_map.end()) {
      road_to_vehicle_id_map.at(ids).insert(vehicle_id);
    } else {
      road_to_vehicle_id_map.insert({ids, std::unordered_set<ActorId>()});
      road_to_vehicle_id_map.at(ids).insert(vehicle_id);
    }
  }

  void TrafficDistributor::EraseVehicleId(ActorId vehicle_id, GeoIds ids) {

    std::unique_lock<std::shared_timed_mutex> lock(distributor_mutex);
    if (road_to_vehicle_id_map.find(ids) != road_to_vehicle_id_map.end()) {
      road_to_vehicle_id_map.at(ids).erase(vehicle_id);
    }
  }

  void TrafficDistributor::SetRoadIds(ActorId vehicle_id, GeoIds ids) {

    std::unique_lock<std::shared_timed_mutex> lock(distributor_mutex);
    if (vehicle_id_to_road_map.find(vehicle_id) != vehicle_id_to_road_map.end()) {
      vehicle_id_to_road_map.at(vehicle_id) = ids;
    } else {
      vehicle_id_to_road_map.insert({vehicle_id, ids});
    }
  }

  GeoIds TrafficDistributor::GetRoadIds(ActorId vehicle_id) const {

    std::shared_lock<std::shared_timed_mutex> lock(distributor_mutex);
    if (vehicle_id_to_road_map.find(vehicle_id) != vehicle_id_to_road_map.end()) {
      return vehicle_id_to_road_map.at(vehicle_id);
    } else {
      return {0, 0, 0};
    }
  }

  std::unordered_set<ActorId> TrafficDistributor::GetVehicleIds(GeoIds ids) const {

    std::shared_lock<std::shared_timed_mutex> lock(distributor_mutex);
    if (road_to_vehicle_id_map.find(ids) != road_to_vehicle_id_map.end()) {
      return road_to_vehicle_id_map.at(ids);
    } else {
      return std::unordered_set<ActorId>();
    }

  }

  void TrafficDistributor::UpdateVehicleRoadPosition(ActorId actor_id, GeoIds road_ids) {

    GeoIds old_ids = GetRoadIds(actor_id);

    if (road_ids.road_id != old_ids.road_id ||
        road_ids.section_id != old_ids.section_id ||
        road_ids.lane_id != old_ids.lane_id) {

      EraseVehicleId(actor_id, old_ids);
      SetVehicleId(actor_id, road_ids);
      SetRoadIds(actor_id, road_ids);
    }
  }

    void TrafficDistributor::DrawLaneChange(carla::road::element::LaneMarking::LaneChange lane_change, const Actor &ego_actor, cc::DebugHelper debug_helper) {
    std::string str;
    if (lane_change == carla::road::element::LaneMarking::LaneChange::Right) {
      str="Right";
      debug_helper.DrawString(
        cg::Location(ego_actor->GetLocation().x, ego_actor->GetLocation().y, ego_actor->GetLocation().z+1),
        str,
        false,
        {255u, 0u, 0u}, 0.1f, true);
    }

    else if (lane_change == carla::road::element::LaneMarking::LaneChange::Left){
      str="Left";
      debug_helper.DrawString(
        cg::Location(ego_actor->GetLocation().x, ego_actor->GetLocation().y, ego_actor->GetLocation().z+1),
        str,
        false,
        {0u, 255u, 0u}, 0.1f, true);
    }

    else if (lane_change == carla::road::element::LaneMarking::LaneChange::Both){
      str="Both";
      debug_helper.DrawString(
        cg::Location(ego_actor->GetLocation().x, ego_actor->GetLocation().y, ego_actor->GetLocation().z+1),
        str,
        false,
        {0u, 0u, 255u}, 0.1f, true);
    }
  }

  std::shared_ptr<SimpleWaypoint> TrafficDistributor::AssignLaneChange(
      Actor vehicle,
      std::shared_ptr<SimpleWaypoint> current_waypoint,
      GeoIds current_road_ids,
      std::shared_ptr<BufferList> buffer_list,
      std::unordered_map<ActorId, uint> &vehicle_id_to_index,
      std::vector<carla::SharedPtr<cc::Actor>> &actor_list,
      cc::DebugHelper &debug_helper,
      bool force,
      bool direction) {

    ActorId actor_id = vehicle->GetId();
    cg::Location vehicle_location = vehicle->GetLocation();
    float vehicle_velocity = vehicle->GetVelocity().Length();
    std::unordered_set<ActorId> co_lane_vehicles = GetVehicleIds(current_road_ids);

    bool need_to_change_lane = false;
    // true -> left, false -> right
    bool lane_change_direction;

    auto left_waypoint = current_waypoint->GetLeftWaypoint();
    auto right_waypoint = current_waypoint->GetRightWaypoint();

    auto lane_change = current_waypoint->GetWaypoint()->GetLaneChange();

    DrawLaneChange(lane_change, vehicle, debug_helper);

    auto change_right = carla::road::element::LaneMarking::LaneChange::Right;
    auto change_left = carla::road::element::LaneMarking::LaneChange::Left;
    auto change_both = carla::road::element::LaneMarking::LaneChange::Both;

    // Don't try to change lane if the current lane has less than two vehicles.
    if (co_lane_vehicles.size() >= 2 && !force) {

      // Check if any vehicle in the current lane is blocking us.
      for (auto i = co_lane_vehicles.begin(); i != co_lane_vehicles.end() && !need_to_change_lane; ++i) {

        const traffic_manager::ActorId &same_lane_vehicle_id = *i;
        traffic_manager::Buffer &other_vehicle_buffer = buffer_list->at(
            vehicle_id_to_index.at(same_lane_vehicle_id));

        std::shared_ptr<traffic_manager::SimpleWaypoint> same_lane_vehicle_waypoint = nullptr;
        cg::Location same_lane_location;
        if (!other_vehicle_buffer.empty()) {

          same_lane_vehicle_waypoint = buffer_list->at(
              vehicle_id_to_index.at(same_lane_vehicle_id)).front();
          same_lane_location = same_lane_vehicle_waypoint->GetLocation();
        }

        // Check if there is another vehicle in the current lane in front for
        // a threshold distance and current position not in a junction.
        if (same_lane_vehicle_id != actor_id &&
            same_lane_vehicle_waypoint != nullptr &&
            !same_lane_vehicle_waypoint->CheckJunction() &&
            DeviationDotProduct(vehicle, same_lane_location) > 0 &&
            (same_lane_location.Distance(vehicle_location)
            < LANE_CHANGE_OBSTACLE_DISTANCE) &&
            (same_lane_location.Distance(vehicle_location)
            > LANE_OBSTACLE_MINIMUM_DISTANCE)) {

          // If lane change connections are available,
          // pick a direction (preferring left) and
          // announce the need for a lane change.
          if (left_waypoint != nullptr && (lane_change == change_left || lane_change == change_both)) {
            traffic_manager::ActorIDSet left_lane_vehicles = GetVehicleIds({
              current_road_ids.road_id,
              current_road_ids.section_id,
              left_waypoint->GetWaypoint()->GetLaneId()
            });
            if (co_lane_vehicles.size() - left_lane_vehicles.size() > 1) {
              need_to_change_lane = true;
              lane_change_direction = true;
            }
          } else if (right_waypoint != nullptr && (lane_change == change_right || lane_change == change_both)) {
            traffic_manager::ActorIDSet right_lane_vehicles = GetVehicleIds({
              current_road_ids.road_id,
              current_road_ids.section_id,
              right_waypoint->GetWaypoint()->GetLaneId()
            });
            if (co_lane_vehicles.size() - right_lane_vehicles.size() > 1) {
              need_to_change_lane = true;
              lane_change_direction = false;
            }
          }
        }
      }
    } else if (force) {
      need_to_change_lane = true;
      lane_change_direction = direction;
    }

    // Change the distance to the target point on the target lane
    // as a function of vehicle velocity.
    float change_over_distance = std::max(vehicle_velocity, MINIMUM_LANE_CHANGE_DISTANCE);

    bool possible_to_lane_change = false;
    std::shared_ptr<traffic_manager::SimpleWaypoint> change_over_point;
    if (need_to_change_lane) {

      if (lane_change_direction) {
        change_over_point = left_waypoint;
      } else {
        change_over_point = right_waypoint;
      }

      if (change_over_point != nullptr) {
        carla::road::LaneId lane_change_id = change_over_point->GetWaypoint()->GetLaneId();
        traffic_manager::ActorIDSet target_lane_vehicles = GetVehicleIds({
          current_road_ids.road_id,
          current_road_ids.section_id,
          lane_change_id
        });

        // If target lane has vehicles, check if there are any obstacles
        // for lane change execution.
        if (target_lane_vehicles.size() > 0 && !force) {

          bool found_hazard = false;
          for (auto i = target_lane_vehicles.begin(); i != target_lane_vehicles.end() && !found_hazard; ++i) {

            const ActorId &other_vehicle_id = *i;
            traffic_manager::Buffer &other_vehicle_buffer = buffer_list->at(
                vehicle_id_to_index.at(other_vehicle_id));

            // If a vehicle on the target lane is behind us, check if we are
            // fast enough to execute lane change.
            if (!other_vehicle_buffer.empty() &&
                other_vehicle_buffer.front()->GetWaypoint()->GetLaneId() == lane_change_id) {

              Actor other_vehicle = actor_list.at(vehicle_id_to_index.at(other_vehicle_id));
              cg::Location other_vehicle_location = other_vehicle_buffer.front()->GetLocation();
              float relative_deviation = DeviationDotProduct(vehicle, other_vehicle_location);

              if (relative_deviation < 0) {

                float time_to_reach_other =
                    change_over_point->Distance(other_vehicle_location) /
                    other_vehicle->GetVelocity().Length();

                float time_to_reach_reference =
                    change_over_point->Distance(vehicle_location) /
                    vehicle->GetVelocity().Length();

                if (relative_deviation > std::cos(M_PI * LATERAL_DETECTION_CONE / 180) ||
                    time_to_reach_other > (time_to_reach_reference + APPROACHING_VEHICLE_TIME_MARGIN)) {

                  found_hazard = true;
                }
              }
              // If a vehicle on the target lane is in front, check if it is far
              // enough to perform a lane change.
              else {

                auto vehicle_reference = boost::static_pointer_cast<cc::Vehicle>(vehicle);
                if (change_over_point->Distance(other_vehicle_location) <
                    (1.0 + change_over_distance + vehicle_reference->GetBoundingBox().extent.x * 2)) {
                  found_hazard = true;
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
      auto starting_point = change_over_point;
      while (change_over_point->DistanceSquared(starting_point) < change_over_distance &&
             !change_over_point->CheckJunction()) {
        change_over_point = change_over_point->GetNextWaypoint()[0];
      }
      return change_over_point;
    } else {
      return nullptr;
    }
  }

  float DeviationCrossProduct(Actor actor, const cg::Location &target_location) {

    cg::Vector3D heading_vector = actor->GetTransform().GetForwardVector();
    heading_vector.z = 0;
    heading_vector = heading_vector.MakeUnitVector();
    cg::Location next_vector = target_location - actor->GetLocation();
    next_vector.z = 0;
    if (next_vector.Length() > 2.0f * std::numeric_limits<float>::epsilon()) {
      next_vector = next_vector.MakeUnitVector();
      float cross_z = heading_vector.x * next_vector.y - heading_vector.y * next_vector.x;
      return cross_z;
    } else {
      return 0;
    }
  }

  float DeviationDotProduct(Actor actor, const cg::Location &target_location) {

    cg::Vector3D heading_vector = actor->GetTransform().GetForwardVector();
    heading_vector.z = 0;
    heading_vector = heading_vector.MakeUnitVector();
    cg::Location next_vector = target_location - actor->GetLocation();
    next_vector.z = 0;
    if (next_vector.Length() > 2.0f * std::numeric_limits<float>::epsilon()) {
      next_vector = next_vector.MakeUnitVector();
      float dot_product = cg::Math::Dot(next_vector, heading_vector);
      return dot_product;
    } else {
      return 0;
    }
  }

}
