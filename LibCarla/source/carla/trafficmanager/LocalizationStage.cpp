// Copyright (c) 2019 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "LocalizationStage.h"

namespace carla {
namespace traffic_manager {

namespace LocalizationConstants {

  static const float WAYPOINT_TIME_HORIZON = 3.0f;
  static const float MINIMUM_HORIZON_LENGTH = 30.0f;
  static const float TARGET_WAYPOINT_TIME_HORIZON = 0.5f;
  static const float TARGET_WAYPOINT_HORIZON_LENGTH = 4.0f;
  static const float MINIMUM_JUNCTION_LOOK_AHEAD = 10.0f;
  static const float HIGHWAY_SPEED = 50 / 3.6f;
  static const float MINIMUM_LANE_CHANGE_DISTANCE = 20.0f;
  static const float MAXIMUM_LANE_OBSTACLE_CURVATURE = 0.93969f;
  static const uint UNREGISTERED_ACTORS_SCAN_INTERVAL = 10;

} // namespace LocalizationConstants

  using namespace LocalizationConstants;

  LocalizationStage::LocalizationStage(
      std::string stage_name,
      std::shared_ptr<LocalizationToPlannerMessenger> planner_messenger,
      std::shared_ptr<LocalizationToCollisionMessenger> collision_messenger,
      std::shared_ptr<LocalizationToTrafficLightMessenger> traffic_light_messenger,
      AtomicActorSet &registered_actors,
      InMemoryMap &local_map,
      Parameters &parameters,
      cc::DebugHelper &debug_helper,
      cc::World& world)
    : PipelineStage(stage_name),
      planner_messenger(planner_messenger),
      collision_messenger(collision_messenger),
      traffic_light_messenger(traffic_light_messenger),
      registered_actors(registered_actors),
      local_map(local_map),
      parameters(parameters),
      debug_helper(debug_helper),
      world(world) {

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

    ScanUnregisteredVehicles();

    // Selecting output frames based on selector keys.
    const auto current_planner_frame = planner_frame_selector ? planner_frame_a : planner_frame_b;
    const auto current_collision_frame = collision_frame_selector ? collision_frame_a : collision_frame_b;
    const auto current_traffic_light_frame =
        traffic_light_frame_selector ? traffic_light_frame_a : traffic_light_frame_b;

    // Looping over registered actors.
    for (uint64_t i = 0u; i < actor_list.size(); ++i) {

      const Actor vehicle = actor_list.at(i);
      const ActorId actor_id = vehicle->GetId();
      const cg::Location vehicle_location = vehicle->GetLocation();
      const float vehicle_velocity = vehicle->GetVelocity().Length();

      //TODO: Improve search so it doesn't do it every loop..
      auto search = idle_time.find(actor_id);
      if (search == idle_time.end()) {
        idle_time[actor_id] = chr::system_clock::now();
      }

      const float horizon_size = std::max(
          WAYPOINT_TIME_HORIZON * std::sqrt(vehicle_velocity * 10.0f),
          MINIMUM_HORIZON_LENGTH);

      Buffer &waypoint_buffer = buffer_list->at(i);

      // Purge passed waypoints.
      if (!waypoint_buffer.empty()) {
        float dot_product = DeviationDotProduct(vehicle, waypoint_buffer.front()->GetLocation(), true);

        while (dot_product <= 0.0f && !waypoint_buffer.empty()) {

          PopWaypoint(waypoint_buffer, actor_id);
          if (!waypoint_buffer.empty()) {
            dot_product = DeviationDotProduct(vehicle, waypoint_buffer.front()->GetLocation(), true);
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
      const SimpleWaypointPtr front_waypoint = waypoint_buffer.front();
      const ChangeLaneInfo lane_change_info = parameters.GetForceLaneChange(vehicle);
      const bool force_lane_change = lane_change_info.change_lane;
      const bool lane_change_direction = lane_change_info.direction;

      if ((parameters.GetAutoLaneChange(vehicle) || force_lane_change) &&
          !front_waypoint->CheckJunction()) {

        SimpleWaypointPtr change_over_point = AssignLaneChange(
            vehicle, force_lane_change, lane_change_direction);

        if (change_over_point != nullptr) {
          auto number_of_pops = waypoint_buffer.size();
          for (uint64_t j = 0u; j < number_of_pops; ++j) {
            PopWaypoint(waypoint_buffer, actor_id);
          }

          PushWaypoint(waypoint_buffer, actor_id, change_over_point);
        }
      }

      // Populating the buffer.
      while (waypoint_buffer.back()->DistanceSquared(waypoint_buffer.front())
          <= std::pow(horizon_size, 2)) {

        std::vector<SimpleWaypointPtr> next_waypoints = waypoint_buffer.back()->GetNextWaypoint();
        uint64_t selection_index = 0u;
        // Pseudo-randomized path selection if found more than one choice.
        if (next_waypoints.size() > 1) {
          selection_index = static_cast<uint>(rand()) % next_waypoints.size();
        }

        PushWaypoint(waypoint_buffer, actor_id, next_waypoints.at(selection_index));
      }

      // Generating output.
      const float target_point_distance = std::max(std::ceil(vehicle_velocity * TARGET_WAYPOINT_TIME_HORIZON),
          TARGET_WAYPOINT_HORIZON_LENGTH);
      SimpleWaypointPtr target_waypoint = waypoint_buffer.front();
      for (uint64_t j = 0u;
          (j < waypoint_buffer.size()) &&
          (waypoint_buffer.front()->DistanceSquared(target_waypoint)
          < std::pow(target_point_distance, 2));
          ++j) {
        target_waypoint = waypoint_buffer.at(j);
      }
      const cg::Location target_location = target_waypoint->GetLocation();
      float dot_product = DeviationDotProduct(vehicle, target_location);
      float cross_product = DeviationCrossProduct(vehicle, target_location);
      dot_product = 1.0f - dot_product;
      if (cross_product < 0.0f) {
        dot_product *= -1.0f;
      }

      float distance = 0.0f; // TODO: use in PID

      // Filtering out false junctions on highways.
      // On highways, if there is only one possible path and the section is
      // marked as intersection, ignore it.
      const auto vehicle_reference = boost::static_pointer_cast<cc::Vehicle>(vehicle);
      const float speed_limit = vehicle_reference->GetSpeedLimit();
      const float look_ahead_distance = std::max(2.0f * vehicle_velocity, MINIMUM_JUNCTION_LOOK_AHEAD);

      SimpleWaypointPtr look_ahead_point = waypoint_buffer.front();
      uint64_t look_ahead_index = 0u;
      for (uint64_t j = 0u;
          (waypoint_buffer.front()->DistanceSquared(look_ahead_point)
          < std::pow(look_ahead_distance, 2)) &&
          (j < waypoint_buffer.size());
          ++j) {
        look_ahead_point = waypoint_buffer.at(j);
        look_ahead_index = j;
      }

      bool approaching_junction = false;
      if (waypoint_buffer.front()->CheckJunction() || (look_ahead_point->CheckJunction() && !(waypoint_buffer.front()->CheckJunction()))) {
        if (speed_limit > HIGHWAY_SPEED) {
          for (uint64_t j = 0u; (j < look_ahead_index) && !approaching_junction; ++j) {
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
      planner_message.distance = distance;
      planner_message.approaching_true_junction = approaching_junction;

      LocalizationToCollisionData &collision_message = current_collision_frame->at(i);
      collision_message.actor = vehicle;
      collision_message.buffer = waypoint_buffer;

      collision_message.overlapping_actors.clear();
      ActorIdSet overlapping_actor_set = track_traffic.GetOverlappingVehicles(actor_id);
      for (ActorId overlapping_actor_id: overlapping_actor_set) {
        Actor actor_ptr = nullptr;
        if (vehicle_id_to_index.find(overlapping_actor_id) != vehicle_id_to_index.end()) {
          actor_ptr = actor_list.at(vehicle_id_to_index.at(overlapping_actor_id));
        } else if (unregistered_actors.find(overlapping_actor_id) != unregistered_actors.end()) {
          actor_ptr = unregistered_actors.at(overlapping_actor_id);
        }
        collision_message.overlapping_actors.insert({overlapping_actor_id, actor_ptr});
      }

      // auto grid_ids = track_traffic.GetGridIds(actor_id);
      // for (auto grid_id: grid_ids) {
      //   debug_helper.DrawLine(vehicle_location + cg::Location(0, 0, 2),
      //                         local_map.GetGeodesicGridCenter(grid_id),
      //                         0.5f, {0u, 255u, 0u}, 0.1f);
      // }

      LocalizationToTrafficLightData &traffic_light_message = current_traffic_light_frame->at(i);
      traffic_light_message.actor = vehicle;
      traffic_light_message.closest_waypoint = waypoint_buffer.front();
      traffic_light_message.junction_look_ahead_waypoint = waypoint_buffer.at(look_ahead_index);
    }

    // auto& grid_actor_map = track_traffic.GetGridActors();
    // for (auto& grid_actors: grid_actor_map) {
    //   if (grid_actors.second.size() > 0) {
    //     for (auto& grid_actor_id: grid_actors.second) {
    //       debug_helper.DrawLine(actor_list.at(vehicle_id_to_index.at(grid_actor_id))->GetLocation() + cg::Location(0, 0, 2),
    //                             local_map.GetGeodesicGridCenter(grid_actors.first),
    //                             0.5f, {0u, 0u, 255u}, 0.1f);
    //     }
    //   }
    // }

  }

  void LocalizationStage::DataReceiver() {

    // Building a list of registered actors and
    // connecting the vehicle ids to their position indices on data arrays.

    if (registered_actors_state != registered_actors.GetState()) {

      actor_list = registered_actors.GetList();

      uint64_t index = 0u;
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

    const DataPacket<std::shared_ptr<LocalizationToPlannerFrame>> planner_data_packet = {
      planner_messenger_state,
      planner_frame_selector ? planner_frame_a : planner_frame_b
    };
    planner_frame_selector = !planner_frame_selector;
    planner_messenger_state = planner_messenger->SendData(planner_data_packet);

    // Send data to collision stage only if it has finished
    // processing, received the previous message and started processing it.

    const DataPacket<std::shared_ptr<LocalizationToCollisionFrame>> collision_data_packet = {
        collision_messenger_state,
        collision_frame_selector ? collision_frame_a : collision_frame_b
      };

    collision_messenger_state = collision_messenger->SendData(collision_data_packet);
    collision_frame_selector = !collision_frame_selector;

    // Send data to traffic light stage only if it has finished
    // processing, received the previous message and started processing it.
    const int traffic_light_messenger_current_state = traffic_light_messenger->GetState();
    if (traffic_light_messenger_current_state != traffic_light_messenger_state) {
      const DataPacket<std::shared_ptr<LocalizationToTrafficLightFrame>> traffic_light_data_packet = {
          traffic_light_messenger_state,
          traffic_light_frame_selector ? traffic_light_frame_a : traffic_light_frame_b
        };

      traffic_light_messenger_state = traffic_light_messenger->SendData(traffic_light_data_packet);
      traffic_light_frame_selector = !traffic_light_frame_selector;
    }
  }

  void LocalizationStage::DrawBuffer(Buffer &buffer) {

    for (uint64_t i = 0u; i < buffer.size() && i < 5; ++i) {
      debug_helper.DrawPoint(buffer.at(i)->GetLocation(), 0.1f, {255u, 0u, 0u}, 0.5f);
    }
  }

  void LocalizationStage::PushWaypoint(Buffer& buffer, ActorId actor_id, SimpleWaypointPtr& waypoint) {

    const uint64_t waypoint_id = waypoint->GetId();
    buffer.push_back(waypoint);
    track_traffic.UpdatePassingVehicle(waypoint_id, actor_id);

    track_traffic.UpdateGridPosition(actor_id, waypoint);
  }

  void LocalizationStage::PopWaypoint(Buffer& buffer, ActorId actor_id) {

    SimpleWaypointPtr removed_waypoint = buffer.front();
    SimpleWaypointPtr remaining_waypoint = nullptr;
    const uint64_t removed_waypoint_id = removed_waypoint->GetId();
    buffer.pop_front();
    track_traffic.RemovePassingVehicle(removed_waypoint_id, actor_id);

    if (!buffer.empty()) {
      remaining_waypoint = buffer.front();
    }
    track_traffic.RemoveGridPosition(actor_id, removed_waypoint, remaining_waypoint);
  }

  void LocalizationStage::ScanUnregisteredVehicles() {
    ++unregistered_scan_duration;
    // Periodically check for actors not spawned by TrafficManager.
    if (unregistered_scan_duration == UNREGISTERED_ACTORS_SCAN_INTERVAL) {
      unregistered_scan_duration = 0;

      const auto world_actors = world.GetActors()->Filter("vehicle.*");
      const auto world_walker = world.GetActors()->Filter("walker.*");
      // Scanning for vehicles.
      for (auto actor: *world_actors.get()) {
        const auto unregistered_id = actor->GetId();
        if (vehicle_id_to_index.find(unregistered_id) == vehicle_id_to_index.end() &&
            unregistered_actors.find(unregistered_id) == unregistered_actors.end()) {
          unregistered_actors.insert({unregistered_id, actor});
        }
      }
      // Scanning for pedestrians.
      for (auto walker: *world_walker.get()) {
        const auto unregistered_id = walker->GetId();
        if (unregistered_actors.find(unregistered_id) == unregistered_actors.end()) {
          unregistered_actors.insert({unregistered_id, walker});
        }
      }
    }

    // Regularly update unregistered actors.
    std::vector<ActorId> actor_ids_to_erase;
    for (auto& actor_info: unregistered_actors) {
      if (actor_info.second->IsAlive()) {
        cg::Location actor_location = actor_info.second->GetLocation();
        SimpleWaypointPtr nearest_waypoint = local_map.GetWaypointInVicinity(actor_location);
        if (nearest_waypoint == nullptr) {
          nearest_waypoint = local_map.GetWaypoint(actor_location);
        }
        SimpleWaypointPtr previous_waypoint = nullptr;
        if (unregistered_waypoints.find(actor_info.first) != unregistered_waypoints.end()) {
          previous_waypoint = unregistered_waypoints.at(actor_info.first);
          unregistered_waypoints.at(actor_info.first) = nearest_waypoint;
        } else {
          unregistered_waypoints.insert({actor_info.first, nearest_waypoint});
        }
        track_traffic.UpdateGridPosition(actor_info.first, nearest_waypoint);
        track_traffic.RemoveGridPosition(actor_info.first, previous_waypoint, nearest_waypoint);
      } else {
        track_traffic.DeleteActor(actor_info.first);
        actor_ids_to_erase.push_back(actor_info.first);
      }
    }
    for (auto actor_id: actor_ids_to_erase) {
      unregistered_actors.erase(actor_id);
    }
  }

  SimpleWaypointPtr LocalizationStage::AssignLaneChange(Actor vehicle, bool force, bool direction) {

    const ActorId actor_id = vehicle->GetId();
    const cg::Location vehicle_location = vehicle->GetLocation();
    const float vehicle_velocity = vehicle->GetVelocity().Length();

    const Buffer& waypoint_buffer = buffer_list->at(vehicle_id_to_index.at(actor_id));
    const SimpleWaypointPtr& current_waypoint = waypoint_buffer.front();

    bool need_to_change_lane = false;
    const auto left_waypoint = current_waypoint->GetLeftWaypoint();
    const auto right_waypoint = current_waypoint->GetRightWaypoint();

    if (!force) {

      const auto blocking_vehicles = track_traffic.GetOverlappingVehicles(actor_id);

      bool abort_lane_change = false;
      for (auto i = blocking_vehicles.begin();
           i != blocking_vehicles.end() && !abort_lane_change;
           ++i) {

        const ActorId &other_vehicle_id = *i;

        // This is totally ignoring unregistered actors during lane changes.
        // Need to fix this. Only a temporary solution.
        if (vehicle_id_to_index.find(other_vehicle_id) != vehicle_id_to_index.end()) {

          const Buffer other_buffer = buffer_list->at(vehicle_id_to_index.at(other_vehicle_id));

          const SimpleWaypointPtr& other_current_waypoint = other_buffer.front();
          const cg::Location other_location = other_current_waypoint->GetLocation();

          bool distant_lane_availability = false;
          const auto other_neighbouring_lanes = {
              other_current_waypoint->GetLeftWaypoint(),
              other_current_waypoint->GetRightWaypoint()};

          for (auto& candidate_lane_wp: other_neighbouring_lanes) {
            if (candidate_lane_wp != nullptr &&
                track_traffic.GetPassingVehicles(candidate_lane_wp->GetId()).size() == 0 &&
                vehicle_velocity < HIGHWAY_SPEED) {
              distant_lane_availability = true;
            }
          }

          const cg::Vector3D reference_heading = current_waypoint->GetForwardVector();
          const cg::Vector3D other_heading = other_current_waypoint->GetForwardVector();

          if (other_vehicle_id != actor_id &&
              !current_waypoint->CheckJunction() &&
              !other_current_waypoint->CheckJunction() &&
              cg::Math::Dot(reference_heading, other_heading) > MAXIMUM_LANE_OBSTACLE_CURVATURE) {

            const float squared_vehicle_distance = cg::Math::DistanceSquared(other_location, vehicle_location);
            const float deviation_dot = DeviationDotProduct(vehicle, other_location);

            if (deviation_dot > 0.0f) {

              if (distant_lane_availability &&
                  squared_vehicle_distance > std::pow(MINIMUM_LANE_CHANGE_DISTANCE, 2)) {

                need_to_change_lane = true;
              } else if (squared_vehicle_distance < std::pow(MINIMUM_LANE_CHANGE_DISTANCE, 2)) {

                need_to_change_lane = false;
                abort_lane_change = true;
              }

            }
          }

        }
      }

    } else {

      need_to_change_lane = true;
    }

    const float change_over_distance = std::max(2.0f*vehicle_velocity, 10.0f);
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
        if (!force &&
            !possible_to_lane_change &&
            target_lane_wp != nullptr &&
            track_traffic.GetPassingVehicles(target_lane_wp->GetId()).size() == 0) {

          possible_to_lane_change = true;
          change_over_point = target_lane_wp;
        } else if (force) {

          possible_to_lane_change = true;
          change_over_point = target_lane_wp;
        }
      }
    }

    if (need_to_change_lane && possible_to_lane_change) {
      const auto starting_point = change_over_point;
      while (change_over_point->DistanceSquared(starting_point) < std::pow(change_over_distance, 2) &&
             !change_over_point->CheckJunction()) {
        change_over_point = change_over_point->GetNextWaypoint()[0];
      }
      return change_over_point;
    } else {
      return nullptr;
    }
  }

} // namespace traffic_manager
} // namespace carla
