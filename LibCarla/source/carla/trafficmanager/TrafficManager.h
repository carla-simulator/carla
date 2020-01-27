// Copyright (c) 2019 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#ifndef __TRAFFICMANAGER__
#define __TRAFFICMANAGER__

#include <algorithm>
#include <memory>
#include <random>
#include <unordered_set>
#include <vector>

#include <sys/socket.h> ///< socket
#include <netinet/in.h> ///< sockaddr_in
#include <arpa/inet.h>  ///< getsockname
#include <unistd.h>     ///< close

#include "carla/client/Actor.h"
#include "carla/client/BlueprintLibrary.h"
#include "carla/client/Map.h"
#include "carla/client/World.h"
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

#define INVALID_INDEX				-1
#define IP_DATA_BUFFER_SIZE			80

namespace carla {
namespace traffic_manager {

using ActorPtr 	= carla::SharedPtr<carla::client::Actor>;

using TLS 		= carla::rpc::TrafficLightState;
using TLGroup 	= std::vector<carla::SharedPtr<carla::client::TrafficLight>>;

/// The function of this class is to integrate all the various stages of
/// the traffic manager appropriately using messengers.
class TrafficManager {

private:
	/// Pointer to hold representative TM class
	static std::unique_ptr<TrafficManagerBase> singleton_pointer;

public:

	/// Private constructor for singleton life cycle management.
	TrafficManager(carla::client::detail::EpisodeProxy &episodeProxy);

	/// Destructor.
	~TrafficManager() {};

	/// This method registers a vehicle with the traffic manager.
	void RegisterVehicles(const std::vector<ActorPtr> &actor_list) {
		if(singleton_pointer) {
			singleton_pointer->RegisterVehicles(actor_list);
		}
	}

	/// This method unregisters a vehicle from traffic manager.
	void UnregisterVehicles(const std::vector<ActorPtr> &actor_list) {
		if(singleton_pointer) {
			singleton_pointer->UnregisterVehicles(actor_list);
		}
	}



	/// Set target velocity specific to a vehicle.
	void SetPercentageSpeedDifference(const ActorPtr &actor, const float percentage) {
		if(singleton_pointer) {
			singleton_pointer->SetPercentageSpeedDifference(actor, percentage);
		}
	}

	/// Set global target velocity.
	void SetGlobalPercentageSpeedDifference(float const percentage){
		if(singleton_pointer) {
			singleton_pointer->SetGlobalPercentageSpeedDifference(percentage);
		}
	}

	/// Set collision detection rules between vehicles.
	void SetCollisionDetection
		( const ActorPtr &reference_actor
		, const ActorPtr &other_actor
		, const bool detect_collision) {
		if(singleton_pointer) {
			singleton_pointer->SetCollisionDetection(reference_actor, other_actor, detect_collision);
		}
	}

	/// Method to force lane change on a vehicle.
	/// Direction flag can be set to true for left and false for right.
	void SetForceLaneChange(const ActorPtr &actor, const bool direction) {
		if(singleton_pointer) {
			singleton_pointer->SetForceLaneChange(actor, direction);
		}
	}

	/// Enable / disable automatic lane change on a vehicle.
	void SetAutoLaneChange(const ActorPtr &actor, const bool enable) {
		if(singleton_pointer) {
			singleton_pointer->SetAutoLaneChange(actor, enable);
		}
	}

	/// Method to specify how much distance a vehicle should maintain to
	/// the leading vehicle.
	void SetDistanceToLeadingVehicle(const ActorPtr &actor, const float distance) {
		if(singleton_pointer) {
			singleton_pointer->SetDistanceToLeadingVehicle(actor, distance);
		}
	}

	/// Method to specify the % chance of ignoring collisions with other actors
	void SetPercentageIgnoreActors(const ActorPtr &actor, const float perc) {
		if(singleton_pointer) {
			singleton_pointer->SetPercentageIgnoreActors(actor, perc);
		}
	}

	/// Method to specify the % chance of running a red light
	void SetPercentageRunningLight(const ActorPtr &actor, const float perc){
		if(singleton_pointer) {
			singleton_pointer->SetPercentageRunningLight(actor, perc);
		}
	}

	/// Method to check if traffic lights are frozen.
	bool CheckAllFrozen(TLGroup tl_to_freeze) {
		if(singleton_pointer) {
			return singleton_pointer->CheckAllFrozen(tl_to_freeze);
		}
		return false;
	}

	/// Method to reset all traffic lights.
	void ResetAllTrafficLights() {
		if(singleton_pointer) {
			return singleton_pointer->ResetAllTrafficLights();
		}
	}
};

} // namespace traffic_manager
} // namespace carla

#endif /* __TRAFFICMANAGER__ */

