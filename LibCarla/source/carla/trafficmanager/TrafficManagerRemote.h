// Copyright (c) 2019 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#ifndef __TRAFFICMANAGERREMOTE__
#define __TRAFFICMANAGERREMOTE__

#include <memory>
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

protected:

	/// To start the TrafficManager.
	void Start() {};

	/// To stop the TrafficManager.
	void Stop() {};

public:

	/// Constructor store remote location information
	TrafficManagerRemote
		( const std::pair<std::string, uint16_t> &_serverTM
		, carla::client::detail::EpisodeProxy &episodeProxy);

	/// Destructor.
	virtual ~TrafficManagerRemote();


	/// This method registers a vehicle with the traffic manager.
	void RegisterVehicles(const std::vector<ActorPtr> &actor_list);

	/// This method unregisters a vehicle from traffic manager.
	void UnregisterVehicles(const std::vector<ActorPtr> &actor_list);

	/// This method kills a vehicle. (Not working right now)
	/// void DestroyVehicle(const ActorPtr &actor);

	/// Set target velocity specific to a vehicle.
	void SetPercentageSpeedDifference(const ActorPtr &actor, const float percentage);

	/// Set global target velocity.
	void SetGlobalPercentageSpeedDifference(float const percentage);

	/// Set collision detection rules between vehicles.
	void SetCollisionDetection
		( const ActorPtr &reference_actor
		, const ActorPtr &other_actor
		, const bool detect_collision);

	/// Method to force lane change on a vehicle.
	/// Direction flag can be set to true for left and false for right.
	void SetForceLaneChange(const ActorPtr &actor, const bool direction);

	/// Enable / disable automatic lane change on a vehicle.
	void SetAutoLaneChange(const ActorPtr &actor, const bool enable);

	/// Method to specify how much distance a vehicle should maintain to
	/// the leading vehicle.
	void SetDistanceToLeadingVehicle(const ActorPtr &actor, const float distance);

	/// Method to specify the % chance of ignoring collisions with other actors
	void SetPercentageIgnoreActors(const ActorPtr &actor, const float perc);

	/// Method to specify the % chance of running a red light
	void SetPercentageRunningLight(const ActorPtr &actor, const float perc);

	/// Method to switch traffic manager into synchronous execution.
	void SetSynchronousMode(bool mode);

	/// Method to set Tick timeout for synchronous execution.
	void SetSynchronousModeTimeOutInMiliSecond(double time);

	/// Method to provide synchronous tick
	bool SynchronousTick();

	/// Method to reset all traffic lights.
	void ResetAllTrafficLights();

    /// Get carla episode information
    carla::client::detail::EpisodeProxy& GetEpisodeProxy();

    /// Call to remote server to check its health
    void HealthCheckRemoteTM();

private:

	/// Remote client using IP & port information it connects to remote RPC TM Server
	TrafficManagerClient client;

    /// Carla's client connection object.
    carla::client::detail::EpisodeProxy episodeProxyTM;
};

} // namespace traffic_manager
} // namespace carla

#endif /*__TRAFFICMANAGERREMOTE__ */

