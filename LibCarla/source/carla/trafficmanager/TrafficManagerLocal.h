// Copyright (c) 2020 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include <atomic>
#include <chrono>
#include <mutex>
#include <thread>
#include <vector>

#include "carla/client/detail/EpisodeProxy.h"
#include "carla/client/TrafficLight.h"
#include "carla/client/World.h"
#include "carla/Memory.h"
#include "carla/rpc/Command.h"

#include "carla/trafficmanager/AtomicActorSet.h"
#include "carla/trafficmanager/InMemoryMap.h"
#include "carla/trafficmanager/Parameters.h"
#include "carla/trafficmanager/RandomGenerator.h"
#include "carla/trafficmanager/SimulationState.h"
#include "carla/trafficmanager/TrackTraffic.h"
#include "carla/trafficmanager/TrafficManagerBase.h"
#include "carla/trafficmanager/TrafficManagerServer.h"

#include "carla/trafficmanager/ALSM.h"
#include "carla/trafficmanager/LocalizationStage.h"
#include "carla/trafficmanager/CollisionStage.h"
#include "carla/trafficmanager/TrafficLightStage.h"
#include "carla/trafficmanager/MotionPlanStage.h"

namespace carla {
namespace traffic_manager {

namespace chr = std::chrono;

using namespace std::chrono_literals;

using TimePoint = chr::time_point<chr::system_clock, chr::nanoseconds>;
using TLGroup = std::vector<carla::SharedPtr<carla::client::TrafficLight>>;
using LocalMapPtr = std::shared_ptr<InMemoryMap>;
using constants::HybridMode::HYBRID_MODE_DT;

/// The function of this class is to integrate all the various stages of
/// the traffic manager appropriately using messengers.
class TrafficManagerLocal : public TrafficManagerBase {

private:
  /// PID controller parameters.
  std::vector<float> longitudinal_PID_parameters;
  std::vector<float> longitudinal_highway_PID_parameters;
  std::vector<float> lateral_PID_parameters;
  std::vector<float> lateral_highway_PID_parameters;
  /// CARLA client connection object.
  carla::client::detail::EpisodeProxy episode_proxy;
  /// CARLA client and object.
  cc::World world;
  /// Set of all actors registered with traffic manager.
  AtomicActorSet registered_vehicles;
  /// State counter to track changes in registered actors.
  int registered_vehicles_state;
  /// List of vehicles registered with the traffic manager in
  /// current update cycle.
  std::vector<ActorId> vehicle_id_list;
  /// Pointer to local map cache.
  LocalMapPtr local_map;
  /// Structures to hold waypoint buffers for all vehicles.
  BufferMap buffer_map;
  /// Object for tracking paths of the traffic vehicles.
  TrackTraffic track_traffic;
  /// Type containing the current state of all actors involved in the simulation.
  SimulationState simulation_state;
  /// Time instance used to calculate dt in asynchronous mode.
  TimePoint previous_update_instance;
  /// Parameterization object.
  Parameters parameters;
  /// Array to hold output data of localization stage.
  LocalizationFrame localization_frame;
  /// Array to hold output data of collision avoidance.
  CollisionFrame collision_frame;
  /// Array to hold output data of traffic light response.
  TLFrame tl_frame;
  /// Array to hold output data of motion planning.
  ControlFrame control_frame;
  /// Variable to keep track of currently reserved array space for frames.
  uint64_t current_reserved_capacity {0u};
  /// Various stages representing core operations of traffic manager.
  LocalizationStage localization_stage;
  CollisionStage collision_stage;
  TrafficLightStage traffic_light_stage;
  MotionPlanStage motion_plan_stage;
  VehicleLightStage vehicle_light_stage;
  ALSM alsm;
  /// Traffic manager server instance.
  TrafficManagerServer server;
  /// Switch to turn on / turn off traffic manager.
  std::atomic<bool> run_traffic_manger{true};
  /// Flags to signal step begin and end.
  std::atomic<bool> step_begin{false};
  std::atomic<bool> step_end{false};
  /// Mutex for progressing synchronous execution.
  std::mutex step_execution_mutex;
  /// Condition variables for progressing synchronous execution.
  std::condition_variable step_begin_trigger;
  std::condition_variable step_end_trigger;
  /// Single worker thread for sequential execution of sub-components.
  std::unique_ptr<std::thread> worker_thread;
  /// Structure holding random devices per vehicle.
  RandomGeneratorMap random_devices;
  /// Randomization seed.
  uint64_t seed {static_cast<uint64_t>(time(NULL))};
  bool is_custom_seed {false};
  std::vector<ActorId> marked_for_removal;
  /// Mutex to prevent vehicle registration during frame array re-allocation.
  std::mutex registration_mutex;

  /// Method to check if all traffic lights are frozen in a group.
  bool CheckAllFrozen(TLGroup tl_to_freeze);

public:
  /// Private constructor for singleton lifecycle management.
  TrafficManagerLocal(std::vector<float> longitudinal_PID_parameters,
                      std::vector<float> longitudinal_highway_PID_parameters,
                      std::vector<float> lateral_PID_parameters,
                      std::vector<float> lateral_highway_PID_parameters,
                      float perc_decrease_from_limit,
                      cc::detail::EpisodeProxy &episode_proxy,
                      uint16_t &RPCportTM);

  /// Destructor.
  virtual ~TrafficManagerLocal();

  /// Method to setup InMemoryMap.
  void SetupLocalMap();

  /// To start the TrafficManager.
  void Start();

  /// Initiates thread to run the TrafficManager sequentially.
  void Run();

  /// To stop the TrafficManager.
  void Stop();

  /// To release the traffic manager.
  void Release();

  /// To reset the traffic manager.
  void Reset();

  /// This method registers a vehicle with the traffic manager.
  void RegisterVehicles(const std::vector<ActorPtr> &actor_list);

  /// This method unregisters a vehicle from traffic manager.
  void UnregisterVehicles(const std::vector<ActorPtr> &actor_list);

  /// Method to set a vehicle's % decrease in velocity with respect to the speed limit.
  /// If less than 0, it's a % increase.
  void SetPercentageSpeedDifference(const ActorPtr &actor, const float percentage);

  /// Methos to set a global % decrease in velocity with respect to the speed limit.
  /// If less than 0, it's a % increase.
  void SetGlobalPercentageSpeedDifference(float const percentage);

  /// Method to set the automatic management of the vehicle lights
  void SetUpdateVehicleLights(const ActorPtr &actor, const bool do_update);

  /// Method to set collision detection rules between vehicles.
  void SetCollisionDetection(const ActorPtr &reference_actor, const ActorPtr &other_actor, const bool detect_collision);

  /// Method to force lane change on a vehicle.
  /// Direction flag can be set to true for left and false for right.
  void SetForceLaneChange(const ActorPtr &actor, const bool direction);

  /// Enable/disable automatic lane change on a vehicle.
  void SetAutoLaneChange(const ActorPtr &actor, const bool enable);

  /// Method to specify how much distance a vehicle should maintain to
  /// the leading vehicle.
  void SetDistanceToLeadingVehicle(const ActorPtr &actor, const float distance);

  /// Method to specify the % chance of ignoring collisions with any walker.
  void SetPercentageIgnoreWalkers(const ActorPtr &actor, const float perc);

  /// Method to specify the % chance of ignoring collisions with any vehicle.
  void SetPercentageIgnoreVehicles(const ActorPtr &actor, const float perc);

  /// Method to specify the % chance of running any traffic light.
  void SetPercentageRunningLight(const ActorPtr &actor, const float perc);

  /// Method to specify the % chance of running any traffic sign.
  void SetPercentageRunningSign(const ActorPtr &actor, const float perc);

  /// Method to switch traffic manager into synchronous execution.
  void SetSynchronousMode(bool mode);

  /// Method to set Tick timeout for synchronous execution.
  void SetSynchronousModeTimeOutInMiliSecond(double time);

  /// Method to provide synchronous tick.
  bool SynchronousTick();

  /// Get CARLA episode information.
  carla::client::detail::EpisodeProxy &GetEpisodeProxy();

  /// Get list of all registered vehicles.
  std::vector<ActorId> GetRegisteredVehiclesIDs();

  /// Method to specify how much distance a vehicle should maintain to
  /// the Global leading vehicle.
  void SetGlobalDistanceToLeadingVehicle(const float distance);

  /// Method to set % to keep on the right lane.
  void SetKeepRightPercentage(const ActorPtr &actor, const float percentage);

  /// Method to set % to randomly do a left lane change.
  void SetRandomLeftLaneChangePercentage(const ActorPtr &actor, const float percentage);

  /// Method to set % to randomly do a right lane change.
  void SetRandomRightLaneChangePercentage(const ActorPtr &actor, const float percentage);

  /// Method to set hybrid physics mode.
  void SetHybridPhysicsMode(const bool mode_switch);

  /// Method to set hybrid physics radius.
  void SetHybridPhysicsRadius(const float radius);

  /// Method to set randomization seed.
  void SetRandomDeviceSeed(const uint64_t _seed);

  /// Method to set Open Street Map mode.
  void SetOSMMode(const bool mode_switch);

  /// Method to set our own imported path.
  void SetCustomPath(const ActorPtr &actor, const Path path, const bool empty_buffer);

  /// Method to remove a list of points.
  void RemoveUploadPath(const ActorId &actor_id, const bool remove_path);

  /// Method to update an already set list of points.
  void UpdateUploadPath(const ActorId &actor_id, const Path path);

  /// Method to set our own imported route.
  void SetImportedRoute(const ActorPtr &actor, const Route route, const bool empty_buffer);

  /// Method to remove a route.
  void RemoveImportedRoute(const ActorId &actor_id, const bool remove_path);

  /// Method to update an already set route.
  void UpdateImportedRoute(const ActorId &actor_id, const Route route);

  /// Method to set automatic respawn of dormant vehicles.
  void SetRespawnDormantVehicles(const bool mode_switch);

  /// Method to set boundaries to respawn of dormant vehicles.
  void SetBoundariesRespawnDormantVehicles(const float lower_bound, const float upper_bound);

  /// Method to set limits for boundaries when respawning dormant vehicles.
  void SetMaxBoundaries(const float lower, const float upper);

  /// Method to get the vehicle's next action.
  Action GetNextAction(const ActorId &actor_id);

  /// Method to get the vehicle's action buffer.
  ActionBuffer GetActionBuffer(const ActorId &actor_id);

  void ShutDown() {};
};

} // namespace traffic_manager
} // namespace carla
