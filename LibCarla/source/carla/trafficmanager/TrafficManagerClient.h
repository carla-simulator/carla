// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/Memory.h"
#include "carla/NonCopyable.h"
#include "carla/client/Actor.h"

#include "carla/Time.h"
#include "carla/geom/Transform.h"
#include "carla/rpc/Actor.h"
#include "carla/rpc/ActorDefinition.h"
#include "carla/rpc/AttachmentType.h"
#include "carla/rpc/Command.h"
#include "carla/rpc/CommandResponse.h"
#include "carla/rpc/EpisodeInfo.h"
#include "carla/rpc/EpisodeSettings.h"
#include "carla/rpc/MapInfo.h"
#include "carla/rpc/TrafficLightState.h"
#include "carla/rpc/VehiclePhysicsControl.h"
#include "carla/rpc/WeatherParameters.h"

#include <functional>
#include <memory>
#include <string>
#include <vector>

// Forward declarations.
namespace carla {
namespace client {
	class TimeoutException;
}
namespace rpc {
	class ActorDescription;
	class DebugShape;
	class VehicleControl;
	class WalkerControl;
	class WalkerBoneControl;
}
}

namespace carla {
namespace traffic_manager {
namespace client {

using ActorPtr 	= carla::SharedPtr<carla::client::Actor>;

  /// Provides communication with the rpc of TrafficManagerServer
  class TrafficManagerClient : private NonCopyable {
  public:

    explicit TrafficManagerClient(
        const std::string &host,
        uint16_t port,
        size_t worker_threads = 0u);

    ~TrafficManagerClient();

    /// Set timeout for TrafficManagerServer in Carla Server
    void SetTimeout(time_duration timeout);

    /// Get current timeout of TrafficManagerServer from Carla Server
    time_duration GetTimeout() const;

    /// Get end point details
    const std::string GetEndpoint() const;

    /// Current version of Traffic Manager client
    std::string GetTrafficManagerClientVersion();

    /// Current version of Traffic Manager server
    std::string GetTrafficManagerServerVersion();

    /// Register remote vehicles
    void RegisterVehicle(const std::vector<ActorPtr> &actor_list);

    /// Unregister remote vehicles
    void UnregisterVehicle(const std::vector<ActorPtr> &actor_list);

  private:

    class Pimpl;
    const std::unique_ptr<Pimpl> _pimpl;
  };

} // namespace detail
} // namespace client
} // namespace carla
