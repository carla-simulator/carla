
#include "boost/pointer_cast.hpp"

#include "carla/client/Actor.h"
#include "carla/client/Vehicle.h"
#include "carla/client/Walker.h"

#include "carla/trafficmanager/Constants.h"
#include "carla/trafficmanager/LocalizationUtils.h"
#include "carla/trafficmanager/SimpleWaypoint.h"

#include "carla/trafficmanager/ALSM.h"

namespace carla {
namespace traffic_manager {

ALSM::ALSM(
  AtomicActorSet &registered_vehicles,
  BufferMap &buffer_map,
  TrackTraffic &track_traffic,
  std::vector<ActorId>& marked_for_removal,
  const Parameters &parameters,
  const cc::World &world,
  const LocalMapPtr &local_map,
  SimulationState &simulation_state,
  LocalizationStage &localization_stage,
  CollisionStage &collision_stage,
  TrafficLightStage &traffic_light_stage,
  MotionPlanStage &motion_plan_stage,
  VehicleLightStage &vehicle_light_stage)
  : registered_vehicles(registered_vehicles),
    buffer_map(buffer_map),
    track_traffic(track_traffic),
    marked_for_removal(marked_for_removal),
    parameters(parameters),
    world(world),
    local_map(local_map),
    simulation_state(simulation_state),
    localization_stage(localization_stage),
    collision_stage(collision_stage),
    traffic_light_stage(traffic_light_stage),
    motion_plan_stage(motion_plan_stage),
    vehicle_light_stage(vehicle_light_stage) {}

void ALSM::Update() {

  bool hybrid_physics_mode = parameters.GetHybridPhysicsMode();

  std::set<ActorId> world_vehicle_ids;
  std::set<ActorId> world_pedestrian_ids;
  std::vector<ActorId> unregistered_list_to_be_deleted;

  current_timestamp = world.GetSnapshot().GetTimestamp();
  ActorList world_actors = world.GetActors();

  // Find destroyed actors and perform clean up.
  const ALSM::DestroyeddActors destroyed_actors = IdentifyDestroyedActors(world_actors);

  const ActorIdSet &destroyed_registered = destroyed_actors.first;
  for (const auto &deletion_id: destroyed_registered) {
    RemoveActor(deletion_id, true);
  }

  const ActorIdSet &destroyed_unregistered = destroyed_actors.second;
  for (auto deletion_id : destroyed_unregistered) {
    RemoveActor(deletion_id, false);
  }

  // Invalidate hero actor if it is not alive anymore.
  if (hero_actors.size() != 0u) {
    ActorIdSet hero_actors_to_delete;
    for (auto &hero_actor_info: hero_actors) {
      if (destroyed_unregistered.find(hero_actor_info.first) != destroyed_unregistered.end()) {
        hero_actors_to_delete.insert(hero_actor_info.first);
      }
      if (destroyed_registered.find(hero_actor_info.first) != destroyed_registered.end()) {
        hero_actors_to_delete.insert(hero_actor_info.first);
      }
    }

    for (auto &deletion_id: hero_actors_to_delete) {
      hero_actors.erase(deletion_id);
    }
  }

  // Scan for new unregistered actors.
  IdentifyNewActors(world_actors);

  // Update dynamic state and static attributes for all registered vehicles.
  ALSM::IdleInfo max_idle_time = std::make_pair(0u, current_timestamp.elapsed_seconds);
  UpdateRegisteredActorsData(hybrid_physics_mode, max_idle_time);

  // Destroy registered vehicle if stuck at a location for too long.
  if (IsVehicleStuck(max_idle_time.first)
      && (current_timestamp.elapsed_seconds - elapsed_last_actor_destruction) > DELTA_TIME_BETWEEN_DESTRUCTIONS
      && hero_actors.find(max_idle_time.first) == hero_actors.end()) {
    registered_vehicles.Destroy(max_idle_time.first);
    RemoveActor(max_idle_time.first, true);
    elapsed_last_actor_destruction = current_timestamp.elapsed_seconds;
  }

  // Destorying vehicles for marked for removal by stages.
  if (parameters.GetOSMMode()) {
    for (const ActorId& actor_id: marked_for_removal) {
      registered_vehicles.Destroy(actor_id);
      RemoveActor(actor_id, true);
    }
    marked_for_removal.clear();
  }

  // Update dynamic state and static attributes for unregistered actors.
  UpdateUnregisteredActorsData();
}

void ALSM::IdentifyNewActors(const ActorList &actor_list) {
  for (auto iter = actor_list->begin(); iter != actor_list->end(); ++iter) {
    ActorPtr actor = *iter;
    ActorId actor_id = actor->GetId();
    // Identify any new hero vehicle
    if (actor->GetTypeId().front() == 'v') {
     if (hero_actors.size() == 0u || hero_actors.find(actor_id) == hero_actors.end()) {
      for (auto&& attribute: actor->GetAttributes()) {
        if (attribute.GetId() == "role_name" && attribute.GetValue() == "hero") {
          hero_actors.insert({actor_id, actor});
        }
      }
    }
  }
    if (!registered_vehicles.Contains(actor_id)
        && unregistered_actors.find(actor_id) == unregistered_actors.end()) {

      unregistered_actors.insert({actor_id, actor});
    }
  }
}

ALSM::DestroyeddActors ALSM::IdentifyDestroyedActors(const ActorList &actor_list) {

  ALSM::DestroyeddActors destroyed_actors;
  ActorIdSet &deleted_registered = destroyed_actors.first;
  ActorIdSet &deleted_unregistered = destroyed_actors.second;

  // Building hash set of actors present in current frame.
  ActorIdSet current_actors;
  for  (auto iter = actor_list->begin(); iter != actor_list->end(); ++iter) {
    current_actors.insert((*iter)->GetId());
  }

  // Searching for destroyed registered actors.
  std::vector<ActorId> registered_ids = registered_vehicles.GetIDList();
  for (const ActorId &actor_id : registered_ids) {
    if (current_actors.find(actor_id) == current_actors.end()) {
      deleted_registered.insert(actor_id);
    }
  }

  // Searching for destroyed unregistered actors.
  for (const auto &actor_info: unregistered_actors) {
    const ActorId &actor_id = actor_info.first;
     if (current_actors.find(actor_id) == current_actors.end()
         || registered_vehicles.Contains(actor_id)) {
      deleted_unregistered.insert(actor_id);
    }
  }

  return destroyed_actors;
}

void ALSM::UpdateRegisteredActorsData(const bool hybrid_physics_mode, ALSM::IdleInfo &max_idle_time) {

  std::vector<ActorPtr> vehicle_list = registered_vehicles.GetList();
  bool hero_actor_present = hero_actors.size() != 0u;
  float physics_radius = parameters.GetHybridPhysicsRadius();
  float physics_radius_square = SQUARE(physics_radius);
  bool is_respawn_vehicles = parameters.GetRespawnDormantVehicles();
  if (is_respawn_vehicles && !hero_actor_present) {
    track_traffic.SetHeroLocation(cg::Location(0,0,0));
  }
  // Update first the information regarding any hero vehicle.
  for (auto &hero_actor_info: hero_actors){
    if (is_respawn_vehicles) {
      track_traffic.SetHeroLocation(hero_actor_info.second->GetTransform().location);
    }
    UpdateData(hybrid_physics_mode, hero_actor_info.second, hero_actor_present, physics_radius_square);
  }
  // Update information for all other registered vehicles.
  for (const Actor &vehicle : vehicle_list) {
    ActorId actor_id = vehicle->GetId();
    if (hero_actors.find(actor_id) == hero_actors.end()) {
      UpdateData(hybrid_physics_mode, vehicle, hero_actor_present, physics_radius_square);
      UpdateIdleTime(max_idle_time, actor_id);
    }
  }
}

void ALSM::UpdateData(const bool hybrid_physics_mode, const Actor &vehicle,
                      const bool hero_actor_present, const float physics_radius_square) {

  ActorId actor_id = vehicle->GetId();
  cg::Transform vehicle_transform = vehicle->GetTransform();
  cg::Location vehicle_location = vehicle_transform.location;
  cg::Rotation vehicle_rotation = vehicle_transform.rotation;
  cg::Vector3D vehicle_velocity = vehicle->GetVelocity();
  bool state_entry_present = simulation_state.ContainsActor(actor_id);

  // Initializing idle times.
  if (idle_time.find(actor_id) == idle_time.end() && current_timestamp.elapsed_seconds != 0.0) {
    idle_time.insert({actor_id, current_timestamp.elapsed_seconds});
  }

  // Check if current actor is in range of hero actor and enable physics in hybrid mode.
  bool in_range_of_hero_actor = false;
  if (hero_actor_present && hybrid_physics_mode) {
    for (auto &hero_actor_info: hero_actors) {
      const ActorId &hero_actor_id =  hero_actor_info.first;
      if (simulation_state.ContainsActor(hero_actor_id)) {
        const cg::Location &hero_location = simulation_state.GetLocation(hero_actor_id);
        if (cg::Math::DistanceSquared(vehicle_location, hero_location) < physics_radius_square) {
          in_range_of_hero_actor = true;
          break;
        }
      }
    }
  }

  bool enable_physics = hybrid_physics_mode ? in_range_of_hero_actor : true;
  if (!has_physics_enabled.count(actor_id) || has_physics_enabled[actor_id] != enable_physics) {
    if (hero_actors.find(actor_id) == hero_actors.end()) {
      vehicle->SetSimulatePhysics(enable_physics);
      has_physics_enabled[actor_id] = enable_physics;
      if (enable_physics == true && state_entry_present) {
        vehicle->SetTargetVelocity(simulation_state.GetVelocity(actor_id));
      }
    }
  }

  // If physics are disabled, calculate velocity based on change in position.
  // Do not use 'enable_physics' as turning off the physics in this tick doesn't remove the velocity.
  // To avoid issues with other clients teleporting the actors, use the previous outpout location.
  if (state_entry_present && !simulation_state.IsPhysicsEnabled(actor_id)){
    cg::Location previous_location = simulation_state.GetLocation(actor_id);
    cg::Location previous_end_location = simulation_state.GetHybridEndLocation(actor_id);
    cg::Vector3D displacement = (previous_end_location - previous_location);
    vehicle_velocity = displacement * INV_HYBRID_DT;
  }

  // Updated kinematic state object.
  auto vehicle_ptr = boost::static_pointer_cast<cc::Vehicle>(vehicle);
  KinematicState kinematic_state{vehicle_location, vehicle_rotation,
                                  vehicle_velocity, vehicle_ptr->GetSpeedLimit(),
                                  enable_physics, vehicle->IsDormant(), cg::Location()};

  // Updated traffic light state object.
  TrafficLightState tl_state = {vehicle_ptr->GetTrafficLightState(), vehicle_ptr->IsAtTrafficLight()};

  // Update simulation state.
  if (state_entry_present) {
    simulation_state.UpdateKinematicState(actor_id, kinematic_state);
    simulation_state.UpdateTrafficLightState(actor_id, tl_state);
  }
  else {
    cg::Vector3D dimensions = vehicle_ptr->GetBoundingBox().extent;
    StaticAttributes attributes{ActorType::Vehicle, dimensions.x, dimensions.y, dimensions.z};

    simulation_state.AddActor(actor_id, kinematic_state, attributes, tl_state);
  }
}


void ALSM::UpdateUnregisteredActorsData() {
  for (auto &actor_info: unregistered_actors) {

    const ActorId actor_id = actor_info.first;
    const ActorPtr actor_ptr = actor_info.second;
    const std::string type_id = actor_ptr->GetTypeId();

    const cg::Transform actor_transform = actor_ptr->GetTransform();
    const cg::Location actor_location = actor_transform.location;
    const cg::Rotation actor_rotation = actor_transform.rotation;
    const cg::Vector3D actor_velocity = actor_ptr->GetVelocity();
    const bool actor_is_dormant = actor_ptr->IsDormant();
    KinematicState kinematic_state {actor_location, actor_rotation, actor_velocity, -1.0f, true, actor_is_dormant, cg::Location()};

    TrafficLightState tl_state;
    ActorType actor_type = ActorType::Any;
    cg::Vector3D dimensions;
    std::vector<SimpleWaypointPtr> nearest_waypoints;

    bool state_entry_not_present = !simulation_state.ContainsActor(actor_id);
    if (type_id.front() == 'v') {
      auto vehicle_ptr = boost::static_pointer_cast<cc::Vehicle>(actor_ptr);
      kinematic_state.speed_limit = vehicle_ptr->GetSpeedLimit();

      tl_state = {vehicle_ptr->GetTrafficLightState(), vehicle_ptr->IsAtTrafficLight()};

      if (state_entry_not_present) {
        dimensions = vehicle_ptr->GetBoundingBox().extent;
        actor_type = ActorType::Vehicle;
        StaticAttributes attributes {actor_type, dimensions.x, dimensions.y, dimensions.z};

        simulation_state.AddActor(actor_id, kinematic_state, attributes, tl_state);
      } else {
        simulation_state.UpdateKinematicState(actor_id, kinematic_state);
        simulation_state.UpdateTrafficLightState(actor_id, tl_state);
      }

      // Identify occupied waypoints.
      cg::Vector3D extent = vehicle_ptr->GetBoundingBox().extent;
      cg::Vector3D heading_vector = vehicle_ptr->GetTransform().GetForwardVector();
      std::vector<cg::Location> corners = {actor_location + cg::Location(extent.x * heading_vector),
                                           actor_location,
                                           actor_location + cg::Location(-extent.x * heading_vector)};
      for (cg::Location &vertex: corners) {
        SimpleWaypointPtr nearest_waypoint = local_map->GetWaypoint(vertex);
        nearest_waypoints.push_back(nearest_waypoint);
      }
    }
    else if (type_id.front() == 'w') {
      auto walker_ptr = boost::static_pointer_cast<cc::Walker>(actor_ptr);

      if (state_entry_not_present) {
        dimensions = walker_ptr->GetBoundingBox().extent;
        actor_type = ActorType::Pedestrian;
        StaticAttributes attributes {actor_type, dimensions.x, dimensions.y, dimensions.z};

        simulation_state.AddActor(actor_id, kinematic_state, attributes, tl_state);
      } else {
        simulation_state.UpdateKinematicState(actor_id, kinematic_state);
      }

      // Identify occupied waypoints.
      SimpleWaypointPtr nearest_waypoint = local_map->GetWaypoint(actor_location);
      nearest_waypoints.push_back(nearest_waypoint);
    }

    track_traffic.UpdateUnregisteredGridPosition(actor_id, nearest_waypoints);
  }
}

void ALSM::UpdateIdleTime(std::pair<ActorId, double>& max_idle_time, const ActorId& actor_id) {
  if (idle_time.find(actor_id) != idle_time.end()) {
    double &idle_duration = idle_time.at(actor_id);
    if (simulation_state.GetVelocity(actor_id).SquaredLength() > SQUARE(STOPPED_VELOCITY_THRESHOLD)) {
      idle_duration = current_timestamp.elapsed_seconds;
    }

    // Checking maximum idle time.
    if (max_idle_time.first == 0u || max_idle_time.second > idle_duration) {
      max_idle_time = std::make_pair(actor_id, idle_duration);
    }
  }
}

bool ALSM::IsVehicleStuck(const ActorId& actor_id) {
  if (idle_time.find(actor_id) != idle_time.end()) {
    double delta_idle_time = current_timestamp.elapsed_seconds - idle_time.at(actor_id);
    TrafficLightState tl_state = simulation_state.GetTLS(actor_id);
    if ((delta_idle_time >= RED_TL_BLOCKED_TIME_THRESHOLD)
    || (delta_idle_time >= BLOCKED_TIME_THRESHOLD && tl_state.tl_state != TLS::Red))
    {
      return true;
    }
  }
  return false;
}

void ALSM::RemoveActor(const ActorId actor_id, const bool registered_actor) {
  if (registered_actor) {
    registered_vehicles.Remove({actor_id});
    buffer_map.erase(actor_id);
    idle_time.erase(actor_id);
    localization_stage.RemoveActor(actor_id);
    collision_stage.RemoveActor(actor_id);
    traffic_light_stage.RemoveActor(actor_id);
    motion_plan_stage.RemoveActor(actor_id);
    vehicle_light_stage.RemoveActor(actor_id);
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
  elapsed_last_actor_destruction = 0.0;
  current_timestamp = world.GetSnapshot().GetTimestamp();
}

} // namespace traffic_manager
} // namespace carla
