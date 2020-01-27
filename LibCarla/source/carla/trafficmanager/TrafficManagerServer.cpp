// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "carla/trafficmanager/TrafficManagerServer.h"

#include <memory.h>
#include <carla/client/Actor.h>
#include <carla/Version.h>
#include <carla/rpc/Actor.h>
#include <carla/rpc/Server.h>

#include <vector>

template <typename T>
using R = carla::rpc::Response<T>;
using ActorPtr 	= carla::SharedPtr<carla::client::Actor>;

// =============================================================================
// -- TrafficManagerServer::FPimpl -----------------------------------------------
// =============================================================================
class TrafficManagerServer::FPimpl
{
public:

	FPimpl(const int RPCPort, carla::traffic_manager::TrafficManagerBase *episode)
	: Server(RPCPort)
	, Episode(episode) {
		BindActions();
	}

	/// Traffic manager client side server to support other clients
	carla::rpc::Server Server;

	/// Traffic manager local instance to support server side APIs
	carla::traffic_manager::TrafficManagerBase *Episode = nullptr;

private:

	void BindActions();
};

class ServerBinder
{
public:

	constexpr ServerBinder(const char *name, carla::rpc::Server &srv, bool sync)
	: _name(name)
	, _server(srv)
	, _sync(sync) {}

	template <typename FuncT>
	auto operator<<(FuncT func) {
		if (_sync) {
			_server.BindSync(_name, func);
		} else {
			_server.BindAsync(_name, func);
		}
		return func;
	}

private:

	/// Function name
	const char *_name;

	/// Server instance
	carla::rpc::Server &_server;

	/// Type of server call
	bool _sync;
};

#define BIND_SYNC(name)   auto name = ServerBinder(# name, Server, true)
#define BIND_ASYNC(name)  auto name = ServerBinder(# name, Server, false)

// =============================================================================
// -- Bind Actions -------------------------------------------------------------
// =============================================================================

void TrafficManagerServer::FPimpl::BindActions()
{
	namespace cr = carla::rpc;
	namespace cg = carla::geom;

	BIND_ASYNC(version) << [] () -> R<std::string>
	{
		return carla::version();
	};

	// ~~ Apply commands in batch ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

	using ActorId = carla::ActorId;


	BIND_SYNC(register_vehicles) << [=](
	const std::vector<ActorPtr> &actor_list)
	{
		Pimpl->Episode->RegisterVehicles(actor_list);
	};

	BIND_SYNC(unregister_vehicles) << [=](
	const std::vector<ActorPtr> &actor_list)
	{
		Pimpl->Episode->UnregisterVehicles(actor_list);
	};
}

// =============================================================================
// -- Undef helper macros ------------------------------------------------------
// =============================================================================

#undef BIND_ASYNC
#undef BIND_SYNC

// =============================================================================
// -- TrafficManagerServer -------------------------------------------------------
// =============================================================================

TrafficManagerServer::TrafficManagerServer() : Pimpl(nullptr) {}

TrafficManagerServer::~TrafficManagerServer() {}

void TrafficManagerServer::AsyncRun
	( const int RPCPort
	, carla::traffic_manager::TrafficManagerBase *episode)
{
	Pimpl = std::unique_ptr<FPimpl>(RPCPort, episode);
	Pimpl->Server.Start();
}

void TrafficManagerServer::Stop()
{
	check(Pimpl != nullptr);
	Pimpl->Server.Stop();
}

