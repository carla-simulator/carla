// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#ifndef __TRAFFICMANAGERCLIENT__
#define __TRAFFICMANAGERCLIENT__

#include "carla/client/Actor.h"
#include <rpc/client.h>

#define TM_TIMEOUT			2000 // In ms
#define TM_DEFAULT_PORT		8000 // TM_SERVER_PORT

/// Provides communication with the rpc of TrafficManagerServer
class TrafficManagerClient {

public:

	TrafficManagerClient(const TrafficManagerClient &) = default;
	TrafficManagerClient(TrafficManagerClient &&) = default;

	TrafficManagerClient &operator=(const TrafficManagerClient &) = default;
	TrafficManagerClient &operator=(TrafficManagerClient &&) = default;

	/// Empty constructor
	TrafficManagerClient() : tmhost(""), tmport(TM_DEFAULT_PORT) {}

	/// Parametric constructor to initialize the parameters
	TrafficManagerClient(const std::string &_host, const uint16_t &_port)
		: tmhost(_host), tmport(_port) {}

	/// Destructor
	~TrafficManagerClient() {};

	/// Set parameters
	void setServerDetails(const std::string &_host, const uint16_t &_port) {
		 tmhost = _host;
		 tmport = _port;
	}

	/// Get parametrs
	void getServerDetails(std::string &_host, uint16_t &_port) {
		_host = tmhost;
		_port = tmport;
	}

	/// Register vehicles to remote TM server via RPC client
	void RegisterVehicle
	(const std::vector<carla::rpc::Actor> &actor_list) {
		rpc::client rpc_client(tmhost, tmport);
		rpc_client.set_timeout(TM_TIMEOUT);
		rpc_client.call("register_vehicle", std::move(actor_list));
	}

	/// Unregister vehicles to remote TM server via RPC client
	void UnregisterVehicle
	(const std::vector<carla::rpc::Actor> &actor_list) {
		rpc::client rpc_client(tmhost, tmport);
		rpc_client.set_timeout(TM_TIMEOUT);
		rpc_client.call("unregister_vehicle", std::move(actor_list));
	}

	/// Set target velocity specific to a vehicle.
	void SetPercentageSpeedDifference(const carla::rpc::Actor &_actor, const float percentage) {
		rpc::client rpc_client(tmhost, tmport);
		rpc_client.set_timeout(TM_TIMEOUT);
		rpc_client.call("set_percentage_speed_difference", std::move(_actor), percentage);
	}

	/// Set global target velocity.
	void SetGlobalPercentageSpeedDifference(const float percentage) {
		rpc::client rpc_client(tmhost, tmport);
		rpc_client.set_timeout(TM_TIMEOUT);
		rpc_client.call("set_global_percentage_speed_difference", percentage);
	}

	/// Set collision detection rules between vehicles.
	void SetCollisionDetection
		( const carla::rpc::Actor &reference_actor
		, const carla::rpc::Actor &other_actor
		, const bool detect_collision) {
		rpc::client rpc_client(tmhost, tmport);
		rpc_client.set_timeout(TM_TIMEOUT);
		rpc_client.call("set_collision_detection", reference_actor, other_actor, detect_collision);
	}

	/// Method to force lane change on a vehicle.
	/// Direction flag can be set to true for left and false for right.
	void SetForceLaneChange(const carla::rpc::Actor &actor, const bool direction) {
		rpc::client rpc_client(tmhost, tmport);
		rpc_client.set_timeout(TM_TIMEOUT);
		rpc_client.call("set_force_lane_change", actor, direction);
	}

	/// Enable / disable automatic lane change on a vehicle.
	void SetAutoLaneChange(const carla::rpc::Actor &actor, const bool enable) {
		rpc::client rpc_client(tmhost, tmport);
		rpc_client.set_timeout(TM_TIMEOUT);
		rpc_client.call("set_auto_lane_change", actor, enable);
	}

	/// Method to specify how much distance a vehicle should maintain to
	/// the leading vehicle.
	void SetDistanceToLeadingVehicle(const carla::rpc::Actor &actor, const float distance) {
		rpc::client rpc_client(tmhost, tmport);
		rpc_client.set_timeout(TM_TIMEOUT);
		rpc_client.call("set_distance_to_leading_vehicle", actor, distance);
	}

	/// Method to specify the % chance of ignoring collisions with other actors
	void SetPercentageIgnoreActors(const carla::rpc::Actor &actor, const float percentage) {
		rpc::client rpc_client(tmhost, tmport);
		rpc_client.set_timeout(TM_TIMEOUT);
		rpc_client.call("set_percentage_ignore_actors", actor, percentage);
	}

	/// Method to specify the % chance of running a red light
	void SetPercentageRunningLight(const carla::rpc::Actor &actor, const float percentage) {
		rpc::client rpc_client(tmhost, tmport);
		rpc_client.set_timeout(TM_TIMEOUT);
		rpc_client.call("set_percentage_running_light", actor, percentage);
	}

	/// Method to switch traffic manager into synchronous execution.
	void SetSynchronousMode(const bool mode) {
		rpc::client rpc_client(tmhost, tmport);
		rpc_client.set_timeout(TM_TIMEOUT);
		rpc_client.call("set_synchronous_mode", mode);
	}

	/// Method to set Tick timeout for synchronous execution.
	void SetSynchronousModeTimeOutInMiliSecond(const double time) {
		rpc::client rpc_client(tmhost, tmport);
		rpc_client.set_timeout(TM_TIMEOUT);
		rpc_client.call("set_synchronous_mode_timeout_in_milisecond", time);
	}

	/// Method to reset all traffic lights.
	bool SynchronousTick() {
		rpc::client rpc_client(tmhost, tmport);
		rpc_client.set_timeout(TM_TIMEOUT);
		auto rs = rpc_client.call("synchronous_tick").as<bool>();
		return rs;
	}

	/// Method to reset all traffic lights.
	void ResetAllTrafficLights() {
		rpc::client rpc_client(tmhost, tmport);
		rpc_client.set_timeout(TM_TIMEOUT);
		rpc_client.call("reset_all_traffic_lights");
	}

	void HealthCheckRemoteTM() {
		rpc::client rpc_client(tmhost, tmport);
		rpc_client.set_timeout(TM_TIMEOUT);
		rpc_client.call("health_check_remote_TM");
	}
private:
	std::string tmhost;
	uint16_t    tmport;
};

#endif /* __TRAFFICMANAGERCLIENT__ */

