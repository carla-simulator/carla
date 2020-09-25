
#pragma once

#include <memory>

#include "carla/client/ActorList.h"
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
#include "carla/trafficmanager/RandomGenerator.h"
#include "carla/trafficmanager/SimulationState.h"
#include "carla/trafficmanager/TrafficLightStage.h"

namespace carla {
namespace traffic_manager {

using namespace constants::HybridMode;
using namespace constants::VehicleRemoval;

namespace chr = std::chrono;
namespace cg = carla::geom;
namespace cc = carla::client;

using ActorList = carla::SharedPtr<cc::ActorList>;
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
  BufferMap &buffer_map;
  // Structure keeping track of duration of vehicles stuck in a location.
  IdleTimeMap idle_time;
  // Structure containing vehicles with attribute role_name with value hero.
  ActorMap hero_actors;
  TrackTraffic &track_traffic;
  // Array of vehicles marked by stages for removal.
  std::vector<ActorId>& marked_for_removal;
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
  // Random devices.
  RandomGeneratorMap &random_devices;

  // Updates the duration for which a registered vehicle is stuck at a location.
  void UpdateIdleTime(std::pair<ActorId, double>& max_idle_time, const ActorId& actor_id);

  // Method to determine if a vehicle is stuck at a place for too long.
  bool IsVehicleStuck(const ActorId& actor_id);

  using ActorVector = std::vector<ActorPtr>;
  // Method to identify actors newly spawned in the simulation since last tick.
  ActorVector IdentifyNewActors(const ActorList &actor_list);

  using DestroyeddActors = std::pair<ActorIdSet, ActorIdSet>;
  // Method to identify actors deleted in the last frame.
  // Arrays of registered and unregistered actors are returned separately.
  DestroyeddActors IdentifyDestroyedActors(const ActorList &actor_list);

  using IdleInfo = std::pair<ActorId, double>;
  void UpdateRegisteredActorsData(const bool hybrid_physics_mode, IdleInfo &max_idle_time);

  void UpdateUnregisteredActorsData();

public:
  ALSM(AtomicActorSet &registered_vehicles,
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
       RandomGeneratorMap &random_devices);

  void Update();

  // Removes an actor from traffic manager and performs clean up of associated data
  // from various stages tracking the said vehicle.
  void RemoveActor(const ActorId actor_id, const bool registered_actor);

  void Reset();
};

} // namespace traffic_manager
} // namespace carla
