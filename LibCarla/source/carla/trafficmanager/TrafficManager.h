// Copyright (c) 2020 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include <algorithm>
#include <memory>
#include <mutex>
#include <random>
#include <unordered_set>
#include <vector>

#include "carla/client/Actor.h"
#include "carla/client/BlueprintLibrary.h"
#include "carla/client/Map.h"
#include "carla/client/World.h"
#include "carla/client/TimeoutException.h"

#include "carla/geom/Transform.h"
#include "carla/Logging.h"
#include "carla/Memory.h"
#include "carla/Sockets.h"

#include "carla/trafficmanager/AtomicActorSet.h"
#include "carla/trafficmanager/AtomicMap.h"
#include "carla/trafficmanager/BatchControlStage.h"
#include "carla/trafficmanager/CollisionStage.h"
#include "carla/trafficmanager/InMemoryMap.h"
#include "carla/trafficmanager/LocalizationStage.h"
#include "carla/trafficmanager/MotionPlannerStage.h"
#include "carla/trafficmanager/Parameters.h"
#include "carla/trafficmanager/TrafficLightStage.h"

#include "carla/trafficmanager/TrafficManagerBase.h"
#include "carla/trafficmanager/TrafficManagerLocal.h"
#include "carla/trafficmanager/TrafficManagerRemote.h"

#define INVALID_INDEX           -1
#define IP_DATA_BUFFER_SIZE     80

namespace carla {
namespace traffic_manager {

using ActorPtr = carla::SharedPtr<carla::client::Actor>;

/// This class integrates all the various stages of
/// the traffic manager appropriately using messengers.
class TrafficManager {

public:
  /// Public constructor for singleton life cycle management.
  explicit TrafficManager(
    carla::client::detail::EpisodeProxy episode_proxy,
    uint16_t port = TM_DEFAULT_PORT);

  TrafficManager(const TrafficManager& other) {
    _port = other._port;
  }

  TrafficManager(TrafficManager &&) = default;

  TrafficManager &operator=(const TrafficManager &) = default;
  TrafficManager &operator=(TrafficManager &&) = default;

  static void Release();

  static void Reset();

  static void Tick();

  /// This method registers a vehicle with the traffic manager.
  void RegisterVehicles(const std::vector<ActorPtr> &actor_list) {
    TrafficManagerBase* tm_ptr = GetTM(_port);
    if(tm_ptr != nullptr){
      tm_ptr->RegisterVehicles(actor_list);
    }
  }

  /// This method unregisters a vehicle from traffic manager.
  void UnregisterVehicles(const std::vector<ActorPtr> &actor_list) {
    TrafficManagerBase* tm_ptr = GetTM(_port);
    if(tm_ptr != nullptr){
      tm_ptr->UnregisterVehicles(actor_list);
    }
  }

  /// Set a vehicle's % decrease in velocity with respect to the speed limit.
  /// If less than 0, it's a % increase.
  void SetPercentageSpeedDifference(const ActorPtr &actor, const float percentage) {
    TrafficManagerBase* tm_ptr = GetTM(_port);
    if(tm_ptr != nullptr){
      tm_ptr->SetPercentageSpeedDifference(actor, percentage);
    }
  }

  /// Set a global % decrease in velocity with respect to the speed limit.
  /// If less than 0, it's a % increase.
  void SetGlobalPercentageSpeedDifference(float const percentage){
    TrafficManagerBase* tm_ptr = GetTM(_port);
    if(tm_ptr != nullptr){
      tm_ptr->SetGlobalPercentageSpeedDifference(percentage);
    }
  }

  /// Method to set collision detection rules between vehicles.
  void SetCollisionDetection(const ActorPtr &reference_actor, const ActorPtr &other_actor, const bool detect_collision) {
    TrafficManagerBase* tm_ptr = GetTM(_port);
    if(tm_ptr != nullptr){
      tm_ptr->SetCollisionDetection(reference_actor, other_actor, detect_collision);
    }
  }

  /// Method to force lane change on a vehicle.
  /// Direction flag can be set to true for left and false for right.
  void SetForceLaneChange(const ActorPtr &actor, const bool direction) {
    TrafficManagerBase* tm_ptr = GetTM(_port);
    if(tm_ptr != nullptr){
      tm_ptr->SetForceLaneChange(actor, direction);
    }
  }

  /// Enable/disable automatic lane change on a vehicle.
  void SetAutoLaneChange(const ActorPtr &actor, const bool enable) {
    TrafficManagerBase* tm_ptr = GetTM(_port);
    if(tm_ptr != nullptr){
      tm_ptr->SetAutoLaneChange(actor, enable);
    }
  }

  /// Method to specify how much distance a vehicle should maintain to
  /// the leading vehicle.
  void SetDistanceToLeadingVehicle(const ActorPtr &actor, const float distance) {
    TrafficManagerBase* tm_ptr = GetTM(_port);
    if(tm_ptr != nullptr){
      tm_ptr->SetDistanceToLeadingVehicle(actor, distance);
    }
  }

  /// Method to specify the % chance of ignoring collisions with any walker.
  void SetPercentageIgnoreWalkers(const ActorPtr &actor, const float perc) {
    TrafficManagerBase* tm_ptr = GetTM(_port);
    if(tm_ptr != nullptr){
      tm_ptr->SetPercentageIgnoreWalkers(actor, perc);
    }
  }

  /// Method to specify the % chance of ignoring collisions with any vehicle.
  void SetPercentageIgnoreVehicles(const ActorPtr &actor, const float perc) {
    TrafficManagerBase* tm_ptr = GetTM(_port);
    if(tm_ptr != nullptr){
      tm_ptr->SetPercentageIgnoreVehicles(actor, perc);
    }
  }

  /// Method to specify the % chance of running a sign.
  void SetPercentageRunningSign(const ActorPtr &actor, const float perc) {
    TrafficManagerBase* tm_ptr = GetTM(_port);
    if(tm_ptr != nullptr){
      tm_ptr->SetPercentageRunningSign(actor, perc);
    }
  }

  /// Method to specify the % chance of running a light.
  void SetPercentageRunningLight(const ActorPtr &actor, const float perc){
    TrafficManagerBase* tm_ptr = GetTM(_port);
    if(tm_ptr != nullptr){
      tm_ptr->SetPercentageRunningLight(actor, perc);
    }
  }

  /// Method to reset all traffic lights.
  void ResetAllTrafficLights() {
    TrafficManagerBase* tm_ptr = GetTM(_port);
    if(tm_ptr != nullptr){
      tm_ptr->ResetAllTrafficLights();
    }
  }

  /// Method to switch traffic manager into synchronous execution.
  void SetSynchronousMode(bool mode) {
    TrafficManagerBase* tm_ptr = GetTM(_port);
    if(tm_ptr != nullptr){
      tm_ptr->SetSynchronousMode(mode);
    }
  }

  /// Method to set tick timeout for synchronous execution.
  void SetSynchronousModeTimeOutInMiliSecond(double time) {
    TrafficManagerBase* tm_ptr = GetTM(_port);
    if(tm_ptr != nullptr){
      tm_ptr->SetSynchronousModeTimeOutInMiliSecond(time);
    }
  }

  /// Method to provide synchronous tick.
  bool SynchronousTick() {
    TrafficManagerBase* tm_ptr = GetTM(_port);
    if(tm_ptr != nullptr){
      return tm_ptr->SynchronousTick();
    }
    return false;
  }

  /// Method to Set Global distance to Leading vehicle
  void SetGlobalDistanceToLeadingVehicle(const float distance) {
    TrafficManagerBase* tm_ptr = GetTM(_port);
    if(tm_ptr != nullptr){
      tm_ptr->SetGlobalDistanceToLeadingVehicle(distance);
    }
  }

  /// Method to set probabilistic preference to keep on the right lane.
  void SetKeepRightPercentage(const ActorPtr &actor, const float percentage) {
    TrafficManagerBase* tm_ptr = GetTM(_port);
    if(tm_ptr != nullptr){
      tm_ptr->SetKeepRightPercentage(actor, percentage);
    }
  }

private:

  void CreateTrafficManagerServer(
    carla::client::detail::EpisodeProxy episode_proxy,
    uint16_t port);


  bool CreateTrafficManagerClient(
    carla::client::detail::EpisodeProxy episode_proxy,
    uint16_t port);

  TrafficManagerBase* GetTM(uint16_t port) const {
    std::lock_guard<std::mutex> lock(_mutex);
    auto it = _tm_map.find(port);
    if (it != _tm_map.end()) {
      _mutex.unlock();
      return it->second.get();
    }
    return nullptr;
  }

  static std::map<uint16_t, std::unique_ptr<TrafficManagerBase>> _tm_map;
  static std::mutex _mutex;

  uint16_t _port = TM_DEFAULT_PORT;

};

} // namespace traffic_manager
} // namespace carla
