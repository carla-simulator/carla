// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#ifndef __TRAFFICMANAGERSERVER__
#define __TRAFFICMANAGERSERVER__

#include "carla/trafficmanager/TrafficManagerBase.h"

#include <memory.h>
#include <carla/client/Actor.h>
#include <carla/Version.h>
#include <carla/rpc/Server.h>

#include <vector>

using ActorPtr = carla::SharedPtr<carla::client::Actor>;

class TrafficManagerServer
{
public:

	TrafficManagerServer(const uint16_t RPCPort, carla::traffic_manager::TrafficManagerBase *_episode)
	: server(new rpc::server(RPCPort)), episode(_episode) {

	    // Binding a lambda function to the name "add".
		server->bind("register_vehicle", [=](std :: vector <carla::rpc::Actor> _actor_list) {
			std::vector<ActorPtr> actor_list;
			for (auto &&actor : _actor_list) {
				actor_list.emplace_back(carla::client::detail::ActorVariant(actor).Get(*(_episode->GetEpisodeProxy())));
			}
			episode->RegisterVehicles(actor_list);
	    	std::cout << "This is server side process ... of remote TM" << std::endl;
	    });

		server->async_run();
	}

	~TrafficManagerServer() {
		if(server) {
			delete server;
		}

		if(episode) {
			episode = nullptr;
		}
	}

	/// Server instance to
	rpc::server *server = nullptr;

	/// Traffic manager local instance to support server side APIs
	carla::traffic_manager::TrafficManagerBase *episode = nullptr;
};

#endif /* __TRAFFICMANAGERSERVER__ */

