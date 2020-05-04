
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
#include "carla/trafficmanager/SimulationState.h"

#include "carla/trafficmanager/LocalizationStage.h"
#include "carla/trafficmanager/CollisionStage.h"
#include "carla/trafficmanager/TrafficLightStage.h"
#include "carla/trafficmanager/MotionPlanStage.h"

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

class ALSM {

private:
  AtomicActorSet &registered_vehicles;
  ActorMap unregistered_actors;
  BufferMapPtr &buffer_map_ptr;
  IdleTimeMap idle_time;
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
  double elapsed_last_actor_destruction;
  cc::Timestamp current_timestamp;

  void UpdateIdleTime(std::pair<ActorId, double>& max_idle_time, const ActorId& actor_id);

  bool IsVehicleStuck(const ActorId& actor_id);

  void RemoveActor(const ActorId actor_id, const bool registered_actor);

  void Reset();

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
};

} // namespace traffic_manager
} // namespace carla
