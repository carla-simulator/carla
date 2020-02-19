// Copyright (c) 2019 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include <memory>
#include "carla/client/Actor.h"
#include "carla/client/detail/Simulator.h"
#include "carla/client/detail/EpisodeProxy.h"

#define MIN_TRY_COUNT       10
#define TM_DEFAULT_PORT     8000

namespace carla {
namespace traffic_manager {

using ActorPtr = carla::SharedPtr<carla::client::Actor>;

/// The function of this class is to integrate all the various stages of
/// the traffic manager appropriately using messengers.
class TrafficManagerBase {

public:
  /// To start the traffic manager.
  virtual void Start() = 0;

  /// To stop the traffic manager.
  virtual void Stop() = 0;

  /// Protected constructor for singleton lifecycle management.
  TrafficManagerBase(uint16_t port) : _port(port) {};

  /// Destructor.
  virtual ~TrafficManagerBase() {};

  /// This method registers a vehicle with the traffic manager.
  virtual void RegisterVehicles(const std::vector<ActorPtr> &actor_list) = 0;

  /// This method unregisters a vehicle from traffic manager.
  virtual void UnregisterVehicles(const std::vector<ActorPtr> &actor_list) = 0;

  /// Set target velocity specific to a vehicle.
  virtual void SetPercentageSpeedDifference(const ActorPtr &actor, const float percentage) = 0;

  /// Set global target velocity.
  virtual void SetGlobalPercentageSpeedDifference(float const percentage) = 0;

  /// Set collision detection rules between vehicles.
  virtual void SetCollisionDetection(const ActorPtr &reference_actor, const ActorPtr &other_actor, const bool detect_collision) = 0;

  /// Method to force lane change on a vehicle.
  /// Direction flag can be set to true for left and false for right.
  virtual void SetForceLaneChange(const ActorPtr &actor, const bool direction) = 0;

  /// Enable/disable automatic lane change on a vehicle.
  virtual void SetAutoLaneChange(const ActorPtr &actor, const bool enable) = 0;

  /// Method to specify how much distance a vehicle should maintain to
  /// the leading vehicle.
  virtual void SetDistanceToLeadingVehicle(const ActorPtr &actor, const float distance) = 0;

  /// Method to specify the % chance of ignoring collisions with any walker.
  virtual void SetPercentageIgnoreWalkers(const ActorPtr &actor, const float perc) = 0;

  /// Method to specify the % chance of ignoring collisions with any vehicle.
  virtual void SetPercentageIgnoreVehicles(const ActorPtr &actor, const float perc) = 0;

  /// Method to specify the % chance of running any traffic light.
  virtual void SetPercentageRunningLight(const ActorPtr &actor, const float perc) = 0;

  /// Method to specify the % chance of running any traffic sign.
  virtual void SetPercentageRunningSign(const ActorPtr &actor, const float perc) = 0;

  /// Method to switch traffic manager into synchronous execution.
  virtual void SetSynchronousMode(bool mode) = 0;

  /// Method to set Tick timeout for synchronous execution.
  virtual void SetSynchronousModeTimeOutInMiliSecond(double time) = 0;

  /// Method to provide synchronous tick
  virtual bool SynchronousTick() = 0;

  /// Method to reset all traffic lights.
  virtual void ResetAllTrafficLights() = 0;

  /// Get carla episode information
  virtual  carla::client::detail::EpisodeProxy& GetEpisodeProxy() = 0;

  uint16_t port() const {
    return _port;
  }

  bool IsServer() const {
    return _is_server;
  }

protected:

  uint16_t _port = TM_DEFAULT_PORT;
  bool _is_server = false;

};

} // namespace traffic_manager
} // namespace carla
