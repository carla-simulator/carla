#include "LocalizationStage.h"

namespace traffic_manager {

namespace LocalizationConstants {
  static const float WAYPOINT_TIME_HORIZON = 3.0f;
  static const float MINIMUM_HORIZON_LENGTH = 30.0f;
  static const float TARGET_WAYPOINT_TIME_HORIZON = 0.5f;
  static const float TARGET_WAYPOINT_HORIZON_LENGTH = 2.0f;
  static const float MINIMUM_JUNCTION_LOOK_AHEAD = 3.0f;
  static const float HIGHWAY_SPEED = 50 / 3.6f;
}
  using namespace LocalizationConstants;

  LocalizationStage::LocalizationStage(
      std::string stage_name,
      std::shared_ptr<LocalizationToPlannerMessenger> planner_messenger,
      std::shared_ptr<LocalizationToCollisionMessenger> collision_messenger,
      std::shared_ptr<LocalizationToTrafficLightMessenger> traffic_light_messenger,
      AtomicActorSet &registered_actors,
      InMemoryMap &local_map,
      Parameters &parameters,
      cc::DebugHelper &debug_helper)
    : PipelineStage(stage_name),
      planner_messenger(planner_messenger),
      collision_messenger(collision_messenger),
      traffic_light_messenger(traffic_light_messenger),
      registered_actors(registered_actors),
      local_map(local_map),
      parameters(parameters),
      debug_helper(debug_helper) {

    // Initializing various output frame selectors.
    planner_frame_selector = true;
    collision_frame_selector = true;
    collision_frame_ready = false;
    traffic_light_frame_selector = true;
    // Initializing the number of vehicles to zero in the begining.
    number_of_vehicles = 0u;

    // Initializing messenger states to initiate data writes
    // preemptively since this is the first stage in the pipeline.
    planner_messenger_state = planner_messenger->GetState() - 1;
    collision_messenger_state = collision_messenger->GetState() - 1;
    traffic_light_messenger_state = traffic_light_messenger->GetState() - 1;
    // Initializing the registered actors container state.
    registered_actors_state = -1;
  }

  LocalizationStage::~LocalizationStage() {}

  void LocalizationStage::Action() {

    // Selecting output frames based on selector keys.
    auto current_planner_frame = planner_frame_selector ? planner_frame_a : planner_frame_b;
    auto current_collision_frame = collision_frame_selector ? collision_frame_a : collision_frame_b;
    auto current_traffic_light_frame =
        traffic_light_frame_selector ? traffic_light_frame_a : traffic_light_frame_b;

    // Looping over registered actors.
    for (uint i = 0u; i < actor_list.size(); ++i) {

      Actor vehicle = actor_list.at(i);
      ActorId actor_id = vehicle->GetId();
      cg::Location vehicle_location = vehicle->GetLocation();
      float vehicle_velocity = vehicle->GetVelocity().Length();

      float horizon_size = std::max(
          WAYPOINT_TIME_HORIZON * std::sqrt(vehicle_velocity * 10.0f),
          MINIMUM_HORIZON_LENGTH);

      Buffer &waypoint_buffer = buffer_list->at(i);

      // Purge passed waypoints.
      if (!waypoint_buffer.empty()) {

        float dot_product = DeviationDotProduct(vehicle, waypoint_buffer.front()->GetLocation());

        while (dot_product <= 0 && !waypoint_buffer.empty()) {

          PopWaypoint(waypoint_buffer, actor_id);
          if (!waypoint_buffer.empty()) {
            dot_product = DeviationDotProduct(vehicle, waypoint_buffer.front()->GetLocation());
          }
        }
      }

      // Initializing buffer if it is empty.
      if (waypoint_buffer.empty()) {
        SimpleWaypointPtr closest_waypoint = local_map.GetWaypointInVicinity(vehicle_location);
        if (closest_waypoint == nullptr) {
          closest_waypoint = local_map.GetWaypoint(vehicle_location);
        }
        PushWaypoint(waypoint_buffer, actor_id, closest_waypoint);
      }

      // Assign a lane change.
      SimpleWaypointPtr front_waypoint = waypoint_buffer.front();
      ChangeLaneInfo lane_change_info = parameters.GetForceLaneChange(vehicle);
      bool force_lane_change = lane_change_info.change_lane;
      bool lane_change_direction = lane_change_info.direction;

      if ((parameters.GetAutoLaneChange(vehicle) || force_lane_change) &&
          !front_waypoint->CheckJunction()) {

        SimpleWaypointPtr change_over_point = AssignLaneChange(
            vehicle, force_lane_change, lane_change_direction);

        if (change_over_point != nullptr) {
          auto number_of_pops = waypoint_buffer.size();
          for (uint j = 0; j < number_of_pops; ++j) {
            PopWaypoint(waypoint_buffer, actor_id);
          }

          PushWaypoint(waypoint_buffer, actor_id, change_over_point);
        }
      }

      // Populating the buffer.
      while (waypoint_buffer.back()->DistanceSquared(waypoint_buffer.front())
          <= std::pow(horizon_size, 2)) {

        std::vector<SimpleWaypointPtr> next_waypoints = waypoint_buffer.back()->GetNextWaypoint();
        uint selection_index = 0u;
        // Pseudo-randomized path selection if found more than one choice.
        if (next_waypoints.size() > 1) {
          selection_index = static_cast<uint>(rand()) % next_waypoints.size();
        }

        PushWaypoint(waypoint_buffer, actor_id, next_waypoints.at(selection_index));
      }

      // Generating output.
      float target_point_distance = std::max(std::ceil(vehicle_velocity * TARGET_WAYPOINT_TIME_HORIZON),
          TARGET_WAYPOINT_HORIZON_LENGTH);
      SimpleWaypointPtr target_waypoint = waypoint_buffer.front();
      for (uint j = 0u;
          (j < waypoint_buffer.size()) &&
          (waypoint_buffer.front()->DistanceSquared(target_waypoint)
          < std::pow(target_point_distance, 2));
          ++j) {
        target_waypoint = waypoint_buffer.at(j);
      }
      cg::Location target_location = target_waypoint->GetLocation();
      float dot_product = DeviationDotProduct(vehicle, target_location);
      float cross_product = DeviationCrossProduct(vehicle, target_location);
      dot_product = 1 - dot_product;
      if (cross_product < 0) {
        dot_product *= -1;
      }

      // Filtering out false junctions on highways.
      // On highways, if there is only one possible path and the section is
      // marked as intersection, ignore it.
      auto vehicle_reference = boost::static_pointer_cast<cc::Vehicle>(vehicle);
      float speed_limit = vehicle_reference->GetSpeedLimit();
      float look_ahead_distance = std::max(2 * vehicle_velocity, MINIMUM_JUNCTION_LOOK_AHEAD);

      SimpleWaypointPtr look_ahead_point = waypoint_buffer.front();
      uint look_ahead_index = 0u;
      for (uint j = 0u;
          (waypoint_buffer.front()->DistanceSquared(look_ahead_point)
          < std::pow(look_ahead_distance, 2)) &&
          (j < waypoint_buffer.size());
          ++j) {
        look_ahead_point = waypoint_buffer.at(j);
        look_ahead_index = j;
      }

      bool approaching_junction = false;
      if (look_ahead_point->CheckJunction() && !(waypoint_buffer.front()->CheckJunction())) {
        if (speed_limit > HIGHWAY_SPEED) {
          for (uint j = 0u; (j < look_ahead_index) && !approaching_junction; ++j) {
            SimpleWaypointPtr swp = waypoint_buffer.at(j);
            if (swp->GetNextWaypoint().size() > 1) {
              approaching_junction = true;
            }
          }
        } else {
          approaching_junction = true;
        }
      }

      // Editing output frames.
      LocalizationToPlannerData &planner_message = current_planner_frame->at(i);
      planner_message.actor = vehicle;
      planner_message.deviation = dot_product;
      planner_message.approaching_true_junction = approaching_junction;

      // Reading current messenger state of the collision stage before modifying it's frame.
      if ((collision_messenger->GetState() != collision_messenger_state) &&
          !collision_frame_ready) {

        LocalizationToCollisionData &collision_message = current_collision_frame->at(i);
        collision_message.actor = vehicle;
        collision_message.buffer = waypoint_buffer;
        collision_message.overlapping_actors = GetOverlappingVehicles(actor_id);
      }

      LocalizationToTrafficLightData &traffic_light_message = current_traffic_light_frame->at(i);
      traffic_light_message.actor = vehicle;
      traffic_light_message.closest_waypoint = waypoint_buffer.front();
      traffic_light_message.junction_look_ahead_waypoint = waypoint_buffer.at(look_ahead_index);
    }

    if ((collision_messenger->GetState() != collision_messenger_state)
        && !collision_frame_ready) {

      collision_frame_ready = true;
    }

  }

  void LocalizationStage::PushWaypoint(Buffer& buffer, ActorId actor_id, SimpleWaypointPtr& waypoint) {

    uint64_t waypoint_id = waypoint->GetId();
    buffer.push_back(waypoint);
    UpdateOverlappingWaypoint(waypoint_id, actor_id);

    ActorIdSet current_actors = GetOverlappingVehicles(actor_id);
    ActorIdSet new_overlapping_actors = GetPassingVehicles(waypoint_id);
    ActorIdSet actor_set_difference;

    std::set_difference(
      new_overlapping_actors.begin(), new_overlapping_actors.end(),
      current_actors.begin(), current_actors.end(),
      std::inserter(actor_set_difference, actor_set_difference.end())
    );

    for (auto new_actor_id: actor_set_difference) {

      UpdateOverlappingVehicle(actor_id, new_actor_id);
      UpdateOverlappingVehicle(new_actor_id, actor_id);
    }
  }

  void LocalizationStage::PopWaypoint(Buffer& buffer, ActorId actor_id) {

    uint64_t removed_waypoint_id = buffer.front()->GetId();
    buffer.pop_front();
    RemoveOverlappingWaypoint(removed_waypoint_id, actor_id);

    if (!buffer.empty()) {
      ActorIdSet current_actors = GetPassingVehicles(removed_waypoint_id);
      ActorIdSet new_overlapping_actors = GetPassingVehicles(buffer.front()->GetId());
      ActorIdSet actor_set_difference;

      std::set_difference(
        current_actors.begin(), current_actors.end(),
        new_overlapping_actors.begin(), new_overlapping_actors.end(),
        std::inserter(actor_set_difference, actor_set_difference.end())
      );

      for (auto old_actor_id: actor_set_difference) {

        RemoveOverlappingVehicle(actor_id, old_actor_id);
        RemoveOverlappingVehicle(old_actor_id, actor_id);
      }
    }
  }

    void LocalizationStage::UpdateOverlappingVehicle(ActorId actor_id, ActorId other_id) {

      if (overlapping_vehicles.find(actor_id) != overlapping_vehicles.end()) {
        ActorIdSet& actor_set = overlapping_vehicles.at(actor_id);
        if (actor_set.find(other_id) == actor_set.end()) {
          actor_set.insert(other_id);
        }
      } else {
        overlapping_vehicles.insert({actor_id, {other_id}});
      }
    }

    void LocalizationStage::RemoveOverlappingVehicle(ActorId actor_id, ActorId other_id) {

      if (overlapping_vehicles.find(actor_id) != overlapping_vehicles.end()) {
        ActorIdSet& actor_set = overlapping_vehicles.at(actor_id);
        if (actor_set.find(actor_id) != actor_set.end()) {
          actor_set.erase(other_id);
        }
      }
    }

    ActorIdSet LocalizationStage::GetOverlappingVehicles(ActorId actor_id) {

      if (overlapping_vehicles.find(actor_id) != overlapping_vehicles.end()) {
        return overlapping_vehicles.at(actor_id);
      } else {
        return ActorIdSet();
      }
    }


  void LocalizationStage::UpdateOverlappingWaypoint(uint64_t waypoint_id, ActorId actor_id) {

    if (waypoint_overlap_tracker.find(waypoint_id) != waypoint_overlap_tracker.end()) {

      ActorIdSet& actor_id_set = waypoint_overlap_tracker.at(waypoint_id);
      if (actor_id_set.find(actor_id) == actor_id_set.end()) {
        actor_id_set.insert(actor_id);
      }
    } else {

      waypoint_overlap_tracker.insert({waypoint_id, {actor_id}});
    }

  }

  void LocalizationStage::RemoveOverlappingWaypoint(uint64_t waypoint_id, ActorId actor_id) {

    if (waypoint_overlap_tracker.find(waypoint_id) != waypoint_overlap_tracker.end()) {

      auto& actor_id_set = waypoint_overlap_tracker.at(waypoint_id);
      if (actor_id_set.find(actor_id) != actor_id_set.end()) {
        actor_id_set.erase(actor_id);
      }

      if (actor_id_set.size() == 0) {
        waypoint_overlap_tracker.erase(waypoint_id);
      }
    }
  }

  ActorIdSet LocalizationStage::GetPassingVehicles(uint64_t waypoint_id) {

    if (waypoint_overlap_tracker.find(waypoint_id) != waypoint_overlap_tracker.end()) {
      return waypoint_overlap_tracker.at(waypoint_id);
    } else {
      return ActorIdSet();
    }
  }

  void LocalizationStage::DataReceiver() {

    // Building a list of registered actors and
    // connecting the vehicle ids to their position indices on data arrays.

    if (registered_actors_state != registered_actors.GetState()) {

      actor_list = registered_actors.GetList();

      uint index = 0u;
      for (auto &actor: actor_list) {

        vehicle_id_to_index.insert({actor->GetId(), index});
        ++index;
      }

      registered_actors_state = registered_actors.GetState();
    }

    // Allocating new containers for the changed number of registered vehicles.
    if (number_of_vehicles != actor_list.size()) {

      number_of_vehicles = static_cast<uint>(actor_list.size());
      // Allocating the buffer lists.
      buffer_list = std::make_shared<BufferList>(number_of_vehicles);
      // Allocating output frames to be shared with the motion planner stage.
      planner_frame_a = std::make_shared<LocalizationToPlannerFrame>(number_of_vehicles);
      planner_frame_b = std::make_shared<LocalizationToPlannerFrame>(number_of_vehicles);
      // Allocating output frames to be shared with the collision stage.
      collision_frame_a = std::make_shared<LocalizationToCollisionFrame>(number_of_vehicles);
      collision_frame_b = std::make_shared<LocalizationToCollisionFrame>(number_of_vehicles);
      // Allocating output frames to be shared with the traffic light stage
      traffic_light_frame_a = std::make_shared<LocalizationToTrafficLightFrame>(number_of_vehicles);
      traffic_light_frame_b = std::make_shared<LocalizationToTrafficLightFrame>(number_of_vehicles);
    }

  }

  void LocalizationStage::DataSender() {

    // Since send/receive calls on messenger objects can block if the other
    // end hasn't received/sent data, choose to block on only those stages
    // which takes the most priority (which needs the highest rate of data feed)
    // to run the system well.

    DataPacket<std::shared_ptr<LocalizationToPlannerFrame>> planner_data_packet = {
      planner_messenger_state,
      planner_frame_selector ? planner_frame_a : planner_frame_b
    };
    planner_frame_selector = !planner_frame_selector;
    planner_messenger_state = planner_messenger->SendData(planner_data_packet);

    // Send data to collision stage only if it has finished
    // processing, received the previous message and started processing it.
    int collision_messenger_current_state = collision_messenger->GetState();
    if ((collision_messenger_current_state != collision_messenger_state) &&
        collision_frame_ready) {

      DataPacket<std::shared_ptr<LocalizationToCollisionFrame>> collision_data_packet = {
        collision_messenger_state,
        collision_frame_selector ? collision_frame_a : collision_frame_b
      };

      collision_messenger_state = collision_messenger->SendData(collision_data_packet);
      collision_frame_selector = !collision_frame_selector;
      collision_frame_ready = false;
    }

    // Send data to traffic light stage only if it has finished
    // processing, received the previous message and started processing it.
    int traffic_light_messenger_current_state = traffic_light_messenger->GetState();
    if (traffic_light_messenger_current_state != traffic_light_messenger_state) {
      DataPacket<std::shared_ptr<LocalizationToTrafficLightFrame>> traffic_light_data_packet = {
        traffic_light_messenger_state,
        traffic_light_frame_selector ? traffic_light_frame_a : traffic_light_frame_b
      };

      traffic_light_messenger_state = traffic_light_messenger->SendData(traffic_light_data_packet);
      traffic_light_frame_selector = !traffic_light_frame_selector;
    }
  }

  void LocalizationStage::DrawBuffer(Buffer &buffer) {

    for (uint i = 0u; i < buffer.size() && i < 5; ++i) {
      debug_helper.DrawPoint(buffer.at(i)->GetLocation(), 0.1f, {255u, 0u, 0u}, 0.5f);
    }
  }

  float LocalizationStage::DeviationCrossProduct(Actor actor, const cg::Location &target_location) {

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

  float LocalizationStage::DeviationDotProduct(Actor actor, const cg::Location &target_location) {

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

  SimpleWaypointPtr LocalizationStage::AssignLaneChange(Actor vehicle, bool force, bool direction) {

    ActorId actor_id = vehicle->GetId();
    cg::Location vehicle_location = vehicle->GetLocation();
    float vehicle_velocity = vehicle->GetVelocity().Length();

    Buffer& waypoint_buffer = buffer_list->at(vehicle_id_to_index.at(actor_id));
    SimpleWaypointPtr current_waypoint = waypoint_buffer.front();

    bool need_to_change_lane = false;
    auto left_waypoint = current_waypoint->GetLeftWaypoint();
    auto right_waypoint = current_waypoint->GetRightWaypoint();
    auto lane_change = current_waypoint->GetWaypoint()->GetLaneChange();

    if (!force) {

      auto current_collision_frame = collision_frame_selector ? collision_frame_a : collision_frame_b;
      uint vehicle_index = vehicle_id_to_index.at(actor_id);

      ActorIdSet overlapping_vehicles = GetOverlappingVehicles(actor_id);

      // Check if any vehicle in the current lane is blocking us.
      bool abort_lane_change = false;
      for (auto i = overlapping_vehicles.begin();
           i != overlapping_vehicles.end() && !abort_lane_change;
           ++i) {

        const ActorId &other_vehicle_id = *i;
        Buffer& other_buffer = buffer_list->at(vehicle_id_to_index.at(other_vehicle_id));
        SimpleWaypointPtr& other_current_waypoint = other_buffer.front();
        cg::Location other_location = other_current_waypoint->GetLocation();

        // debug_helper.DrawArrow(
        //   vehicle_location + cg::Location(0,0,4),
        //   other_location + cg::Location(0,0,4),
        //   0.2f, 0.2f, {0u, 0u, 255u}, 0.1f);

        // Check if there is another vehicle in the front within
        // a threshold distance and current position not in a junction.
        if (!current_waypoint->CheckJunction()) {
          if (other_vehicle_id != actor_id &&
              DeviationDotProduct(vehicle, other_location) > 0.0f &&
              // Account for these constants and make them speed dependent.
              other_location.Distance(vehicle_location) < 20.f &&
              other_location.Distance(vehicle_location) > 10.0f) {

            need_to_change_lane = true;
          } else if (DeviationDotProduct(vehicle, other_location) > 0.0f &&
                     other_location.Distance(vehicle_location) < 10.0f) {

            need_to_change_lane = false;
            abort_lane_change = true;
          }
        }
      }

    } else {

      need_to_change_lane = true;
    }

    float change_over_distance = std::max(2.0f*vehicle_velocity, 10.0f);
    bool possible_to_lane_change = false;
    SimpleWaypointPtr change_over_point = nullptr;

    if (need_to_change_lane) {

      std::vector<SimpleWaypointPtr> candidate_points;
      if (force) {
        if (direction) {
          candidate_points.push_back(left_waypoint);
        } else {
          candidate_points.push_back(right_waypoint);
        }
      } else {
        candidate_points.push_back(left_waypoint);
        candidate_points.push_back(right_waypoint);
      }

      for (auto target_lane_wp: candidate_points) {
        if (!possible_to_lane_change && target_lane_wp != nullptr) {

          auto target_lane_vehicles = GetPassingVehicles(target_lane_wp->GetId());

          // If target lane has vehicles, check if there are any obstacles
          // for lane change execution.
          if (target_lane_vehicles.size() > 0 && !force) {

            bool found_hazard = false;
            for (auto i = target_lane_vehicles.begin(); i != target_lane_vehicles.end() && !found_hazard; ++i) {

              const ActorId &other_vehicle_id = *i;
              traffic_manager::Buffer &other_vehicle_buffer = buffer_list->at(
                  vehicle_id_to_index.at(other_vehicle_id));
              cg::Location other_vehicle_location = other_vehicle_buffer.front()->GetLocation();
              float relative_deviation = DeviationDotProduct(vehicle, other_vehicle_location);

              // If there is a vehicle approaching from behind, do not change lane.
              if (relative_deviation < 0.0f) {
                found_hazard = true;
              }
              // If a vehicle on the target lane is in front, check if it is far
              // enough to perform a lane change.
              else {

                auto vehicle_reference = boost::static_pointer_cast<cc::Vehicle>(vehicle);
                if (target_lane_wp->Distance(other_vehicle_location) <
                    (1.0 + change_over_distance + vehicle_reference->GetBoundingBox().extent.x * 2)) {
                  found_hazard = true;
                }
              }
            }

            if (!found_hazard) {

              possible_to_lane_change = true;
            }
          } else {

            possible_to_lane_change = true;
          }

          if (possible_to_lane_change) {
            change_over_point = target_lane_wp;
          }
        }
      }
    }

    // debug_helper.DrawString(vehicle_location + cg::Location(0,0,2),
    //                         std::to_string(need_to_change_lane), false, {255u, 0u, 0u}, 0.1f);

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

}
