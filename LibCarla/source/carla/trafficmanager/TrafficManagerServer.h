// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once
#include <memory.h>
#include "carla/trafficmanager/TrafficManagerBase.h"

class TrafficManagerServer
{
public:

	TrafficManagerServer();

	~TrafficManagerServer();

	void AsyncRun
		( const int RPCPort
		, carla::traffic_manager::TrafficManagerBase *episode);

	void Stop();

private:

	class FPimpl;
	std::unique_ptr<FPimpl> Pimpl;
};
