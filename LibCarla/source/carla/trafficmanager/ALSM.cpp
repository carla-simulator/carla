
#include "carla/trafficmanager/ALSM.h"

namespace carla {
namespace traffic_manager {

ALSM::ALSM(
  AtomicActorSet &registered_vehicles,
  BufferMapPtr &buffer_map_ptr,
  TrackTraffic &track_traffic,
  const Parameters &parameters,
  const cc::World &world,
  const LocalMapPtr &local_map,
  SimulationState &simulation_state,
  LocalizationStage &localization_stage,
  CollisionStage &collision_stage,
  TrafficLightStage &traffic_light_stage,
  MotionPlanStage &motion_plan_stage)
  : registered_vehicles(registered_vehicles),
    buffer_map_ptr(buffer_map_ptr),
    track_traffic(track_traffic),
    parameters(parameters),
    world(world),
    local_map(local_map),
    simulation_state(simulation_state),
    localization_stage(localization_stage),
    collision_stage(collision_stage),
    traffic_light_stage(traffic_light_stage),
    motion_plan_stage(motion_plan_stage) {}

void ALSM::Update() {
  bool hybrid_physics_mode = parameters.GetHybridPhysicsMode();

  std::set<ActorId> world_vehicle_ids;
  std::set<ActorId> world_pedestrian_ids;
  std::vector<ActorId> unregistered_list_to_be_deleted;

  current_timestamp = world.GetSnapshot().GetTimestamp();
  ActorList world_actors = world.GetActors();
  ActorList world_vehicles = world_actors->Filter("vehicle.*");
  ActorList world_pedestrians = world_actors->Filter("walker.*");

  // Scanning for new unregistered vehicles.
  for (auto iter = world_vehicles->begin(); iter != world_vehicles->end(); ++iter) {
    // Building set containing current world vehicle ids.
    const auto unregistered_id = (*iter)->GetId();
    world_vehicle_ids.insert(unregistered_id);
    if (!registered_vehicles.Contains(unregistered_id)
        && unregistered_actors.find(unregistered_id) == unregistered_actors.end()) {
      unregistered_actors.insert({unregistered_id, *iter});
    }
  }

  // Scanning for new pedestrians.
  for (auto iter = world_pedestrians->begin(); iter != world_pedestrians->end(); ++iter) {
    // Building set containing current world pedestrian ids.
    const auto unregistered_id = (*iter)->GetId();
    world_pedestrian_ids.insert(unregistered_id);
    if (unregistered_actors.find(unregistered_id) == unregistered_actors.end()) {
      unregistered_actors.insert({unregistered_id, *iter});
    }
  }

  // Identify hero vehicle if currently not present
  // and system is in hybrid physics mode.
  if (hybrid_physics_mode) {
    for (auto iter = unregistered_actors.begin(); iter != unregistered_actors.end(); ++iter) {
      ActorPtr actor_ptr = iter->second;
      if (actor_ptr->GetTypeId().front() == 'v') {
        ActorId hero_actor_id = actor_ptr->GetId();
        if (hero_actors.find(hero_actor_id) == hero_actors.end()) {
          for (auto&& attribute: actor_ptr->GetAttributes()) {
            if (attribute.GetId() == "role_name" && attribute.GetValue() == "hero") {
              hero_actors.insert({hero_actor_id, actor_ptr});
            }
          }
        }
      }
    }
  }

  // Invalidate hero actor if it is not alive anymore.
  ActorIdSet hero_actors_to_delete;
  if (hybrid_physics_mode && hero_actors.size() != 0u) {
    for (auto &hero_actor_info: hero_actors) {
      if(world_vehicle_ids.find(hero_actor_info.first) == world_vehicle_ids.end()) {
        hero_actors_to_delete.insert(hero_actor_info.first);
      }
    }
  }
  for (auto &deletion_id: hero_actors_to_delete) {
    hero_actors.erase(deletion_id);
  }

  bool vehicles_unregistered = false;
  // Search for invalid/destroyed registered vehicles.
  std::vector<ActorId> vehicle_id_list = registered_vehicles.GetIDList();
  for (const auto &deletion_id : vehicle_id_list) {
    if (world_vehicle_ids.find(deletion_id) == world_vehicle_ids.end()) {
      RemoveActor(deletion_id, true);
      if (!vehicles_unregistered) {
        vehicles_unregistered = true;
      }
    }
  }

  // Regularly update unregistered actor grid position and identify any invalid actors.
  for (auto iter = unregistered_actors.begin(); iter != unregistered_actors.cend(); ++iter) {
    ActorId unregistered_actor_id = iter->first;
    if (registered_vehicles.Contains(unregistered_actor_id)
        || (world_vehicle_ids.find(unregistered_actor_id) == world_vehicle_ids.end()
            && world_pedestrian_ids.find(unregistered_actor_id) == world_pedestrian_ids.end())) {
      unregistered_list_to_be_deleted.push_back(iter->first);
    }
    else {
      // Updating data structures.
      cg::Location location = iter->second->GetLocation();
      const auto type = iter->second->GetTypeId();

      std::vector<SimpleWaypointPtr> nearest_waypoints;
      if (type[0] == 'v') {
        auto vehicle_ptr = boost::static_pointer_cast<cc::Vehicle>(iter->second);
        cg::Vector3D extent = vehicle_ptr->GetBoundingBox().extent;
        cg::Vector3D heading_vector = vehicle_ptr->GetTransform().GetForwardVector();
        std::vector<cg::Location> corners = {location + cg::Location(extent.x * heading_vector),
                                             location,
                                             location + cg::Location(-extent.x * heading_vector)};
        for (cg::Location &vertex: corners) {
          SimpleWaypointPtr nearest_waypoint = local_map->GetWaypointInVicinity(vertex);
          if (nearest_waypoint == nullptr) {nearest_waypoint = local_map->GetPedWaypoint(vertex);};
          if (nearest_waypoint == nullptr) {nearest_waypoint = local_map->GetWaypoint(location);};
          nearest_waypoints.push_back(nearest_waypoint);
        }
      } else if (type[0] == 'w') {
        SimpleWaypointPtr nearest_waypoint = local_map->GetPedWaypoint(location);
        if (nearest_waypoint == nullptr) {nearest_waypoint = local_map->GetWaypoint(location);};
        nearest_waypoints.push_back(nearest_waypoint);
      }

      track_traffic.UpdateUnregisteredGridPosition(iter->first, nearest_waypoints);
    }
  }

  // Removing invalid/destroyed unregistered actors.
  for (auto deletion_id : unregistered_list_to_be_deleted) {
    RemoveActor(deletion_id, false);
  }

  // Update dynamic state and static attributes for all registered vehicles.
  float dt = HYBRID_MODE_DT;
  std::pair<ActorId, double> max_idle_time = std::make_pair(0u, current_timestamp.elapsed_seconds);
  std::vector<ActorPtr> vehicle_list = registered_vehicles.GetList();
  for (const Actor &vehicle : vehicle_list) {
    ActorId actor_id = vehicle->GetId();
    cg::Transform vehicle_transform = vehicle->GetTransform();
    cg::Location vehicle_location = vehicle_transform.location;
    cg::Rotation vehicle_rotation = vehicle_transform.rotation;
    cg::Vector3D vehicle_velocity = vehicle->GetVelocity();

    // Initializing idle times.
    if (idle_time.find(actor_id) == idle_time.end() && current_timestamp.elapsed_seconds != 0) {
      idle_time.insert({actor_id, current_timestamp.elapsed_seconds});
    }

    // Check if current actor is in range of hero actor and enable physics in hybrid mode.
    float hybrid_physics_radius = parameters.GetHybridPhysicsRadius();
    bool in_range_of_hero_actor = false;
    if (hybrid_physics_mode && hero_actors.size() != 0u) {
        for (auto &hero_actor_info: hero_actors) {
        if (simulation_state.ContainsActor(hero_actor_info.first)) {
          const cg::Location &hero_location = simulation_state.GetLocation(hero_actor_info.first);
          if (cg::Math::DistanceSquared(vehicle_location, hero_location) < std::pow(hybrid_physics_radius, 2)) {
            in_range_of_hero_actor = true;
            break;
          }
        }
      }
    }
    bool enable_physics = hybrid_physics_mode ? in_range_of_hero_actor : true;
    vehicle->SetSimulatePhysics(enable_physics);

    // If physics is disabled, calculate velocity based on change in position.
    if (!enable_physics) {
      cg::Location previous_location;
      if (simulation_state.ContainsActor(actor_id)) {
        previous_location = simulation_state.GetLocation(actor_id);
      } else {
        previous_location = vehicle_location;
      }
      cg::Vector3D displacement = (vehicle_location - previous_location);
      vehicle_velocity = displacement / dt;
    }

    // Updated kinematic state object.
    auto vehicle_ptr = boost::static_pointer_cast<cc::Vehicle>(vehicle);
    KinematicState kinematic_state{enable_physics, vehicle_location, vehicle_rotation,
                                   vehicle_velocity, vehicle_ptr->GetSpeedLimit()};

    // Updated traffic light state object.
    TrafficLightState tl_state = {vehicle_ptr->GetTrafficLightState(), vehicle_ptr->IsAtTrafficLight()};

    // Update simulation state.
    if (simulation_state.ContainsActor(actor_id)) {
      simulation_state.UpdateKinematicState(actor_id, kinematic_state);
      simulation_state.UpdateTrafficLightState(actor_id, tl_state);
    }
    else {
      cg::Vector3D dimensions = vehicle_ptr->GetBoundingBox().extent;
      StaticAttributes attributes{ActorType::Vehicle, dimensions.x, dimensions.y, dimensions.z};

      simulation_state.AddActor(actor_id, kinematic_state, attributes, tl_state);
    }

    // Updating idle time when necessary.
    UpdateIdleTime(max_idle_time, actor_id);
  }

  // Destroy registered vehicle if stuck at a location for too long.
  if (IsVehicleStuck(max_idle_time.first)
      && (current_timestamp.elapsed_seconds - elapsed_last_actor_destruction) > DELTA_TIME_BETWEEN_DESTRUCTIONS) {
    registered_vehicles.Destroy(max_idle_time.first);
    RemoveActor(max_idle_time.first, true);
    elapsed_last_actor_destruction = current_timestamp.elapsed_seconds;
  }

  // Update kinematic state and static attributes for unregistered actors.
  for (auto &unregistered_actor: unregistered_actors) {
    const ActorId actor_id = unregistered_actor.first;
    const ActorPtr actor_ptr = unregistered_actor.second;

    const cg::Transform actor_transform = actor_ptr->GetTransform();
    const cg::Location actor_location = actor_transform.location;
    const cg::Rotation actor_rotation = actor_transform.rotation;
    const cg::Vector3D actor_velocity = actor_ptr->GetVelocity();
    KinematicState kinematic_state {true, actor_location, actor_rotation, actor_velocity, -1.0f};

    TrafficLightState tl_state;

    const std::string type_id = actor_ptr->GetTypeId();
    ActorType actor_type = ActorType::Any;
    cg::Vector3D dimensions;
    if (type_id.front() == 'v') {
      auto vehicle_ptr = boost::static_pointer_cast<cc::Vehicle>(actor_ptr);
      kinematic_state.speed_limit = vehicle_ptr->GetSpeedLimit();

      tl_state = {vehicle_ptr->GetTrafficLightState(), vehicle_ptr->IsAtTrafficLight()};

      if (!simulation_state.ContainsActor(actor_id)) {
        dimensions = vehicle_ptr->GetBoundingBox().extent;
        actor_type = ActorType::Vehicle;
        StaticAttributes attributes {actor_type, dimensions.x, dimensions.y, dimensions.z};

        simulation_state.AddActor(actor_id, kinematic_state, attributes, tl_state);
      } else {
        simulation_state.UpdateKinematicState(actor_id, kinematic_state);
        simulation_state.UpdateTrafficLightState(actor_id, tl_state);
      }
    }
    else if (type_id.front() == 'w') {
      auto walker_ptr = boost::static_pointer_cast<cc::Walker>(actor_ptr);

      if (!simulation_state.ContainsActor(actor_id)) {
        dimensions = walker_ptr->GetBoundingBox().extent;
        actor_type = ActorType::Pedestrian;
        StaticAttributes attributes {actor_type, dimensions.x, dimensions.y, dimensions.z};

        simulation_state.AddActor(actor_id, kinematic_state, attributes, tl_state);
      } else {
        simulation_state.UpdateKinematicState(actor_id, kinematic_state);
      }
    }
  }
}

void ALSM::UpdateIdleTime(std::pair<ActorId, double>& max_idle_time, const ActorId& actor_id) {
  if (idle_time.find(actor_id) != idle_time.end()) {
    TrafficLightState tl_state = simulation_state.GetTLS(actor_id);
    if (simulation_state.GetVelocity(actor_id).Length() > STOPPED_VELOCITY_THRESHOLD
        || (tl_state.at_traffic_light && tl_state.tl_state != TLS::Green)) {
      idle_time[actor_id] = current_timestamp.elapsed_seconds;
    }

    // Checking maximum idle time.
    if (max_idle_time.first == 0u || max_idle_time.second > idle_time[actor_id]) {
      max_idle_time = std::make_pair(actor_id, idle_time[actor_id]);
    }
  }
}

bool ALSM::IsVehicleStuck(const ActorId& actor_id) {
  if (idle_time.find(actor_id) != idle_time.end()) {
    auto delta_idle_time = current_timestamp.elapsed_seconds - idle_time.at(actor_id);
    if (delta_idle_time >= BLOCKED_TIME_THRESHOLD) {
      return true;
    }
  }
  return false;
}

void ALSM::RemoveActor(const ActorId actor_id, const bool registered_actor) {
  if (registered_actor) {
    registered_vehicles.Remove({actor_id});
    buffer_map_ptr->erase(actor_id);
    idle_time.erase(actor_id);
    localization_stage.RemoveActor(actor_id);
    collision_stage.RemoveActor(actor_id);
    traffic_light_stage.RemoveActor(actor_id);
    motion_plan_stage.RemoveActor(actor_id);
  }
  else {
    unregistered_actors.erase(actor_id);
    hero_actors.erase(actor_id);
  }

  track_traffic.DeleteActor(actor_id);
  simulation_state.RemoveActor(actor_id);
}

void ALSM::Reset() {
  unregistered_actors.clear();
  idle_time.clear();
  hero_actors.clear();
  elapsed_last_actor_destruction = std::numeric_limits<double>::infinity();
  current_timestamp = world.GetSnapshot().GetTimestamp();
}

} // namespace traffic_manager
} // namespace carla
