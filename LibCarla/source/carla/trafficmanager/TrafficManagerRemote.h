// Copyright (c) 2020 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include <condition_variable>
#include <memory>
#include <mutex>
#include <vector>

#include "carla/client/Actor.h"
#include "carla/client/detail/Simulator.h"
#include "carla/client/detail/EpisodeProxy.h"
#include "carla/trafficmanager/TrafficManagerBase.h"
#include "carla/trafficmanager/TrafficManagerClient.h"

namespace carla {
namespace traffic_manager {

using ActorPtr = carla::SharedPtr<carla::client::Actor>;
using TLS = carla::rpc::TrafficLightState;
using TLGroup = std::vector<carla::SharedPtr<carla::client::TrafficLight>>;

/// The function of this class is to integrate all the various stages of
/// the traffic manager appropriately using messengers.
class TrafficManagerRemote : public TrafficManagerBase {

public:

  /// To start the TrafficManager.
  void Start();

  /// To stop the TrafficManager.
  void Stop();

  /// To release the traffic manager.
  void Release();

  /// To reset the traffic manager.
  void Reset();

  /// Constructor store remote location information.
  TrafficManagerRemote(const std::pair<std::string, uint16_t> &_serverTM, carla::client::detail::EpisodeProxy &episodeProxy);

  /// Destructor.
  virtual ~TrafficManagerRemote();

  /// This method registers a vehicle with the traffic manager.
  void RegisterVehicles(const std::vector<ActorPtr> &actor_list);

  /// This method unregisters a vehicle from traffic manager.
  void UnregisterVehicles(const std::vector<ActorPtr> &actor_list);

  /// Method to set a vehicle's % decrease in velocity with respect to the speed limit.
  /// If less than 0, it's a % increase.
  void SetPercentageSpeedDifference(const ActorPtr &actor, const float percentage);

  /// Method to set a global % decrease in velocity with respect to the speed limit.
  /// If less than 0, it's a % increase.
  void SetGlobalPercentageSpeedDifference(float const percentage);

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

  /// Method to specify Global Distance
  void SetGlobalDistanceToLeadingVehicle(const float distance);

  /// Method to specify the % chance of ignoring collisions with any walker.
  void SetPercentageIgnoreWalkers(const ActorPtr &actor, const float perc);

  /// Method to specify the % chance of ignoring collisions with any vehicle.
  void SetPercentageIgnoreVehicles(const ActorPtr &actor, const float perc);

  /// Method to specify the % chance of running any traffic light
  void SetPercentageRunningLight(const ActorPtr &actor, const float perc);

  /// Method to specify the % chance of running any traffic sign
  void SetPercentageRunningSign(const ActorPtr &actor, const float perc);

  /// Method to switch traffic manager into synchronous execution.
  void SetSynchronousMode(bool mode);

  /// Method to set Tick timeout for synchronous execution.
  void SetSynchronousModeTimeOutInMiliSecond(double time);

  /// Method to set probabilistic preference to keep on the right lane.
  void SetKeepRightPercentage(const ActorPtr &actor, const float percentage);

  /// Method to provide synchronous tick
  bool SynchronousTick();

  /// Method to reset all traffic lights.
  void ResetAllTrafficLights();

  /// Get CARLA episode information.
  carla::client::detail::EpisodeProxy& GetEpisodeProxy();

  /// Method to check server is alive or not.
  void HealthCheckRemoteTM();

private:

  /// Remote client using the IP and port information it connects to
  /// as remote RPC traffic manager server.
  TrafficManagerClient client;

  /// CARLA client connection object.
  carla::client::detail::EpisodeProxy episodeProxyTM;

  std::condition_variable _cv;

  std::mutex _mutex;

  bool _keep_alive = true;
};

} // namespace traffic_manager
} // namespace carla
