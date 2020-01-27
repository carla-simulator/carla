// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#ifndef __TRAFFICMANAGERCLIENT__
#define __TRAFFICMANAGERCLIENT__

#include "carla/client/Actor.h"
#include <rpc/client.h>

#define TM_TIMEOUT				5000 // In ms

/// Provides communication with the rpc of TrafficManagerServer
class TrafficManagerClient {

public:

	explicit TrafficManagerClient
	(const std::string &host, uint16_t port)
	: tmhost(host), tmport(port) {}

	~TrafficManagerClient() {};

	void RegisterVehicle
	(const std::vector<carla::rpc::Actor> &actor_list) {
		std::cout << "This is client side call ... to remote TM Before" << std::endl;
		rpc::client rpc_client(tmhost, tmport);
		rpc_client.set_timeout(TM_TIMEOUT);
		rpc_client.call("register_vehicle", std::move(actor_list));
	}

	void UnregisterVehicle
	(const std::vector<int> &actor_list) {
		rpc::client rpc_client(tmhost, tmport);
		rpc_client.set_timeout(TM_TIMEOUT);
		rpc_client.call("unregister_vehicle", std::move(actor_list));
	}
private:
	std::string tmhost;
	uint16_t    tmport;
};

#endif /* __TRAFFICMANAGERCLIENT__ */

