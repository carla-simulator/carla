// Copyright (c) 2020 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "carla/trafficmanager/LocalizationStage.h"
#include "carla/client/DebugHelper.h"

namespace carla {
namespace traffic_manager {

namespace LocalizationConstants {

  static const float WAYPOINT_TIME_HORIZON = 5.0f;
  static const float MINIMUM_HORIZON_LENGTH = 30.0f;
  static const float TARGET_WAYPOINT_TIME_HORIZON = 1.0f;
  static const float TARGET_WAYPOINT_HORIZON_LENGTH = 5.0f;
  static const float MINIMUM_JUNCTION_LOOK_AHEAD = 10.0f;
  static const float HIGHWAY_SPEED = 50.0f / 3.6f;
  static const float MINIMUM_LANE_CHANGE_DISTANCE = 10.0f;
  static const float MAXIMUM_LANE_OBSTACLE_DISTANCE = 50.0f;
  static const float MAXIMUM_LANE_OBSTACLE_CURVATURE = 0.6f;
  static const uint64_t UNREGISTERED_ACTORS_SCAN_INTERVAL = 10;
  static const float BLOCKED_TIME_THRESHOLD = 90.0f;
  static const float DELTA_TIME_BETWEEN_DESTRUCTIONS = 10.0f;
  static const float STOPPED_VELOCITY_THRESHOLD = 0.8f;  // meters per second.
  static const float INTER_LANE_CHANGE_DISTANCE = 10.0f;

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
      carla::client::DebugHelper &debug_helper,
      carla::client::detail::EpisodeProxy &episodeProxy)
    : PipelineStage(stage_name),
      planner_messenger(planner_messenger),
      collision_messenger(collision_messenger),
      traffic_light_messenger(traffic_light_messenger),
      registered_actors(registered_actors),
      local_map(local_map),
      parameters(parameters),
      debug_helper(debug_helper),
      episode_proxy_ls(episodeProxy) {

    // Initializing various output frame selectors.
    planner_frame_selector = true;
    collision_frame_selector = true;
    traffic_light_frame_selector = true;
    // Initializing the number of vehicles to zero in the begining.
    number_of_vehicles = 0u;
    // Initializing the registered actors container state.
    registered_actors_state = -1;
    // Initializing buffer lists.
    buffer_list = std::make_shared<BufferList>();
    // Initializing maximum idle time to null.
    maximum_idle_time = std::make_pair(nullptr, 0.0);
    // Initializing srand.
    srand(static_cast<unsigned>(time(NULL)));

  }

  LocalizationStage::~LocalizationStage() {}

  void LocalizationStage::Action() {

    ScanUnregisteredVehicles();

    // Selecting output frames based on selector keys.
    const auto current_planner_frame = planner_frame_selector ? planner_frame_a : planner_frame_b;
    const auto current_collision_frame = collision_frame_selector ? collision_frame_a : collision_frame_b;
    const auto current_traffic_light_frame =
        traffic_light_frame_selector ? traffic_light_frame_a : traffic_light_frame_b;

    // Selecting current timestamp from the world snapshot.
    current_timestamp = episode_proxy_ls.Lock()->GetWorldSnapshot().GetTimestamp();

    // Looping over registered actors.
    for (uint64_t i = 0u; i < actor_list.size(); ++i) {

      const Actor vehicle = actor_list.at(i);
      const ActorId actor_id = vehicle->GetId();
      const cg::Location vehicle_location = vehicle->GetLocation();
      const float vehicle_velocity = vehicle->GetVelocity().Length();

      // Initializing idle times.
      if (idle_time.find(actor_id) == idle_time.end() && current_timestamp.elapsed_seconds != 0) {
        idle_time[actor_id] = current_timestamp.elapsed_seconds;
      }

      const float horizon_size = std::max(
          WAYPOINT_TIME_HORIZON * std::sqrt(vehicle_velocity * 10.0f),
          MINIMUM_HORIZON_LENGTH);

      if (buffer_list->find(actor_id) == buffer_list->end()) {
        buffer_list->insert({actor_id, Buffer()});
      }

      Buffer &waypoint_buffer = buffer_list->at(actor_id);

      // Clear buffer if vehicle is too far from the first waypoint in the buffer.
      if (!waypoint_buffer.empty() &&
          cg::Math::DistanceSquared(waypoint_buffer.front()->GetLocation(), vehicle_location) > std::pow(30.0f, 2)) {

        auto number_of_pops = waypoint_buffer.size();
        for (uint64_t j = 0u; j < number_of_pops; ++j) {
          PopWaypoint(waypoint_buffer, actor_id);
        }
      }

      // Purge passed waypoints.
      if (!waypoint_buffer.empty()) {
        float dot_product = DeviationDotProduct(vehicle, vehicle_location, waypoint_buffer.front()->GetLocation(), true);

        while (dot_product <= 0.0f && !waypoint_buffer.empty()) {

          PopWaypoint(waypoint_buffer, actor_id);
          if (!waypoint_buffer.empty()) {
            dot_product = DeviationDotProduct(vehicle, vehicle_location, waypoint_buffer.front()->GetLocation(), true);
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

      const ChangeLaneInfo lane_change_info = parameters.GetForceLaneChange(vehicle);
      bool force_lane_change = lane_change_info.change_lane;
      bool lane_change_direction = lane_change_info.direction;

      if (!force_lane_change) {
        float perc_keep_right = parameters.GetKeepRightPercentage(vehicle);
        if (perc_keep_right >= 0.0f && perc_keep_right >= (rand() % 101)) {
            force_lane_change = true;
            lane_change_direction = true;
        }
      }

      const SimpleWaypointPtr front_waypoint = waypoint_buffer.front();
      const double lane_change_distance = std::pow(std::max(10.0f * vehicle_velocity, INTER_LANE_CHANGE_DISTANCE), 2);

      if (((parameters.GetAutoLaneChange(vehicle) || force_lane_change) && !front_waypoint->CheckJunction())
          && (last_lane_change_location.find(actor_id) == last_lane_change_location.end()
              || cg::Math::DistanceSquared(last_lane_change_location.at(actor_id), vehicle_location)
                 > lane_change_distance )) {

        SimpleWaypointPtr change_over_point = AssignLaneChange(
            vehicle, vehicle_location, force_lane_change, lane_change_direction);

        if (change_over_point != nullptr) {
          if (last_lane_change_location.find(actor_id) != last_lane_change_location.end()) {
            last_lane_change_location.at(actor_id) = vehicle_location;
          } else {
            last_lane_change_location.insert({actor_id, vehicle_location});
          }
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
          selection_index = static_cast<uint64_t>(rand()) % next_waypoints.size();
        }
        SimpleWaypointPtr next_wp = next_waypoints.at(selection_index);
        if (next_wp == nullptr) {
          for (auto& wp: next_waypoints) {
            if (wp != nullptr) {
              next_wp = wp;
              break;
            }
          }
        }
        PushWaypoint(waypoint_buffer, actor_id, next_wp);
      }

      // Updating geodesic grid position for actor.
      track_traffic.UpdateGridPosition(actor_id, waypoint_buffer);

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
      float dot_product = DeviationDotProduct(vehicle, vehicle_location, target_location);
      float cross_product = DeviationCrossProduct(vehicle, vehicle_location, target_location);
      dot_product = 1.0f - dot_product;
      if (cross_product < 0.0f) {
        dot_product *= -1.0f;
      }

      float distance = 0.0f; // TODO: use in PID

      // Filtering out false junctions on highways:
      // on highways, if there is only one possible path and the section is
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
      if (look_ahead_point->CheckJunction() && !(waypoint_buffer.front()->CheckJunction())) {
        if (speed_limit*3.6f > HIGHWAY_SPEED) {
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

      // Reset the variables when no longer approaching a junction.
      if (!approaching_junction && approached[actor_id]){
        final_safe_points[actor_id] = nullptr;
        approached[actor_id] = false;
      }

      // Only do once, when the junction has just been seen.
      else if (approaching_junction && !approached[actor_id]){

        SimpleWaypointPtr final_point = nullptr;
        final_point = GetSafeLocationAfterJunction(vehicle_reference, waypoint_buffer);
        if(final_point != nullptr){
          final_safe_points[actor_id] = final_point;
          approaching_junction = false;
          approached[actor_id] = true;
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
        if (actor_ptr!=nullptr) {
          collision_message.overlapping_actors.insert({overlapping_actor_id, actor_ptr});
        }
        collision_message.safe_point_after_junction = final_safe_points[actor_id];
      }
      collision_message.closest_waypoint = waypoint_buffer.front();
      collision_message.junction_look_ahead_waypoint = waypoint_buffer.at(look_ahead_index);

      LocalizationToTrafficLightData &traffic_light_message = current_traffic_light_frame->at(i);
      traffic_light_message.actor = vehicle;
      traffic_light_message.closest_waypoint = waypoint_buffer.front();
      traffic_light_message.junction_look_ahead_waypoint = waypoint_buffer.at(look_ahead_index);

      // Updating idle time when necessary.
      UpdateIdleTime(vehicle);
    }

    if (IsVehicleStuck(maximum_idle_time.first)) {
      TryDestroyVehicle(maximum_idle_time.first);
    }

    // Updating maximum idle time to null for the next iteration.
    maximum_idle_time = std::make_pair(nullptr, current_timestamp.elapsed_seconds);
  }

  void LocalizationStage::DataReceiver() {
    bool is_deleted_actors_present = false;
    std::set<uint32_t> world_actor_id;
    std::vector<ActorPtr> actor_list_to_be_deleted;

    // Filter function to collect the data.
    auto Filter = [&](auto &actors, auto &wildcard_pattern) {
      std::vector<carla::client::detail::ActorVariant> filtered;
      for (auto &&actor : actors) {
        if (carla::StringUtil::Match(carla::client::detail::ActorVariant(actor).GetTypeId(), wildcard_pattern)) {
          filtered.push_back(actor);
        }
      }
      return filtered;
    };

    // Get all the actors.
    auto world_actors_list = episode_proxy_ls.Lock()->GetAllTheActorsInTheEpisode();

    // Filter with vehicle wildcard.
    auto vehicles = Filter(world_actors_list, "vehicle.*");

    // Building a set of vehicle ids in the world.
    for (const auto &actor : vehicles) {
      world_actor_id.insert(actor.GetId());
    }

    // Search for invalid/destroyed vehicles.
    for (auto &actor : actor_list) {
      if (world_actor_id.find(actor->GetId()) == world_actor_id.end()) {
        actor_list_to_be_deleted.emplace_back(actor);
        track_traffic.DeleteActor(actor->GetId());
        last_lane_change_location.erase(actor->GetId());
      }
    }

    // Clearing the registered actor list.
    if(!actor_list_to_be_deleted.empty()) {
      registered_actors.Remove(actor_list_to_be_deleted);
      actor_list.clear();
      actor_list = registered_actors.GetList();
      is_deleted_actors_present = true;
    }

    // Building a list of registered actors and connecting
    // the vehicle ids to their position indices on data arrays.

    if (is_deleted_actors_present || (registered_actors_state != registered_actors.GetState())) {
      actor_list.clear();
      actor_list_to_be_deleted.clear();
      actor_list = registered_actors.GetList();
      uint64_t index = 0u;
      vehicle_id_to_index.clear();
      for (auto &actor : actor_list) {
        vehicle_id_to_index.insert({actor->GetId(), index});
        ++index;
      }
      registered_actors_state = registered_actors.GetState();
    }

    // Allocating new containers for the changed number of registered vehicles.
    if (number_of_vehicles != actor_list.size()) {
      number_of_vehicles = static_cast<uint64_t>(actor_list.size());
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

    planner_messenger->Push(planner_frame_selector ? planner_frame_a : planner_frame_b);
    planner_frame_selector = !planner_frame_selector;

    const auto current_collision_frame = collision_frame_selector ? collision_frame_a : collision_frame_b;
    collision_messenger->Push(current_collision_frame);
    collision_frame_selector = !collision_frame_selector;

    traffic_light_messenger->Push(traffic_light_frame_selector ? traffic_light_frame_a : traffic_light_frame_b);
    traffic_light_frame_selector = !traffic_light_frame_selector;
  }

  void LocalizationStage::DrawBuffer(Buffer &buffer) {

    for (uint64_t i = 0u; i < buffer.size(); ++i) {
      if(buffer.at(i)->GetWaypoint()->IsJunction()){
        debug_helper.DrawPoint(buffer.at(i)->GetLocation() + cg::Location(0.0f,0.0f,2.0f), 0.3f, {0u, 0u, 255u}, 0.05f);
      } else {
        debug_helper.DrawPoint(buffer.at(i)->GetLocation() + cg::Location(0.0f,0.0f,2.0f), 0.3f, {0u, 255u, 255u}, 0.05f);
      }
    }
  }

  void LocalizationStage::PushWaypoint(Buffer& buffer, ActorId actor_id, SimpleWaypointPtr& waypoint) {

    const uint64_t waypoint_id = waypoint->GetId();
    buffer.push_back(waypoint);
    track_traffic.UpdatePassingVehicle(waypoint_id, actor_id);
  }

  void LocalizationStage::PopWaypoint(Buffer& buffer, ActorId actor_id) {

    SimpleWaypointPtr removed_waypoint = buffer.front();
    SimpleWaypointPtr remaining_waypoint = nullptr;
    const uint64_t removed_waypoint_id = removed_waypoint->GetId();
    buffer.pop_front();
    track_traffic.RemovePassingVehicle(removed_waypoint_id, actor_id);
  }

  void LocalizationStage::ScanUnregisteredVehicles() {
    ++unregistered_scan_duration;
  // Periodically check for actors not spawned by TrafficManager.
  if (unregistered_scan_duration == UNREGISTERED_ACTORS_SCAN_INTERVAL) {
    unregistered_scan_duration = 0;

    auto Filter = [&](auto &actors, auto &wildcard_pattern) {
      std::vector<carla::client::detail::ActorVariant> filtered;
      for (auto &&actor : actors) {
        if (carla::StringUtil::Match
             ( carla::client::detail::ActorVariant(actor).GetTypeId()
             , wildcard_pattern)) {
          filtered.push_back(actor);
        }
      }
      return filtered;
    };

    /// Get all actors of the world
    auto world_actors_list = episode_proxy_ls.Lock()->GetAllTheActorsInTheEpisode();

    /// Filter based on wildcard_pattern
    const auto world_actors = Filter(world_actors_list, "vehicle.*");
    const auto world_walker = Filter(world_actors_list, "walker.*");

    // Scanning for vehicles.
    for (auto actor: world_actors) {
      const auto unregistered_id = actor.GetId();
      if (vehicle_id_to_index.find(unregistered_id) == vehicle_id_to_index.end() &&
          unregistered_actors.find(unregistered_id) == unregistered_actors.end()) {
        unregistered_actors.insert({unregistered_id, actor.Get(episode_proxy_ls)});
      }
    }
    // Scanning for pedestrians.
    for (auto walker: world_walker) {
      const auto unregistered_id = walker.GetId();
      if (unregistered_actors.find(unregistered_id) == unregistered_actors.end()) {
        unregistered_actors.insert({unregistered_id, walker.Get(episode_proxy_ls)});
      }
    }
  }

    // Regularly update unregistered actors.
    const auto current_snapshot = episode_proxy_ls.Lock()->GetWorldSnapshot();
    for (auto it = unregistered_actors.cbegin(); it != unregistered_actors.cend();) {
      if (registered_actors.Contains(it->first) || !current_snapshot.Contains(it->first)) {
        track_traffic.DeleteActor(it->first);
        it = unregistered_actors.erase(it);
      } else {
        // Updating data structures.
        cg::Location location = it->second->GetLocation();
        const auto type = it->second->GetTypeId();

        SimpleWaypointPtr nearest_waypoint = nullptr;
        if (type[0] == 'v') {
          nearest_waypoint = local_map.GetWaypointInVicinity(location);
        } else if (type[0] == 'w') {
          nearest_waypoint = local_map.GetPedWaypoint(location);
        }
        if (nearest_waypoint == nullptr) {
          nearest_waypoint = local_map.GetWaypoint(location);
        }

        track_traffic.UpdateUnregisteredGridPosition(it->first, nearest_waypoint);

        ++it;
      }
    }
  }

  SimpleWaypointPtr LocalizationStage::AssignLaneChange(Actor vehicle, const cg::Location &vehicle_location,
                                                        bool force, bool direction)
  {

    const ActorId actor_id = vehicle->GetId();
    const float vehicle_velocity = vehicle->GetVelocity().Length();

    // Waypoint representing the new starting point for the waypoint buffer
    // due to lane change. Remains nullptr if lane change not viable.
    SimpleWaypointPtr change_over_point = nullptr;

    // Retrieve waypoint buffer for current vehicle.
    const Buffer& waypoint_buffer = buffer_list->at(actor_id);

    // Check buffer is not empty.
    if (!waypoint_buffer.empty())
    {
      // Get the left and right waypoints for the current closest waypoint.
      const SimpleWaypointPtr& current_waypoint = waypoint_buffer.front();
      const SimpleWaypointPtr left_waypoint = current_waypoint->GetLeftWaypoint();
      const SimpleWaypointPtr right_waypoint = current_waypoint->GetRightWaypoint();

      // Retrieve vehicles with overlapping waypoint buffers with current vehicle.
      const auto blocking_vehicles = track_traffic.GetOverlappingVehicles(actor_id);

      // Find immediate in-lane obstacle and check if any are too close to initiate lane change.
      bool obstacle_too_close = false;
      float minimum_squared_distance = std::numeric_limits<float>::infinity();
      ActorId obstacle_actor_id = 0u;
      for (auto i = blocking_vehicles.begin();
           i != blocking_vehicles.end() && !obstacle_too_close && !force;
           ++i)
      {
        const ActorId &other_actor_id = *i;
        // Find vehicle in registered list and check if it's buffer is not empty.
        if (vehicle_id_to_index.find(other_actor_id) != vehicle_id_to_index.end()
            && buffer_list->find(other_actor_id) != buffer_list->end()
            && !buffer_list->at(other_actor_id).empty())
        {
          const Buffer& other_buffer = buffer_list->at(other_actor_id);
          const SimpleWaypointPtr& other_current_waypoint = other_buffer.front();
          const cg::Location other_location = other_current_waypoint->GetLocation();

          const cg::Vector3D reference_heading = current_waypoint->GetForwardVector();
          cg::Vector3D reference_to_other = other_current_waypoint->GetLocation()
                                            - current_waypoint->GetLocation();
          const cg::Vector3D other_heading = other_current_waypoint->GetForwardVector();

          // Check both vehicles are not in junction,
          // Check if the other vehicle is in front of the current vehicle,
          // Check if the two vehicles have acceptable angular deviation between their headings.
          if (!current_waypoint->CheckJunction()
              && !other_current_waypoint->CheckJunction()
              && other_current_waypoint->GetWaypoint()->GetRoadId() == current_waypoint->GetWaypoint()->GetRoadId()
              && other_current_waypoint->GetWaypoint()->GetLaneId() == current_waypoint->GetWaypoint()->GetLaneId()
              && cg::Math::Dot(reference_heading, reference_to_other) > 0.0f
              && cg::Math::Dot(reference_heading, other_heading) > MAXIMUM_LANE_OBSTACLE_CURVATURE)
          {
            float squared_distance = cg::Math::DistanceSquared(vehicle_location, other_location);
            // Abort if the obstacle is too close.
            if (squared_distance > std::pow(MINIMUM_LANE_CHANGE_DISTANCE, 2))
            {
              // Remember if the new vehicle is closer.
              if (squared_distance < minimum_squared_distance
                  && squared_distance < std::pow(MAXIMUM_LANE_OBSTACLE_DISTANCE, 2.0f))
              {
                minimum_squared_distance = squared_distance;
                obstacle_actor_id = other_actor_id;
              }
            } else {
              obstacle_too_close = true;
            }
          }
        }
      }

      // If a valid immediate obstacle found.
      if (!obstacle_too_close && obstacle_actor_id != 0u && !force)
      {
        const Buffer& other_buffer = buffer_list->at(obstacle_actor_id);
        const SimpleWaypointPtr& other_current_waypoint = other_buffer.front();
        const auto other_neighbouring_lanes = {other_current_waypoint->GetLeftWaypoint(),
                                               other_current_waypoint->GetRightWaypoint()};

        // Flags reflecting whether adjacent lanes are free near the obstacle.
        bool distant_left_lane_free = false;
        bool distant_right_lane_free = false;

        // Check if the neighbouring lanes near the obstructing vehicle are free of other vehicles.
        bool left_right = true;
        for (auto& candidate_lane_wp: other_neighbouring_lanes) {
          if (candidate_lane_wp != nullptr &&
              track_traffic.GetPassingVehicles(candidate_lane_wp->GetId()).size() == 0) {

            if (left_right) distant_left_lane_free = true;
            else distant_right_lane_free = true;

          }
          left_right = !left_right;
        }

        // Based on what lanes are free near the obstacle,
        // find the change over point with no vehicles passing through them.
        if (distant_right_lane_free && right_waypoint != nullptr
            && track_traffic.GetPassingVehicles(right_waypoint->GetId()).size() == 0)
        {
          change_over_point = right_waypoint;
        } else if (distant_left_lane_free && left_waypoint != nullptr
                   && track_traffic.GetPassingVehicles(left_waypoint->GetId()).size() == 0)
        {
          change_over_point = left_waypoint;
        }
      } else if (force) {
        if (direction && right_waypoint != nullptr) {
          change_over_point = right_waypoint;
        } else if (!direction && left_waypoint != nullptr) {
          change_over_point = left_waypoint;
        }
      }

      if (change_over_point != nullptr)
      {
        const float change_over_distance =  cg::Math::Clamp(1.5f*vehicle_velocity, 3.0f, 20.0f);
        const auto starting_point = change_over_point;
        while (change_over_point->DistanceSquared(starting_point) < std::pow(change_over_distance, 2) &&
              !change_over_point->CheckJunction()) {
          change_over_point = change_over_point->GetNextWaypoint()[0];
        }

        // Reset this variable if needed
        if (approached[actor_id]){
          approached[actor_id] = false;
        }
      }
    }

    return change_over_point;
  }

  SimpleWaypointPtr LocalizationStage::GetSafeLocationAfterJunction(const Vehicle &vehicle, Buffer &waypoint_buffer){

    ActorId actor_id = vehicle->GetId();
    // Get the length of the car
    float length = vehicle->GetBoundingBox().extent.x;
    // First Waypoint before the junction
    const SimpleWaypointPtr initial_point;
    uint64_t initial_index = 0;
    // First Waypoint after the junction
    SimpleWaypointPtr safe_point = nullptr;
    uint64_t safe_index = 0;
    // Vehicle position after the junction
    SimpleWaypointPtr final_point = nullptr;
    // Safe space after the junction
    const float safe_distance = 1.5f*length;

    for (uint64_t j = 0u; j < waypoint_buffer.size(); ++j){
      if (waypoint_buffer.at(j)->CheckJunction()){
        initial_index = j;
        break;
      }
    }

    // Stop if something failed
    if (initial_index == 0 && !waypoint_buffer.front()->CheckJunction()){
      return final_point;
    }

    // 2) Search for the end of the intersection (if it is in the buffer)
    for (uint64_t i = initial_index; i < waypoint_buffer.size(); ++i){

      if (!waypoint_buffer.at(i)->CheckJunction()){
        safe_point = waypoint_buffer.at(i);
        safe_index = i;
        break;
      }
    }

    // If it hasn't been found, extend the buffer
    if(safe_point == nullptr){
      while (waypoint_buffer.back()->CheckJunction()) {

          std::vector<SimpleWaypointPtr> next_waypoints = waypoint_buffer.back()->GetNextWaypoint();
          uint64_t selection_index = 0u;
          if (next_waypoints.size() > 1) {
            selection_index = static_cast<uint64_t>(rand()) % next_waypoints.size();
          }

          PushWaypoint(waypoint_buffer, actor_id, next_waypoints.at(selection_index));
        }
      // Save the last one
      safe_point = waypoint_buffer.back();
    }

    // Stop if something failed
    if (safe_index == 0){
      return final_point;
    }

    // 3) Search for final_point (again, if it is in the buffer)

    for(uint64_t k = safe_index; k < waypoint_buffer.size(); ++k){

      if(safe_point->Distance(waypoint_buffer.at(k)->GetLocation()) > safe_distance){
        final_point = waypoint_buffer.at(k);
        break;
      }
    }

    // If it hasn't been found, extend the buffer
    if(final_point == nullptr){
      while (safe_point->Distance(waypoint_buffer.back()->GetLocation()) < safe_distance) {

        // Record the last point as a safe one and save it
        std::vector<SimpleWaypointPtr> next_waypoints = waypoint_buffer.back()->GetNextWaypoint();
        uint64_t selection_index = 0u;
        // Pseudo-randomized path selection if found more than one choice.
        if (next_waypoints.size() > 1) {
          selection_index = static_cast<uint64_t>(rand()) % next_waypoints.size();
        }

        PushWaypoint(waypoint_buffer, actor_id, next_waypoints.at(selection_index));
      }
      final_point = waypoint_buffer.back();
    }

    return final_point;
  }

  void LocalizationStage::UpdateIdleTime(const Actor& actor) {
    if (idle_time.find(actor->GetId()) == idle_time.end()) {
      return;
    }

    const auto vehicle = boost::static_pointer_cast<cc::Vehicle>(actor);
    if (actor->GetVelocity().Length() > STOPPED_VELOCITY_THRESHOLD || (vehicle->IsAtTrafficLight() && vehicle->GetTrafficLightState() != TLS::Green)) {
      idle_time[actor->GetId()] = current_timestamp.elapsed_seconds;
    }

    // Checking maximum idle time.
    if (maximum_idle_time.first == nullptr || maximum_idle_time.second > idle_time[actor->GetId()]) {
      maximum_idle_time = std::make_pair(actor, idle_time[actor->GetId()]);
    }
  }

  bool LocalizationStage::IsVehicleStuck(const Actor& actor) {
    if (actor == nullptr) {
      return false;
    }

    if (idle_time.find(actor->GetId()) != idle_time.end()) {
      auto delta_idle_time = current_timestamp.elapsed_seconds - idle_time.at(actor->GetId());
      if (delta_idle_time >= BLOCKED_TIME_THRESHOLD) {
        return true;
      }
    }
    return false;
  }

  void LocalizationStage::CleanActor(const ActorId actor_id) {
    track_traffic.DeleteActor(actor_id);
    for (const auto& waypoint : buffer_list->at(actor_id)) {
      track_traffic.RemovePassingVehicle(waypoint->GetId(), actor_id);
    }

    idle_time.erase(actor_id);
    buffer_list->erase(actor_id);
  }

  bool LocalizationStage::TryDestroyVehicle(const Actor& actor) {
    if (!actor->IsAlive()) {
      return false;
    }

    const ActorId actor_id = actor->GetId();

    auto delta_last_actor_destruction = current_timestamp.elapsed_seconds - elapsed_last_actor_destruction;
    if (delta_last_actor_destruction >= DELTA_TIME_BETWEEN_DESTRUCTIONS) {
      registered_actors.Destroy(actor);

      // Clean actor data structures.
      CleanActor(actor_id);

      elapsed_last_actor_destruction = current_timestamp.elapsed_seconds;

      return true;
    }
    return false;
  }

} // namespace traffic_manager
} // namespace carla
