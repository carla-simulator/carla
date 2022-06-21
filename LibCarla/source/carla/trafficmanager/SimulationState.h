
#pragma once

#include <unordered_set>

#include "carla/trafficmanager/DataStructures.h"

namespace carla {
namespace traffic_manager {

enum ActorType {
  Vehicle,
  Pedestrian,
  Any
};

struct KinematicState {
  cg::Location location;
  cg::Rotation rotation;
  cg::Vector3D velocity;
  float speed_limit;
  bool physics_enabled;
  bool is_dormant;
};
using KinematicStateMap = std::unordered_map<ActorId, KinematicState>;

struct TrafficLightState {
  TLS tl_state;
  bool at_traffic_light;
};
using TrafficLightStateMap = std::unordered_map<ActorId, TrafficLightState>;

struct StaticAttributes {
  ActorType actor_type;
  float half_length;
  float half_width;
  float half_height;
};
using StaticAttributeMap = std::unordered_map<ActorId, StaticAttributes>;

/// This class holds the state of all the vehicles in the simlation.
class SimulationState {

private:
  // Structure to hold ids of all actors in the simulation.
  std::unordered_set<ActorId> actor_set;
  // Structure containing dynamic motion related state of actors.
  KinematicStateMap kinematic_state_map;
  // Structure containing static attributes of actors.
  StaticAttributeMap static_attribute_map;
  // Structure containing dynamic traffic light related state of actors.
  TrafficLightStateMap tl_state_map;

public :
  SimulationState();

  // Method to add an actor to the simulation state.
  void AddActor(ActorId actor_id,
                KinematicState kinematic_state,
                StaticAttributes attributes,
                TrafficLightState tl_state);

  // Method to verify if an actor is present currently present in the simulation state.
  bool ContainsActor(ActorId actor_id) const;

  // Method to remove an actor from simulation state.
  void RemoveActor(ActorId actor_id);

  // Method to flush all states and actors.
  void Reset();

  void UpdateKinematicState(ActorId actor_id, KinematicState state);

  void UpdateTrafficLightState(ActorId actor_id, TrafficLightState state);

  cg::Location GetLocation(const ActorId actor_id) const;

  cg::Rotation GetRotation(const ActorId actor_id) const;

  cg::Vector3D GetHeading(const ActorId actor_id) const;

  cg::Vector3D GetVelocity(const ActorId actor_id) const;

  float GetSpeedLimit(const ActorId actor_id) const;

  bool IsPhysicsEnabled(const ActorId actor_id) const;

  bool IsDormant(const ActorId actor_id) const;

  cg::Location GetHeroLocation(const ActorId actor_id) const;

  TrafficLightState GetTLS(const ActorId actor_id) const;

  ActorType GetType(const ActorId actor_id) const;

  cg::Vector3D GetDimensions(const ActorId actor_id) const;

};

} // namespace traffic_manager
} // namespace carla
