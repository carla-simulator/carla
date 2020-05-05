
/// This class holds the state of all the vehicles in the simlation.

#pragma once

#include <unordered_set>

#include "carla/geom/Vector3D.h"
#include "carla/rpc/ActorId.h"
#include "carla/trafficmanager/DataStructures.h"

namespace carla
{
namespace traffic_manager
{

namespace cg = carla::geom;

using ActorId = carla::rpc::ActorId;

class SimulationState
{

private:
  std::unordered_set<ActorId> actor_set;
  KinematicStateMap kinematic_state_map;
  StaticAttributeMap static_attribute_map;
  TrafficLightStateMap tl_state_map;

public :
  SimulationState(/* args */);
  ~SimulationState();

  void AddActor(ActorId actor_id,
                KinematicState kinematic_state,
                StaticAttributes attributes,
                TrafficLightState tl_state);

  bool ContainsActor(ActorId actor_id) const;

  void RemoveActor(ActorId actor_id);

  void Reset();

  void UpdateKinematicState(ActorId actorid, KinematicState state);

  void UpdateTrafficLightState(ActorId actor_id, TrafficLightState state);

  cg::Location GetLocation(const ActorId actor_id) const;

  cg::Rotation GetRotation(const ActorId actor_id) const;

  cg::Vector3D GetHeading(const ActorId actor_id) const;

  cg::Vector3D GetVelocity(const ActorId actor_id) const;

  float GetSpeedLimit(const ActorId actor_id) const;

  bool IsPhysicsEnabled(const ActorId actor_id) const;

  TrafficLightState GetTLS(const ActorId actor_id) const;

  ActorType GetType(const ActorId actor_id) const;

  cg::Vector3D GetDimensions(const ActorId actor_id) const;

};

} // namespace traffic_manager
} // namespace carla
