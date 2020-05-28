
#pragma once

#include <memory>
#include <unordered_map>

#include "carla/client/Timestamp.h"
#include "carla/client/World.h"
#include "carla/Memory.h"

#include "carla/trafficmanager/AtomicActorSet.h"
#include "carla/trafficmanager/CollisionStage.h"
#include "carla/trafficmanager/DataStructures.h"
#include "carla/trafficmanager/InMemoryMap.h"
#include "carla/trafficmanager/LocalizationStage.h"
#include "carla/trafficmanager/MotionPlanStage.h"
#include "carla/trafficmanager/Parameters.h"
#include "carla/trafficmanager/SimulationState.h"
#include "carla/trafficmanager/TrafficLightStage.h"

namespace carla {
namespace traffic_manager {

using namespace constants::HybridMode;
using namespace constants::VehicleRemoval;

namespace chr = std::chrono;
namespace cg = carla::geom;
namespace cc = carla::client;

using ActorPtr = carla::SharedPtr<cc::Actor>;
using ActorMap = std::unordered_map<ActorId, ActorPtr>;
using IdleTimeMap = std::unordered_map<ActorId, double>;
using LocalMapPtr = std::shared_ptr<InMemoryMap>;

/// ALSM: Agent Lifecycle and State Managerment
/// This class has functionality to update the local cache of kinematic states
/// and manage memory and cleanup for varying number of vehicles in the simulation.
class ALSM {

private:
  AtomicActorSet &registered_vehicles;
  // Structure containing vehicles in the simulator not registered with the traffic manager.
  ActorMap unregistered_actors;
  BufferMapPtr &buffer_map_ptr;
  // Structure keeping track of duration of vehicles stuck in a location.
  IdleTimeMap idle_time;
  // Structure containing vehicles with attribute role_name with value hero.
  ActorMap hero_actors;
  TrackTraffic &track_traffic;
  const Parameters &parameters;
  const cc::World &world;
  const LocalMapPtr &local_map;
  SimulationState &simulation_state;
  LocalizationStage &localization_stage;
  CollisionStage &collision_stage;
  TrafficLightStage &traffic_light_stage;
  MotionPlanStage &motion_plan_stage;
  // Time elapsed since last vehicle destruction due to being idle for too long.
  double elapsed_last_actor_destruction {0.0};
  cc::Timestamp current_timestamp;

  // Updates the duration for which a registered vehicle is stuck at a location.
  void UpdateIdleTime(std::pair<ActorId, double>& max_idle_time, const ActorId& actor_id);

  // Method to determine if a vehicle is stuck at a place for too long.
  bool IsVehicleStuck(const ActorId& actor_id);

  // Removes an actor from traffic manager and performs clean up of associated data
  // from various stages tracking the said vehicle.
  void RemoveActor(const ActorId actor_id, const bool registered_actor);

public:
  ALSM(AtomicActorSet &registered_vehicles,
       BufferMapPtr &buffer_map_ptr,
       TrackTraffic &track_traffic,
       const Parameters &parameters,
       const cc::World &world,
       const LocalMapPtr &local_map,
       SimulationState &simulation_state,
       LocalizationStage &localization_stage,
       CollisionStage &collision_stage,
       TrafficLightStage &traffic_light_stage,
       MotionPlanStage &motion_plan_stage);

  void Update();

  void Reset();
};

} // namespace traffic_manager
} // namespace carla
