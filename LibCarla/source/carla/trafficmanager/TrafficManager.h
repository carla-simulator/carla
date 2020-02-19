// Copyright (c) 2019 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include <algorithm>
#include <memory>
#include <random>
#include <unordered_set>
#include <vector>

#if _WIN32
  #include <winsock2.h>   ///< socket
  #include <Ws2tcpip.h>
#else
  #include <sys/socket.h> ///< socket
  #include <netinet/in.h> ///< sockaddr_in
  #include <arpa/inet.h>  ///< getsockname
  #include <unistd.h>     ///< close
#endif
#include "carla/client/Actor.h"
#include "carla/client/BlueprintLibrary.h"
#include "carla/client/Map.h"
#include "carla/client/World.h"
#include "carla/client/TimeoutException.h"

#include "carla/geom/Transform.h"
#include "carla/Logging.h"
#include "carla/Memory.h"

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

private:
  /// Pointer to hold representative TM class
  static std::unique_ptr<TrafficManagerBase> singleton_pointer;

public:

  /// Public release method to clear allocated data
  static void Release();

  static void Reset();

  /// Private constructor for singleton life cycle management.
  explicit TrafficManager(
    carla::client::detail::EpisodeProxy episodeProxy,
    uint16_t port = TM_DEFAULT_PORT);

  TrafficManager(const TrafficManager &) = default;
  TrafficManager(TrafficManager &&) = default;

  TrafficManager &operator=(const TrafficManager &) = default;
  TrafficManager &operator=(TrafficManager &&) = default;

  /// This method registers a vehicle with the traffic manager.
  void RegisterVehicles(const std::vector<ActorPtr> &actor_list) {
    DEBUG_ASSERT(singleton_pointer != nullptr);
    singleton_pointer->RegisterVehicles(actor_list);
  }

  /// This method unregisters a vehicle from traffic manager.
  void UnregisterVehicles(const std::vector<ActorPtr> &actor_list) {
    DEBUG_ASSERT(singleton_pointer != nullptr);
    singleton_pointer->UnregisterVehicles(actor_list);
  }

  /// Set target velocity specific to a vehicle.
  void SetPercentageSpeedDifference(const ActorPtr &actor, const float percentage) {
    DEBUG_ASSERT(singleton_pointer != nullptr);
    singleton_pointer->SetPercentageSpeedDifference(actor, percentage);
  }

  /// Set global target velocity.
  void SetGlobalPercentageSpeedDifference(float const percentage){
    DEBUG_ASSERT(singleton_pointer != nullptr);
    singleton_pointer->SetGlobalPercentageSpeedDifference(percentage);
  }

  /// Set collision detection rules between vehicles.
  void SetCollisionDetection(const ActorPtr &reference_actor, const ActorPtr &other_actor, const bool detect_collision) {
    DEBUG_ASSERT(singleton_pointer != nullptr);
    singleton_pointer->SetCollisionDetection(reference_actor, other_actor, detect_collision);
  }

  /// Method to force lane change on a vehicle.
  /// Direction flag can be set to true for left and false for right.
  void SetForceLaneChange(const ActorPtr &actor, const bool direction) {
    DEBUG_ASSERT(singleton_pointer != nullptr);
    singleton_pointer->SetForceLaneChange(actor, direction);
  }

  /// Enable/disable automatic lane change on a vehicle.
  void SetAutoLaneChange(const ActorPtr &actor, const bool enable) {
    DEBUG_ASSERT(singleton_pointer != nullptr);
    singleton_pointer->SetAutoLaneChange(actor, enable);
  }

  /// Method to specify how much distance a vehicle should maintain to
  /// the leading vehicle.
  void SetDistanceToLeadingVehicle(const ActorPtr &actor, const float distance) {
    DEBUG_ASSERT(singleton_pointer != nullptr);
    singleton_pointer->SetDistanceToLeadingVehicle(actor, distance);
  }

  /// Method to specify the % chance of ignoring collisions with any walker.
  void SetPercentageIgnoreWalkers(const ActorPtr &actor, const float perc) {
    DEBUG_ASSERT(singleton_pointer != nullptr);
    singleton_pointer->SetPercentageIgnoreWalkers(actor, perc);
  }

  /// Method to specify the % chance of ignoring collisions with any vehicle.
  void SetPercentageIgnoreVehicles(const ActorPtr &actor, const float perc) {
    DEBUG_ASSERT(singleton_pointer != nullptr);
    singleton_pointer->SetPercentageIgnoreVehicles(actor, perc);
  }

  /// Method to specify the % chance of running a sign.
  void SetPercentageRunningSign(const ActorPtr &actor, const float perc) {
    DEBUG_ASSERT(singleton_pointer != nullptr);
    singleton_pointer->SetPercentageRunningSign(actor, perc);
  }

  /// Method to specify the % chance of running a light.
  void SetPercentageRunningLight(const ActorPtr &actor, const float perc){
    DEBUG_ASSERT(singleton_pointer != nullptr);
    singleton_pointer->SetPercentageRunningLight(actor, perc);
  }

  /// Method to reset all traffic lights.
  void ResetAllTrafficLights() {
    DEBUG_ASSERT(singleton_pointer != nullptr);
    singleton_pointer->ResetAllTrafficLights();
  }

  /// Method to switch traffic manager into synchronous execution.
  void SetSynchronousMode(bool mode) {
    DEBUG_ASSERT(singleton_pointer != nullptr);
    singleton_pointer->SetSynchronousMode(mode);
  }

  /// Method to set tick timeout for synchronous execution.
  void SetSynchronousModeTimeOutInMiliSecond(double time) {
    DEBUG_ASSERT(singleton_pointer != nullptr);
    singleton_pointer->SetSynchronousModeTimeOutInMiliSecond(time);
  }

  /// Method to provide synchronous tick.
  bool SynchronousTick() {
    DEBUG_ASSERT(singleton_pointer != nullptr);
    return singleton_pointer->SynchronousTick();
  }

protected:

  static void CreateTrafficManagerServer(
    carla::client::detail::EpisodeProxy episodeProxy,
    uint16_t port);


  static bool CreateTrafficManagerClient(
    carla::client::detail::EpisodeProxy episodeProxy,
    uint16_t port);

};

} // namespace traffic_manager
} // namespace carla
