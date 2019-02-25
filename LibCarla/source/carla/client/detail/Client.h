// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/Memory.h"
#include "carla/NonCopyable.h"
#include "carla/Time.h"
#include "carla/geom/Transform.h"
#include "carla/rpc/Actor.h"
#include "carla/rpc/ActorDefinition.h"
#include "carla/rpc/EpisodeInfo.h"
#include "carla/rpc/MapInfo.h"
#include "carla/rpc/WeatherParameters.h"
#include "carla/rpc/TrafficLightState.h"

#include <functional>
#include <memory>
#include <string>
#include <vector>

// Forward declarations.
namespace carla {
  class Buffer;
namespace rpc {
  class ActorDescription;
  class DebugShape;
  class VehicleControl;
  class WalkerControl;
}
namespace sensor {
  class SensorData;
}
namespace streaming {
  class Token;
}
}

namespace carla {
namespace client {
namespace detail {

  /// Provides communication with the rpc and streaming servers of a CARLA
  /// simulator.
  ///
  /// @todo Make sure this class is really thread-safe.
  class Client : private NonCopyable {
  public:

    explicit Client(
        const std::string &host,
        uint16_t port,
        size_t worker_threads = 0u);

    ~Client();

    void SetTimeout(time_duration timeout);

    time_duration GetTimeout() const;

    const std::string &GetEndpoint() const;

    std::string GetClientVersion();

    std::string GetServerVersion();

    rpc::EpisodeInfo GetEpisodeInfo();

    rpc::MapInfo GetMapInfo();

    std::vector<rpc::ActorDefinition> GetActorDefinitions();

    rpc::Actor GetSpectator();

    rpc::WeatherParameters GetWeatherParameters();

    void SetWeatherParameters(const rpc::WeatherParameters &weather);

    std::vector<rpc::Actor> GetActorsById(const std::vector<actor_id_type> &ids);

    rpc::Actor SpawnActor(
        const rpc::ActorDescription &description,
        const geom::Transform &transform);

    rpc::Actor SpawnActorWithParent(
        const rpc::ActorDescription &description,
        const geom::Transform &transform,
        const rpc::Actor &parent);

    bool DestroyActor(const rpc::Actor &actor);

    void SetActorLocation(
        const rpc::Actor &actor,
        const geom::Location &location);

    void SetActorTransform(
        const rpc::Actor &actor,
        const geom::Transform &transform);

    void SetActorSimulatePhysics(
        const rpc::Actor &actor,
        bool enabled);

    void SetActorAutopilot(
        const rpc::Actor &vehicle,
        bool enabled);

    void ApplyControlToVehicle(
        const rpc::Actor &vehicle,
        const rpc::VehicleControl &control);

    void ApplyControlToWalker(
        const rpc::Actor &walker,
        const rpc::WalkerControl &control);

    void SetTrafficLightState(
        const rpc::Actor &trafficLight,
        const rpc::TrafficLightState trafficLightState);

    void SetTrafficLightGreenTime(
        const rpc::Actor &trafficLight,
        float greenTime);

    void SetTrafficLightYellowTime(
        const rpc::Actor &trafficLight,
        float yellowTime);

    void SetTrafficLightRedTime(
        const rpc::Actor &trafficLight,
        float redTime);

    void FreezeTrafficLight(
        const rpc::Actor &trafficLight,
        bool freeze);

    void SetActorVelocity(
        const rpc::Actor &actor,
        const geom::Vector3D &vector);

    void SetActorAngularVelocity(
        const rpc::Actor &actor,
        const geom::Vector3D &vector);

    void AddActorImpulse(
        const rpc::Actor &actor,
        const geom::Vector3D &vector);

    void SubscribeToStream(
        const streaming::Token &token,
        std::function<void(Buffer)> callback);

    void UnSubscribeFromStream(const streaming::Token &token);

    void DrawDebugShape(const rpc::DebugShape &shape);

  private:

    class Pimpl;
    const std::unique_ptr<Pimpl> _pimpl;
  };

} // namespace detail
} // namespace client
} // namespace carla
