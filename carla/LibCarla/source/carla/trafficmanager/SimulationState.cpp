
#include "carla/trafficmanager/SimulationState.h"

namespace carla {
namespace traffic_manager {

SimulationState::SimulationState() {}

void SimulationState::AddActor(ActorId actor_id,
                               KinematicState kinematic_state,
                               StaticAttributes attributes,
                               TrafficLightState tl_state) {
  actor_set.insert(actor_id);
  kinematic_state_map.insert({actor_id, kinematic_state});
  static_attribute_map.insert({actor_id, attributes});
  tl_state_map.insert({actor_id, tl_state});
}

bool SimulationState::ContainsActor(ActorId actor_id) const {
  return actor_set.find(actor_id) != actor_set.end();
}

void SimulationState::RemoveActor(ActorId actor_id) {
  actor_set.erase(actor_id);
  kinematic_state_map.erase(actor_id);
  static_attribute_map.erase(actor_id);
  tl_state_map.erase(actor_id);
}

void SimulationState::Reset() {
  actor_set.clear();
  kinematic_state_map.clear();
  static_attribute_map.clear();
  tl_state_map.clear();
}

void SimulationState::UpdateKinematicState(ActorId actor_id, KinematicState state) {
  kinematic_state_map.at(actor_id) = state;
}

void SimulationState::UpdateKinematicHybridEndLocation(ActorId actor_id, cg::Location location) {
  kinematic_state_map.at(actor_id).hybrid_end_location = location;
}

void SimulationState::UpdateTrafficLightState(ActorId actor_id, TrafficLightState state) {
  // The green-yellow state transition is not notified to the vehicle. This is done to avoid
  // having vehicles stopped very near the intersection when only the rear part of the vehicle
  // is colliding with the trigger volume of the traffic light.
  auto previous_tl_state = GetTLS(actor_id);
  if (previous_tl_state.at_traffic_light && previous_tl_state.tl_state == TLS::Green) {
    state.tl_state = TLS::Green;
  }

  tl_state_map.at(actor_id) = state;
}

cg::Location SimulationState::GetLocation(ActorId actor_id) const {
  return kinematic_state_map.at(actor_id).location;
}

cg::Location SimulationState::GetHybridEndLocation(ActorId actor_id) const {
  return kinematic_state_map.at(actor_id).hybrid_end_location;
}

cg::Rotation SimulationState::GetRotation(ActorId actor_id) const {
  return kinematic_state_map.at(actor_id).rotation;
}

cg::Vector3D SimulationState::GetHeading(ActorId actor_id) const {
  return kinematic_state_map.at(actor_id).rotation.GetForwardVector();
}

cg::Vector3D SimulationState::GetVelocity(ActorId actor_id) const {
  return kinematic_state_map.at(actor_id).velocity;
}

float SimulationState::GetSpeedLimit(ActorId actor_id) const {
  return kinematic_state_map.at(actor_id).speed_limit;
}

bool SimulationState::IsPhysicsEnabled(ActorId actor_id) const {
  return kinematic_state_map.at(actor_id).physics_enabled;
}

bool SimulationState::IsDormant(ActorId actor_id) const {
  return kinematic_state_map.at(actor_id).is_dormant;
}

TrafficLightState SimulationState::GetTLS(ActorId actor_id) const {
  return tl_state_map.at(actor_id);
}

ActorType SimulationState::GetType(ActorId actor_id) const {
  return static_attribute_map.at(actor_id).actor_type;
}

cg::Vector3D SimulationState::GetDimensions(ActorId actor_id) const {
  const StaticAttributes &attributes = static_attribute_map.at(actor_id);
  return cg::Vector3D(attributes.half_length, attributes.half_width, attributes.half_height);
}

} // namespace  traffic_manager
} // namespace carla
