#include "LocalizationStage.h"

namespace traffic_manager {

  const float WAYPOINT_TIME_HORIZON = 3.0;
  const float MINIMUM_HORIZON_LENGTH = 25.0;
  const float TARGET_WAYPOINT_TIME_HORIZON = 0.5;
  const float TARGET_WAYPOINT_HORIZON_LENGTH = 2.0;
  const int MINIMUM_JUNCTION_LOOK_AHEAD = 5;
  const float HIGHWAY_SPEED = 50 / 3.6;

  LocalizationStage::LocalizationStage (
      std::shared_ptr<LocalizationToPlannerMessenger> planner_messenger,
      std::shared_ptr<LocalizationToCollisionMessenger> collision_messenger,
      std::shared_ptr<LocalizationToTrafficLightMessenger> traffic_light_messenger,
      int number_of_vehicles,
      int pool_size,
      std::vector<carla::SharedPtr<carla::client::Actor>>& actor_list,
      InMemoryMap& local_map,
      carla::client::DebugHelper& debug_helper
  ) :
  planner_messenger(planner_messenger),
  collision_messenger(collision_messenger),
  traffic_light_messenger(traffic_light_messenger),
  actor_list(actor_list),
  local_map(local_map),
  debug_helper(debug_helper),
  PipelineStage(pool_size, number_of_vehicles),
  last_lane_change_location(std::vector<carla::geom::Location>(number_of_vehicles))
  {

    planner_frame_selector = true;
    collision_frame_selector = true;
    traffic_light_frame_selector = true;

    buffer_list_a = std::make_shared<BufferList>(number_of_vehicles);
    buffer_list_b = std::make_shared<BufferList>(number_of_vehicles);

    buffer_map.insert({true, buffer_list_a});
    buffer_map.insert({false, buffer_list_b});

    planner_frame_a = std::make_shared<LocalizationToPlannerFrame>(number_of_vehicles);
    planner_frame_b = std::make_shared<LocalizationToPlannerFrame>(number_of_vehicles);

    planner_frame_map.insert({true, planner_frame_a});
    planner_frame_map.insert({false, planner_frame_b});

    collision_frame_a = std::make_shared<LocalizationToCollisionFrame>(number_of_vehicles);
    collision_frame_b = std::make_shared<LocalizationToCollisionFrame>(number_of_vehicles);

    collision_frame_map.insert({true, collision_frame_a});
    collision_frame_map.insert({false, collision_frame_b});

    traffic_light_frame_a = std::make_shared<LocalizationToTrafficLightFrame>(number_of_vehicles);
    traffic_light_frame_b = std::make_shared<LocalizationToTrafficLightFrame>(number_of_vehicles);

    traffic_light_frame_map.insert({true, traffic_light_frame_a});
    traffic_light_frame_map.insert({false, traffic_light_frame_b});

    planner_messenger_state = planner_messenger->GetState() -1;
    collision_messenger_state = collision_messenger->GetState() -1;
    traffic_light_messenger_state = traffic_light_messenger->GetState() -1;

    for (int i=0; i <number_of_vehicles; i++) {
      divergence_choice.push_back(rand());
    }

    int index=0;
    for (auto& actor: actor_list) {
      vehicle_id_to_index.insert({actor->GetId(), index});
      index++;
    }

    for (int i=0; i < actor_list.size(); i++) {
      last_lane_change_location.at(i) = actor_list.at(i)->GetLocation();
      index++;
    }
  }

  LocalizationStage::~LocalizationStage() {}

  void LocalizationStage::Action(int start_index, int end_index) {

    for (int i = start_index; i <= end_index; i++) {
      // std::this_thread::sleep_for(1s);

      auto vehicle = actor_list.at(i);
      auto actor_id = vehicle->GetId();

      auto vehicle_location = vehicle->GetLocation();
      auto vehicle_velocity = vehicle->GetVelocity().Length();

      float horizon_size = std::max(
          WAYPOINT_TIME_HORIZON * vehicle_velocity,
          MINIMUM_HORIZON_LENGTH);

      auto& waypoint_buffer = buffer_map.at(collision_frame_selector)->at(i);
      auto& copy_waypoint_buffer = buffer_map.at(!collision_frame_selector)->at(i);

      /// Sync lane change from buffer copy
      if (
        !waypoint_buffer.empty() and !copy_waypoint_buffer.empty()
        and
        (
          copy_waypoint_buffer.front()->getWaypoint()->GetLaneId()
            != waypoint_buffer.front()->getWaypoint()->GetLaneId()
          or
          copy_waypoint_buffer.front()->getWaypoint()->GetSectionId()
            != waypoint_buffer.front()->getWaypoint()->GetSectionId()
        )
      ) {
        waypoint_buffer.clear();
        waypoint_buffer.assign(copy_waypoint_buffer.begin(), copy_waypoint_buffer.end());
      }

      /// Purge passed waypoints
      if (!waypoint_buffer.empty()) {
        auto dot_product = DeviationDotProduct(
          vehicle,
          waypoint_buffer.front()->getLocation());
        while (dot_product <= 0) {
          waypoint_buffer.pop_front();
          if (!waypoint_buffer.empty()) {
            dot_product = DeviationDotProduct(
              vehicle,
              waypoint_buffer.front()->getLocation());
          } else {
            break;
          }
        }
      }

      /// Initialize buffer if empty
      if (waypoint_buffer.empty()) {
        auto closest_waypoint = local_map.getWaypoint(vehicle_location);
        waypoint_buffer.push_back(closest_waypoint);
      }

      // Assign lane change
      auto front_waypoint = waypoint_buffer.front();
      GeoIds current_road_ids = {
        front_waypoint->getWaypoint()->GetRoadId(),
        front_waypoint->getWaypoint()->GetSectionId(),
        front_waypoint->getWaypoint()->GetLaneId()
      };

      traffic_distribution.UpdateVehicleRoadPosition(
        actor_id,
        current_road_ids
      );

      auto current_road_key = traffic_distribution.MakeRoadKey(current_road_ids);
      // debug_helper.DrawString(
      //   vehicle_location+carla::geom::Location(0, 0, 2),
      //   std::to_string(traffic_distribution.GetVehicleIds(current_road_key).size()),
      //   false, {0, 0, 255}, 0.1f
      // );

      if (
        last_lane_change_location.at(i).Distance(vehicle_location) > 15
        and 
        !front_waypoint->checkJunction()
      ) {

        auto co_lane_vehicles = traffic_distribution.GetVehicleIds(current_road_key);

        bool need_to_change_lane = false;
        if (co_lane_vehicles.size() >= 2) {
          for (auto& same_lane_vehicle_id: co_lane_vehicles) {

            auto& other_vehicle_buffer = buffer_map.at(collision_frame_selector)
              ->at(vehicle_id_to_index.at(same_lane_vehicle_id));

            std::shared_ptr<traffic_manager::SimpleWaypoint> same_lane_vehicle_waypoint;
            if (!other_vehicle_buffer.empty()) {
              same_lane_vehicle_waypoint = buffer_map.at(collision_frame_selector)
                ->at(vehicle_id_to_index.at(same_lane_vehicle_id)).front();
            }

            if (
              same_lane_vehicle_id != actor_id
              and
              !other_vehicle_buffer.empty()
              and
              DeviationDotProduct(
                vehicle, 
                same_lane_vehicle_waypoint->getLocation()
              ) > 0   // check other vehicle is ahead
              and
              same_lane_vehicle_waypoint->getLocation().Distance(vehicle_location) < 20   // meters
            ) {
              need_to_change_lane = true;
              break;
            }
          }
        }

        // need_to_change_lane = true;
        bool possible_to_lane_change = true;
        std::shared_ptr<traffic_manager::SimpleWaypoint> change_over_point;
        if (need_to_change_lane) {
          auto left_waypoint = front_waypoint->getLeftWaypoint();
          auto right_waypoint = front_waypoint->getRightWaypoint();

          if (left_waypoint == nullptr and right_waypoint == nullptr) {
            possible_to_lane_change = false;
          } else {

            for (auto lane_change_point: {left_waypoint, right_waypoint}) {
              if (lane_change_point != nullptr) {

                change_over_point = lane_change_point;
                auto lane_change_id = lane_change_point->getWaypoint()->GetLaneId();
                auto target_lane_key = traffic_distribution.MakeRoadKey({
                  current_road_ids.road_id,
                  current_road_ids.section_id,
                  lane_change_id
                });
                auto target_lane_vehicles = traffic_distribution.GetVehicleIds(target_lane_key);

                if (target_lane_vehicles.size() > 0) {

                  for (auto other_vehicle_id: target_lane_vehicles) {
                    
                    auto& other_vehicle_buffer = buffer_map.at(collision_frame_selector)
                      ->at(vehicle_id_to_index.at(other_vehicle_id));
                    
                    if (
                      !other_vehicle_buffer.empty()
                      and 
                      other_vehicle_buffer.front()->getWaypoint()->GetLaneId()
                      == lane_change_id
                    ) {
                      if (
                        abs(DeviationDotProduct(
                          vehicle, other_vehicle_buffer.front()->getLocation())
                        ) < 0.71
                      ) {
                        possible_to_lane_change = false;
                        break;
                      }
                    } else {
                      possible_to_lane_change = false;
                    }
                  }
                }
                if (possible_to_lane_change) {
                  break;
                }
              }
            }
          }
        } else {
          possible_to_lane_change = false;
        }

        if (need_to_change_lane and possible_to_lane_change) {
          int change_over_distance = static_cast<int>(
            std::max(std::ceil(0.5 * vehicle_velocity), 5.0)  // Account for constants
          );
          for (int i = change_over_distance; i >=0; i--) {
            change_over_point = change_over_point->getNextWaypoint()[0];
          }
          waypoint_buffer.clear();
          waypoint_buffer.push_back(change_over_point);
          last_lane_change_location.at(i) = vehicle_location;
        }
      }

      /// Populate buffer
      while (
        waypoint_buffer.back()->distance(
        waypoint_buffer.front()->getLocation()) <= horizon_size
      ) {

        auto way_front = waypoint_buffer.back();
        auto pre_selection_id = way_front->getWaypoint()->GetId();
        auto next_waypoints = way_front->getNextWaypoint();
        auto selection_index = 0;
        if (next_waypoints.size() > 1) {
          selection_index = divergence_choice.at(i) * (1+ pre_selection_id)% next_waypoints.size();
        }

        way_front = next_waypoints.at(selection_index);
        waypoint_buffer.push_back(way_front);
      }

      // drawBuffer(waypoint_buffer);

      /// Generate output
      auto horizon_index = static_cast<int>(
        std::max(
          std::ceil(vehicle_velocity * TARGET_WAYPOINT_TIME_HORIZON),
          TARGET_WAYPOINT_HORIZON_LENGTH
        )
      );
      auto target_waypoint = waypoint_buffer.at(horizon_index);
      auto dot_product = DeviationDotProduct(vehicle, target_waypoint->getLocation());
      float cross_product = DeviationCrossProduct(vehicle, target_waypoint->getLocation());
      dot_product = 1 - dot_product;
      if (cross_product < 0) {
        dot_product *= -1;
      }

      // Filtering out false junctions on highways
      auto vehicle_reference = boost::static_pointer_cast<carla::client::Vehicle>(vehicle);
      auto speed_limit = vehicle_reference->GetSpeedLimit();
      int look_ahead_index = std::max(
        static_cast<int>(std::floor(2*vehicle_velocity)),
        MINIMUM_JUNCTION_LOOK_AHEAD
      );

      std::shared_ptr<SimpleWaypoint> look_ahead_point;
      if (waypoint_buffer.size() > look_ahead_index) {
        look_ahead_point = waypoint_buffer.at(look_ahead_index);
      } else {
        look_ahead_point =  waypoint_buffer.back();
      }

      bool approaching_junction = false;
      if (
        look_ahead_point->checkJunction()
        and
        !(waypoint_buffer.front()->checkJunction())
      ) {
        if (speed_limit > HIGHWAY_SPEED) {
          for (int i=0; i<look_ahead_index; i++) {
            auto swp = waypoint_buffer.at(i);
            if (swp->getNextWaypoint().size() > 1) {
              approaching_junction = true;
              break;
            }
          }
        } else {
          approaching_junction = true;
        }
      }

      // Editing output frames

      auto& planner_message = planner_frame_map.at(planner_frame_selector)->at(i);
      planner_message.actor = vehicle;
      planner_message.deviation = dot_product;
      planner_message.approaching_true_junction = approaching_junction;

      auto& collision_message = collision_frame_map.at(collision_frame_selector)->at(i);
      collision_message.actor = vehicle;
      collision_message.buffer = &waypoint_buffer;

      auto& traffic_light_message = traffic_light_frame_map.at(traffic_light_frame_selector)->at(i);
      traffic_light_message.actor = vehicle;
      traffic_light_message.closest_waypoint = waypoint_buffer.front();
      traffic_light_message.junction_look_ahead_waypoint = waypoint_buffer.at(look_ahead_index);

    }
  }

  void LocalizationStage::DataReceiver() {
  }

  void LocalizationStage::DataSender() {

    DataPacket<std::shared_ptr<LocalizationToPlannerFrame>> planner_data_packet = {
      planner_messenger_state,
      planner_frame_map.at(planner_frame_selector)
    };
    planner_frame_selector = !planner_frame_selector;
    planner_messenger_state = planner_messenger->SendData(planner_data_packet);

    auto collision_messenger_current_state = collision_messenger->GetState();
    if (collision_messenger_current_state != collision_messenger_state) {
      DataPacket<std::shared_ptr<LocalizationToCollisionFrame>> collision_data_packet = {
        collision_messenger_state,
        collision_frame_map.at(collision_frame_selector)
      };

      collision_messenger_state = collision_messenger->SendData(collision_data_packet);
      collision_frame_selector = !collision_frame_selector;
    }

    DataPacket<std::shared_ptr<LocalizationToTrafficLightFrame>> traffic_light_data_packet = {
      traffic_light_messenger_state,
      traffic_light_frame_map.at(traffic_light_frame_selector)
    };
    auto traffic_light_messenger_current_state = traffic_light_messenger->GetState();
    if (traffic_light_messenger_current_state != traffic_light_messenger_state) {
      traffic_light_messenger_state = traffic_light_messenger->SendData(traffic_light_data_packet);
      traffic_light_frame_selector = !traffic_light_frame_selector;
    }
  }

  float LocalizationStage::DeviationDotProduct(
      carla::SharedPtr<carla::client::Actor> actor,
      const carla::geom::Location &target_location) const {
    auto heading_vector = actor->GetTransform().GetForwardVector();
    auto next_vector = target_location - actor->GetLocation();
    next_vector = next_vector.MakeUnitVector();
    auto dot_product = next_vector.x * heading_vector.x +
        next_vector.y * heading_vector.y + next_vector.z * heading_vector.z;
    return dot_product;
  }

  float LocalizationStage::DeviationCrossProduct(
      carla::SharedPtr<carla::client::Actor> actor,
      const carla::geom::Location &target_location) const {
    auto heading_vector = actor->GetTransform().GetForwardVector();
    auto next_vector = target_location - actor->GetLocation();
    next_vector = next_vector.MakeUnitVector();
    float cross_z = heading_vector.x * next_vector.y - heading_vector.y * next_vector.x;
    return cross_z;
  }

   void LocalizationStage::drawBuffer(Buffer& buffer) {

    for (int i = 0; i<buffer.size(); i++) {
      debug_helper.DrawPoint(buffer.at(i)->getLocation(), 0.1f, {255U, 0U, 0U}, 0.1f);
    }
  }
}
