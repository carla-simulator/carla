// Copyright (c) 2019 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "carla/trafficmanager/TrafficManagerRemote.h"


#include <carla/client/Client.h>
#include "carla/client/TrafficLight.h"
#include "carla/client/ActorList.h"
#include "carla/client/DebugHelper.h"

namespace carla {
namespace traffic_manager {

/// Constructor store remote location information
TrafficManagerRemote :: TrafficManagerRemote
	( const std::pair<std::string, uint16_t> &_serverTM
	, carla::client::detail::EpisodeProxy &episodeProxy)
	: episodeProxyTM(episodeProxy) {

    /// Set server details
	client.setServerDetails(_serverTM.first, _serverTM.second);
}

/// Destructor.
TrafficManagerRemote :: ~TrafficManagerRemote() {}


/// This method registers a vehicle with the traffic manager.
void TrafficManagerRemote::RegisterVehicles(const std::vector<ActorPtr> &_actor_list) {
	/// Prepare rpc actor list
	std::vector<carla::rpc::Actor> actor_list;
	for (auto &&actor : _actor_list) {
		actor_list.emplace_back(actor->Serialize());
	}
	/// Call client method
	client.RegisterVehicle(actor_list);
}

/// This method unregisters a vehicle from traffic manager.
void TrafficManagerRemote::UnregisterVehicles(const std::vector<ActorPtr> &_actor_list) {
	/// Prepare rpc actor list
	std::vector<carla::rpc::Actor> actor_list;
	for (auto &&actor : _actor_list) {
		actor_list.emplace_back(actor->Serialize());
	}
	/// Call client method
	client.UnregisterVehicle(actor_list);
}

/// Set target velocity specific to a vehicle.
void TrafficManagerRemote::SetPercentageSpeedDifference(const ActorPtr &_actor, const float percentage) {
	/// Prepare rpc actor list
	carla::rpc::Actor actor(_actor->Serialize());

	/// Call client method
	client.SetPercentageSpeedDifference(actor, percentage);
}

/// Set global target velocity.
void TrafficManagerRemote::SetGlobalPercentageSpeedDifference(const float percentage) {
	/// Call client method
	client.SetGlobalPercentageSpeedDifference(percentage);
}

/// Set collision detection rules between vehicles.
void TrafficManagerRemote::SetCollisionDetection
	( const ActorPtr &_reference_actor
	, const ActorPtr &_other_actor
	, const bool detect_collision) {
	/// Prepare rpc actor list
	carla::rpc::Actor reference_actor(_reference_actor->Serialize());
	carla::rpc::Actor other_actor(_other_actor->Serialize());

	/// Call client method
	client.SetCollisionDetection(reference_actor, other_actor, detect_collision);
}

/// Method to force lane change on a vehicle.
/// Direction flag can be set to true for left and false for right.
void TrafficManagerRemote::SetForceLaneChange(const ActorPtr &_actor, const bool direction) {
	/// Prepare rpc actor list
	carla::rpc::Actor actor(_actor->Serialize());

	/// Call client method
	client.SetForceLaneChange(actor, direction);
}

/// Enable / disable automatic lane change on a vehicle.
void TrafficManagerRemote::SetAutoLaneChange(const ActorPtr &_actor, const bool enable) {
	/// Prepare rpc actor list
	carla::rpc::Actor actor(_actor->Serialize());

	/// Call client method
	client.SetAutoLaneChange(actor, enable);
}

/// Method to specify how much distance a vehicle should maintain to
/// the leading vehicle.
void TrafficManagerRemote::SetDistanceToLeadingVehicle(const ActorPtr &_actor, const float distance) {
	/// Prepare rpc actor list
	carla::rpc::Actor actor(_actor->Serialize());

	/// Call client method
	client.SetDistanceToLeadingVehicle(actor, distance);
}

/// Method to specify the % chance of ignoring collisions with other actors
void TrafficManagerRemote::SetPercentageIgnoreActors(const ActorPtr &_actor, const float percentage) {
	/// Prepare rpc actor list
	carla::rpc::Actor actor(_actor->Serialize());

	/// Call client method
	client.SetPercentageIgnoreActors(actor, percentage);
}

/// Method to specify the % chance of running a red light
void TrafficManagerRemote::SetPercentageRunningLight(const ActorPtr &_actor, const float percentage) {
	/// Prepare rpc actor list
	carla::rpc::Actor actor(_actor->Serialize());

	/// Call client method
	client.SetPercentageRunningLight(actor, percentage);
}

/// Method to reset all traffic lights.
void TrafficManagerRemote::ResetAllTrafficLights() {
	/// Call client method
	client.ResetAllTrafficLights();
}

/// Method to switch traffic manager into synchronous execution.
void TrafficManagerRemote::SetSynchronousMode(bool mode) {
	client.SetSynchronousMode(mode);
}

/// Method to set Tick timeout for synchronous execution.
void TrafficManagerRemote::SetSynchronousModeTimeOutInMiliSecond(double time) {
	client.SetSynchronousModeTimeOutInMiliSecond(time);
}

/// Method to provide synchronous tick
bool TrafficManagerRemote::SynchronousTick() {
	return client.SynchronousTick();
}

/// Method to reset all traffic lights.
void TrafficManagerRemote::HealthCheckRemoteTM() {
	/// Call client method
	client.HealthCheckRemoteTM();
}
/// Get carla episode information
carla::client::detail::EpisodeProxy& TrafficManagerRemote::GetEpisodeProxy() {
	return episodeProxyTM;
}

} // namespace traffic_manager
} // namespace carla
