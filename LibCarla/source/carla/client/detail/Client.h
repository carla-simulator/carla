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

    void LoadEpisode(std::string map_name);

    rpc::EpisodeInfo GetEpisodeInfo();

    rpc::MapInfo GetMapInfo();

    std::vector<std::string> GetAvailableMaps();

    std::vector<rpc::ActorDefinition> GetActorDefinitions();

    rpc::Actor GetSpectator();

    rpc::EpisodeSettings GetEpisodeSettings();

    void SetEpisodeSettings(const rpc::EpisodeSettings &settings);

    rpc::WeatherParameters GetWeatherParameters();

    void SetWeatherParameters(const rpc::WeatherParameters &weather);

    std::vector<rpc::Actor> GetActorsById(const std::vector<ActorId> &ids);

    rpc::VehiclePhysicsControl GetVehiclePhysicsControl(
        const rpc::ActorId &vehicle) const;

    void ApplyPhysicsControlToVehicle(
        const rpc::ActorId &vehicle,
        const rpc::VehiclePhysicsControl &physics_control);

    rpc::Actor SpawnActor(
        const rpc::ActorDescription &description,
        const geom::Transform &transform);

    rpc::Actor SpawnActorWithParent(
        const rpc::ActorDescription &description,
        const geom::Transform &transform,
        rpc::ActorId parent,
        rpc::AttachmentType attachment_type);

    bool DestroyActor(rpc::ActorId actor);

    void SetActorLocation(
        rpc::ActorId actor,
        const geom::Location &location);

    void SetActorTransform(
        rpc::ActorId actor,
        const geom::Transform &transform);

    void SetActorVelocity(
        rpc::ActorId actor,
        const geom::Vector3D &vector);

    void SetActorAngularVelocity(
        rpc::ActorId actor,
        const geom::Vector3D &vector);

    void AddActorImpulse(
        rpc::ActorId actor,
        const geom::Vector3D &vector);

    void SetActorSimulatePhysics(
        rpc::ActorId actor,
        bool enabled);

    void SetActorAutopilot(
        rpc::ActorId vehicle,
        bool enabled);

    void ApplyControlToVehicle(
        rpc::ActorId vehicle,
        const rpc::VehicleControl &control);

    void ApplyControlToWalker(
        rpc::ActorId walker,
        const rpc::WalkerControl &control);

    void SetTrafficLightState(
        rpc::ActorId traffic_light,
        const rpc::TrafficLightState trafficLightState);

    void SetTrafficLightGreenTime(
        rpc::ActorId traffic_light,
        float green_time);

    void SetTrafficLightYellowTime(
        rpc::ActorId traffic_light,
        float yellow_time);

    void SetTrafficLightRedTime(
        rpc::ActorId traffic_light,
        float red_time);

    void FreezeTrafficLight(
        rpc::ActorId traffic_light,
        bool freeze);

    std::vector<ActorId> GetGroupTrafficLights(
        const rpc::ActorId &traffic_light);

    std::string StartRecorder(std::string name);

    void StopRecorder();

    std::string ShowRecorderFileInfo(std::string name, bool show_all);

    std::string ShowRecorderCollisions(std::string name, char type1, char type2);

    std::string ShowRecorderActorsBlocked(std::string name, double min_time, double min_distance);

    std::string ReplayFile(std::string name, double start, double duration, uint32_t follow_id);

    void SetReplayerTimeFactor(double time_factor);

    void SubscribeToStream(
        const streaming::Token &token,
        std::function<void(Buffer)> callback);

    void UnSubscribeFromStream(const streaming::Token &token);

    void DrawDebugShape(const rpc::DebugShape &shape);

    void ApplyBatch(
        std::vector<rpc::Command> commands,
        bool do_tick_cue);

    std::vector<rpc::CommandResponse> ApplyBatchSync(
        std::vector<rpc::Command> commands,
        bool do_tick_cue);

    void SendTickCue();

  private:

    class Pimpl;
    const std::unique_ptr<Pimpl> _pimpl;
  };

} // namespace detail
} // namespace client
} // namespace carla
