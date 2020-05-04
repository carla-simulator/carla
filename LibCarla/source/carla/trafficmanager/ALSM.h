
/// ALSM: Agent Lifecycle and State Managerment
/// This file has functionality to update the local cache of kinematic states
/// and manage memory and cleanup for varying number of vehicles in the simulation.

#pragma once

#include <memory>
#include <mutex>
#include <unordered_map>

#include "carla/client/Actor.h"
#include "carla/client/ActorList.h"
#include "carla/client/Timestamp.h"
#include "carla/client/Vehicle.h"
#include "carla/client/Walker.h"
#include "carla/client/World.h"
#include "carla/Memory.h"
#include "carla/rpc/ActorId.h"
#include "carla/rpc/TrafficLightState.h"
#include "boost/pointer_cast.hpp"

#include "carla/trafficmanager/AtomicActorSet.h"
#include "carla/trafficmanager/Constants.h"
#include "carla/trafficmanager/DataStructures.h"
#include "carla/trafficmanager/InMemoryMap.h"
#include "carla/trafficmanager/LocalizationUtils.h"
#include "carla/trafficmanager/Parameters.h"
#include "carla/trafficmanager/SimpleWaypoint.h"
#include "carla/trafficmanager/VehicleStateAndAttributeQuery.h"

namespace carla
{
namespace traffic_manager
{

using namespace constants::HybridMode;
using namespace constants::VehicleRemoval;

namespace chr = std::chrono;
namespace cg = carla::geom;
namespace cc = carla::client;

using ActorMap = std::unordered_map<ActorId, ActorPtr>;
using ActorList = carla::SharedPtr<cc::ActorList>;
using LaneChangeLocationMap = std::unordered_map<ActorId, cg::Location>;
using IdleTimeMap = std::unordered_map<ActorId, double>;
using IdToIndexMap = std::unordered_map<ActorId, unsigned long>;
using LocalMapPtr = std::shared_ptr<InMemoryMap>;

void UpdateIdleTime(IdleTimeMap& idle_time,
                    std::pair<ActorId, double>& max_idle_time,
                    const ActorId& actor_id,
                    const KinematicStateMap &kinematic_state_map,
                    const TrafficLightStateMap &tl_state_map,
                    const cc::Timestamp &current_timestamp)
{
  if (idle_time.find(actor_id) != idle_time.end()) {
    TrafficLightState tl_state = GetTLS(tl_state_map, actor_id);
    if (GetVelocity(kinematic_state_map, actor_id).Length() > STOPPED_VELOCITY_THRESHOLD
        || (tl_state.at_traffic_light && tl_state.tl_state != TLS::Green)) {
      idle_time[actor_id] = current_timestamp.elapsed_seconds;
    }

    // Checking maximum idle time.
    if (max_idle_time.first == 0u || max_idle_time.second > idle_time[actor_id]) {
      max_idle_time = std::make_pair(actor_id, idle_time[actor_id]);
    }
  }
}

bool IsVehicleStuck(const IdleTimeMap& idle_time, const ActorId& actor_id, const cc::Timestamp &current_timestamp)
{
  if (idle_time.find(actor_id) != idle_time.end()) {
    auto delta_idle_time = current_timestamp.elapsed_seconds - idle_time.at(actor_id);
    if (delta_idle_time >= BLOCKED_TIME_THRESHOLD) {
      return true;
    }
  }
  return false;
}

void RemoveActor(const ActorId actor_id,
                 AtomicActorSet &registered_actors,
                 TrackTraffic &track_traffic,
                 ActorMap &unregistered_actors,
                 BufferMapPtr &buffer_map_ptr,
                 KinematicStateMap &kinematic_state_map,
                 StaticAttributeMap &attribute_map,
                 TrafficLightStateMap &tl_state_map,
                 LaneChangeLocationMap &lane_change_location_map,
                 const bool registered_actor)
{
  if (registered_actor)
  {
    registered_actors.Remove({actor_id});
    buffer_map_ptr->erase(actor_id);
    lane_change_location_map.erase(actor_id);
  }
  else
  {
    unregistered_actors.erase(actor_id);
  }

  track_traffic.DeleteActor(actor_id);
  kinematic_state_map.erase(actor_id);
  attribute_map.erase(actor_id);
  tl_state_map.erase(actor_id);
}

void AgentLifecycleAndStateManagement(AtomicActorSet &registered_vehicles,
                                      std::vector<ActorId> &vehicle_id_list,
                                      ActorMap &unregistered_actors,
                                      const int &registered_vehicles_state,
                                      BufferMapPtr &buffer_map_ptr,
                                      TrackTraffic &track_traffic,
                                      IdleTimeMap &idle_time,
                                      std::unordered_map<ActorId, Actor> &hero_actors,
                                      LaneChangeLocationMap &last_lane_change_location,
                                      KinematicStateMap &kinematic_state_map,
                                      StaticAttributeMap &static_attribute_map,
                                      TrafficLightStateMap &tls_map,
                                      double &elapsed_last_actor_destruction,
                                      const Parameters &parameters,
                                      const cc::World &world,
                                      const LocalMapPtr &local_map)
{

  bool hybrid_physics_mode = parameters.GetHybridPhysicsMode();

  bool is_deleted_actors_present = false;
  std::set<unsigned int> world_vehicle_ids;
  std::set<unsigned int> world_pedestrian_ids;
  std::vector<ActorId> unregistered_list_to_be_deleted;

  const cc::Timestamp current_timestamp = world.GetSnapshot().GetTimestamp();
  ActorList world_actors = world.GetActors();
  ActorList world_vehicles = world_actors->Filter("vehicle.*");
  ActorList world_pedestrians = world_actors->Filter("walker.*");

  // Scanning for new unregistered vehicles.
  for (auto iter = world_vehicles->begin(); iter != world_vehicles->end(); ++iter)
  {
    // Building set containing current world vehicle ids.
    const auto unregistered_id = (*iter)->GetId();
    world_vehicle_ids.insert(unregistered_id);
    if (!registered_vehicles.Contains(unregistered_id)
        && unregistered_actors.find(unregistered_id) == unregistered_actors.end())
    {
      unregistered_actors.insert({unregistered_id, *iter});
    }
  }

  // Scanning for new pedestrians.
  for (auto iter = world_pedestrians->begin(); iter != world_pedestrians->end(); ++iter)
  {
    // Building set containing current world pedestrian ids.
    const auto unregistered_id = (*iter)->GetId();
    world_pedestrian_ids.insert(unregistered_id);
    if (unregistered_actors.find(unregistered_id) == unregistered_actors.end())
    {
      unregistered_actors.insert({unregistered_id, *iter});
    }
  }

  // Identify hero vehicle if currently not present
  // and system is in hybrid physics mode.
  if (hybrid_physics_mode)
  {
    for (auto iter = unregistered_actors.begin(); iter != unregistered_actors.end(); ++iter)
    {
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
  for (const auto &deletion_id : vehicle_id_list)
  {
    if (world_vehicle_ids.find(deletion_id) == world_vehicle_ids.end())
    {
      RemoveActor(deletion_id,
                  registered_vehicles,
                  track_traffic,
                  unregistered_actors,
                  buffer_map_ptr,
                  kinematic_state_map,
                  static_attribute_map,
                  tls_map,
                  last_lane_change_location,
                  true);
      if (!vehicles_unregistered) {
        vehicles_unregistered = true;
      }
    }
  }

  // Building a list of registered actors.
  if (vehicles_unregistered || (registered_vehicles_state != registered_vehicles.GetState()))
  {
    vehicle_id_list.clear();
    vehicle_id_list = registered_vehicles.GetIDList();
    is_deleted_actors_present = true;
  }

  // Regularly update unregistered actor states and clean up any invalid actors.
  for (auto iter = unregistered_actors.begin(); iter != unregistered_actors.cend(); ++iter)
  {
    ActorId unregistered_actor_id = iter->first;
    if (registered_vehicles.Contains(unregistered_actor_id)
        || (world_vehicle_ids.find(unregistered_actor_id) == world_vehicle_ids.end()
            && world_pedestrian_ids.find(unregistered_actor_id) == world_pedestrian_ids.end()))
    {
      unregistered_list_to_be_deleted.push_back(iter->first);
    }
    else
    {
      // Updating data structures.
      cg::Location location = iter->second->GetLocation();
      const auto type = iter->second->GetTypeId();

      std::vector<SimpleWaypointPtr> nearest_waypoints;
      if (type[0] == 'v') {
        auto vehicle_ptr = boost::static_pointer_cast<cc::Vehicle>(iter->second);
        cg::Vector3D extent = vehicle_ptr->GetBoundingBox().extent;
        float bx = extent.x;
        float by = extent.y;
        std::vector<cg::Location> corners = {location + cg::Location(bx, by, 0.0f),
                                              location + cg::Location(-bx, by, 0.0f),
                                              location + cg::Location(bx, -by, 0.0f),
                                              location + cg::Location(-bx, -by, 0.0f)};
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
  for (auto deletion_id : unregistered_list_to_be_deleted)
  {
    RemoveActor(deletion_id,
                registered_vehicles,
                track_traffic,
                unregistered_actors,
                buffer_map_ptr,
                kinematic_state_map,
                static_attribute_map,
                tls_map,
                last_lane_change_location,
                false);
  }

  // Update dynamic state and static attributes for all registered vehicles.
  float dt = HYBRID_MODE_DT;
  std::pair<ActorId, double> max_idle_time = std::make_pair(0u, current_timestamp.elapsed_seconds);
  std::vector<ActorPtr> vehicle_list = registered_vehicles.GetList();
  for (const Actor &vehicle : vehicle_list)
  {

    ActorId actor_id = vehicle->GetId();
    cg::Transform vehicle_transform = vehicle->GetTransform();
    cg::Location vehicle_location = vehicle_transform.location;
    cg::Rotation vehicle_rotation = vehicle_transform.rotation;
    cg::Vector3D vehicle_velocity = vehicle->GetVelocity();

    // Initializing idle times.
    if (idle_time.find(actor_id) == idle_time.end() && current_timestamp.elapsed_seconds != 0) {
      idle_time.insert({actor_id, current_timestamp.elapsed_seconds});
    }

    auto vehicle_ptr = boost::static_pointer_cast<cc::Vehicle>(vehicle);

    // Update static attribute map if entry not present.
    if (static_attribute_map.find(actor_id) == static_attribute_map.end())
    {
      cg::Vector3D dimensions = vehicle_ptr->GetBoundingBox().extent;
      static_attribute_map.insert({actor_id, {ActorType::Vehicle,
                                              dimensions.x, dimensions.y, dimensions.z,
                                              vehicle_ptr->GetSpeedLimit()}});
    }

    // Update traffic light state.
    TrafficLightState tl_state = {vehicle_ptr->GetTrafficLightState(), vehicle_ptr->IsAtTrafficLight()};
    if (tls_map.find(actor_id) == tls_map.end()) {
      tls_map.insert({actor_id, tl_state});
    } else {
      tls_map.at(actor_id) = tl_state;
    }

    // Adding entry if not present.
    if (kinematic_state_map.find(actor_id) == kinematic_state_map.end())
    {
      kinematic_state_map.insert({actor_id, KinematicState{true, vehicle_location, vehicle_rotation, cg::Vector3D()}});
    }

    // Check if current actor is in range of hero actor and enable physics in hybrid mode.
    float hybrid_physics_radius = parameters.GetHybridPhysicsRadius();
    bool in_range_of_hero_actor = false;
    if (hybrid_physics_mode && hero_actors.size() != 0u) {
        for (auto &hero_actor_info: hero_actors) {
        if (kinematic_state_map.find(hero_actor_info.first) != kinematic_state_map.end()) {
          const cg::Location &hero_location = kinematic_state_map.at(hero_actor_info.first).location;
          if (cg::Math::DistanceSquared(vehicle_location, hero_location) < std::pow(hybrid_physics_radius, 2)) {
            in_range_of_hero_actor = true;
            break;
          }
        }
      }
    }
    bool enable_physics = hybrid_physics_mode ? in_range_of_hero_actor : true;
    kinematic_state_map.at(actor_id).physics_enabled = enable_physics;
    vehicle->SetSimulatePhysics(enable_physics);

    if (!enable_physics)
    {
      cg::Vector3D displacement = (vehicle_location - kinematic_state_map.at(actor_id).location);
      vehicle_velocity = displacement / dt;
    }

    // Updating state.
    kinematic_state_map.at(actor_id) = {enable_physics, vehicle_location, vehicle_rotation, vehicle_velocity};

    // Updating idle time when necessary.
    UpdateIdleTime(idle_time, max_idle_time, actor_id, kinematic_state_map, tls_map, current_timestamp);
  }

  if (IsVehicleStuck(idle_time, max_idle_time.first, current_timestamp)
      && current_timestamp.elapsed_seconds - elapsed_last_actor_destruction > DELTA_TIME_BETWEEN_DESTRUCTIONS) {
    RemoveActor(max_idle_time.first,
                registered_vehicles,
                track_traffic,
                unregistered_actors,
                buffer_map_ptr,
                kinematic_state_map,
                static_attribute_map,
                tls_map,
                last_lane_change_location,
                true);
    elapsed_last_actor_destruction = current_timestamp.elapsed_seconds;
  }

  // Update kinematic state and static attributes for unregistered actors.
  for (auto &unregistered_actor: unregistered_actors)
  {
    const ActorId actor_id = unregistered_actor.first;
    const ActorPtr actor_ptr = unregistered_actor.second;
    // Update static attribute map if entry not present.
    if (static_attribute_map.find(actor_id) == static_attribute_map.end())
    {
      const std::string type_id = actor_ptr->GetTypeId();
      ActorType actor_type = ActorType::Any;
      cg::Vector3D dimensions;
      float speed_limit = -1.0f;
      if (type_id.front() == 'v')
      {
        auto vehicle_ptr = boost::static_pointer_cast<cc::Vehicle>(actor_ptr);
        dimensions = vehicle_ptr->GetBoundingBox().extent;
        actor_type = ActorType::Vehicle;
        speed_limit = vehicle_ptr->GetSpeedLimit();

        // Update traffic light state.
        TrafficLightState tl_state = {vehicle_ptr->GetTrafficLightState(), vehicle_ptr->IsAtTrafficLight()};
        if (tls_map.find(actor_id) == tls_map.end()) {
          tls_map.insert({actor_id, tl_state});
        } else {
          tls_map.at(actor_id) = tl_state;
        }
      }
      else if (type_id.front() == 'w')
      {
        auto walker_ptr = boost::static_pointer_cast<cc::Walker>(actor_ptr);
        dimensions = walker_ptr->GetBoundingBox().extent;
        actor_type = ActorType::Pedestrian;
      }
      static_attribute_map.insert({actor_id, {actor_type, dimensions.x, dimensions.y, dimensions.z, speed_limit}});
    }

    // Update kinematic state for the actor.
    const cg::Transform actor_transform = actor_ptr->GetTransform();
    const cg::Location actor_location = actor_transform.location;
    const cg::Rotation actor_rotation = actor_transform.rotation;
    const cg::Vector3D actor_velocity = actor_ptr->GetVelocity();
    const KinematicState kinematic_state = KinematicState{true, actor_location, actor_rotation, actor_velocity};
    // Adding entry if not present.
    if (kinematic_state_map.find(actor_id) == kinematic_state_map.end())
    {
      kinematic_state_map.insert({actor_id, kinematic_state});
    }
    else
    {
      kinematic_state_map.at(actor_id) = kinematic_state;
    }
  }
}

} // namespace traffic_manager
} // namespace carla
