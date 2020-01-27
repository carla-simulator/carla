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

void TrafficManagerRemote::RegisterVehicles(const std::vector<ActorPtr> &_actor_list) {
	std::vector<carla::rpc::Actor> actor_list;
	for (auto &&actor : _actor_list) {
		actor_list.emplace_back(actor->Serialize());
	}
	client->RegisterVehicle(actor_list);
	std :: cout << actor_list.size() << std :: endl;
}

void TrafficManagerRemote::UnregisterVehicles(const std::vector<ActorPtr> &actor_list) {
	std :: cout << actor_list.size() << std :: endl;
}

void TrafficManagerRemote::Start() {}

void TrafficManagerRemote::Stop() {}

void TrafficManagerRemote::SetPercentageSpeedDifference(const ActorPtr &actor, const float percentage) {
	std :: cout << actor->IsAlive() << percentage << std :: endl;
}

void TrafficManagerRemote::SetGlobalPercentageSpeedDifference(const float percentage) {
	std :: cout << percentage << std :: endl;
}

void TrafficManagerRemote::SetCollisionDetection(
		const ActorPtr &reference_actor,
		const ActorPtr &other_actor,
		const bool detect_collision) {
	std :: cout <<  reference_actor->IsAlive() << other_actor->IsAlive() << detect_collision << std :: endl;
}

void TrafficManagerRemote::SetForceLaneChange(const ActorPtr &actor, const bool direction) {
	std :: cout << actor->IsAlive() << direction << std :: endl;
}

void TrafficManagerRemote::SetAutoLaneChange(const ActorPtr &actor, const bool enable) {
	std :: cout << actor->IsAlive() << enable << std :: endl;
}

void TrafficManagerRemote::SetDistanceToLeadingVehicle(const ActorPtr &actor, const float distance) {
	std :: cout << actor->IsAlive() << distance << std :: endl;
}

void TrafficManagerRemote::SetPercentageIgnoreActors(const ActorPtr &actor, const float perc) {
	std :: cout << actor->IsAlive() << perc << std :: endl;
}

void TrafficManagerRemote::SetPercentageRunningLight(const ActorPtr &actor, const float perc) {
	std :: cout << actor->IsAlive() << perc << std :: endl;
}


bool TrafficManagerRemote::CheckAllFrozen(TLGroup tl_to_freeze) {
	std :: cout << tl_to_freeze.size() << std :: endl;
	return true;
}
void TrafficManagerRemote::ResetAllTrafficLights() {}

/// Get carla episode information
carla::client::detail::EpisodeProxy* TrafficManagerRemote::GetEpisodeProxy() {
	return &episodeProxyTM;
}

} // namespace traffic_manager
} // namespace carla
