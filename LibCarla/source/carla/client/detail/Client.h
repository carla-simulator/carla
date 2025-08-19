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
#include "carla/geom/Location.h"
#include "carla/rpc/Actor.h"
#include "carla/rpc/ActorDefinition.h"
#include "carla/rpc/AttachmentType.h"
#include "carla/rpc/Command.h"
#include "carla/rpc/CommandResponse.h"
#include "carla/rpc/EnvironmentObject.h"
#include "carla/rpc/EpisodeInfo.h"
#include "carla/rpc/EpisodeSettings.h"
#include "carla/rpc/LabelledPoint.h"
#include "carla/rpc/LightState.h"
#include "carla/rpc/MapInfo.h"
#include "carla/rpc/MapLayer.h"
#include "carla/rpc/OpendriveGenerationParameters.h"
#include "carla/rpc/TrafficLightState.h"
#include "carla/rpc/VehicleDoor.h"
#include "carla/rpc/VehicleLightStateList.h"
#include "carla/rpc/VehicleLightState.h"
#include "carla/rpc/VehiclePhysicsControl.h"
#include "carla/rpc/VehicleTelemetryData.h"
#include "carla/rpc/VehicleWheels.h"
#include "carla/rpc/WeatherParameters.h"
#include "carla/rpc/Texture.h"
#include "carla/rpc/MaterialParameter.h"

#include <functional>
#include <memory>
#include <string>
#include <vector>

// Forward declarations.
namespace carla {
  class Buffer;
namespace rpc {
  class AckermannControllerSettings;
  class ActorDescription;
  class DebugShape;
  class VehicleAckermannControl;
  class VehicleControl;
  class WalkerControl;
  class WalkerBoneControlIn;
  class WalkerBoneControlOut;
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
  class Client : private NonCopyable {
  public:

    explicit Client(
        const std::string &host,
        uint16_t port,
        size_t worker_threads = 0u);

    ~Client();

    /// Querry to know if a Traffic Manager is running on port
    bool IsTrafficManagerRunning(uint16_t port) const;

    /// Gets a pair filled with the <IP, port> of the Trafic Manager running on port.
    /// If there is no Traffic Manager running the pair will be ("", 0)
    std::pair<std::string, uint16_t> GetTrafficManagerRunning(uint16_t port) const;

    /// Informs the server that a Traffic Manager is running on <IP, port>
    bool AddTrafficManagerRunning(std::pair<std::string, uint16_t> trafficManagerInfo) const;

    void DestroyTrafficManager(uint16_t port) const;

    void SetTimeout(time_duration timeout);

    time_duration GetTimeout() const;

    const std::string GetEndpoint() const;

    std::string GetClientVersion();

    std::string GetServerVersion();

    void LoadEpisode(std::string map_name, bool reset_settings = true, rpc::MapLayer map_layer = rpc::MapLayer::All);

    void LoadLevelLayer(rpc::MapLayer map_layer) const;

    void UnloadLevelLayer(rpc::MapLayer map_layer) const;

    void CopyOpenDriveToServer(
        std::string opendrive, const rpc::OpendriveGenerationParameters & params);

    void ApplyColorTextureToObjects(
        const std::vector<std::string> &objects_name,
        const rpc::MaterialParameter& parameter,
        const rpc::TextureColor& Texture);

    void ApplyColorTextureToObjects(
        const std::vector<std::string> &objects_name,
        const rpc::MaterialParameter& parameter,
        const rpc::TextureFloatColor& Texture);

    std::vector<std::string> GetNamesOfAllObjects() const;

    rpc::EpisodeInfo GetEpisodeInfo();

    rpc::MapInfo GetMapInfo();

    std::vector<uint8_t> GetNavigationMesh() const;

    bool SetFilesBaseFolder(const std::string &path);

    std::vector<std::string> GetRequiredFiles(const std::string &folder = "", const bool download = true) const;

    std::string GetMapData() const;

    void RequestFile(const std::string &name) const;

    std::vector<uint8_t> GetCacheFile(const std::string &name, const bool request_otherwise = true) const;

    std::vector<std::string> GetAvailableMaps();

    std::vector<rpc::ActorDefinition> GetActorDefinitions();

    rpc::Actor GetSpectator();

    rpc::EpisodeSettings GetEpisodeSettings();

    uint64_t SetEpisodeSettings(const rpc::EpisodeSettings &settings);

    rpc::WeatherParameters GetWeatherParameters();

    void SetWeatherParameters(const rpc::WeatherParameters &weather);

    float GetIMUISensorGravity() const;

    void SetIMUISensorGravity( float NewIMUISensorGravity );

    std::vector<rpc::Actor> GetActorsById(const std::vector<ActorId> &ids);

    rpc::VehiclePhysicsControl GetVehiclePhysicsControl(rpc::ActorId vehicle) const;

    rpc::VehicleLightState GetVehicleLightState(rpc::ActorId vehicle) const;

    void ApplyPhysicsControlToVehicle(
        rpc::ActorId vehicle,
        const rpc::VehiclePhysicsControl &physics_control);

    void SetLightStateToVehicle(
        rpc::ActorId vehicle,
        const rpc::VehicleLightState &light_state);

    void OpenVehicleDoor(
        rpc::ActorId vehicle,
        const rpc::VehicleDoor door_idx);

    void CloseVehicleDoor(
        rpc::ActorId vehicle,
        const rpc::VehicleDoor door_idx);

    rpc::Actor SpawnActor(
        const rpc::ActorDescription &description,
        const geom::Transform &transform);

    rpc::Actor SpawnActorWithParent(
        const rpc::ActorDescription &description,
        const geom::Transform &transform,
        rpc::ActorId parent,
        rpc::AttachmentType attachment_type,
        const std::string& socket_name = "");

    bool DestroyActor(rpc::ActorId actor);

    void SetActorLocation(
        rpc::ActorId actor,
        const geom::Location &location);

    void SetActorTransform(
        rpc::ActorId actor,
        const geom::Transform &transform);

    void SetActorTargetVelocity(
        rpc::ActorId actor,
        const geom::Vector3D &vector);

    void SetActorTargetAngularVelocity(
        rpc::ActorId actor,
        const geom::Vector3D &vector);

    void EnableActorConstantVelocity(
        rpc::ActorId actor,
        const geom::Vector3D &vector);

    void DisableActorConstantVelocity(
        rpc::ActorId actor);

    void AddActorImpulse(
        rpc::ActorId actor,
        const geom::Vector3D &impulse);

    void AddActorImpulse(
        rpc::ActorId actor,
        const geom::Vector3D &impulse,
        const geom::Vector3D &location);

    void AddActorForce(
        rpc::ActorId actor,
        const geom::Vector3D &force);

    void AddActorForce(
        rpc::ActorId actor,
        const geom::Vector3D &force,
        const geom::Vector3D &location);

    void AddActorAngularImpulse(
        rpc::ActorId actor,
        const geom::Vector3D &vector);

    void AddActorTorque(
        rpc::ActorId actor,
        const geom::Vector3D &vector);

    geom::BoundingBox GetActorBoundingBox(
        rpc::ActorId actor);

    geom::Transform GetActorComponentWorldTransform(
        rpc::ActorId actor,
        const std::string componentName);

    geom::Transform GetActorComponentRelativeTransform(
        rpc::ActorId actor,
        const std::string componentName);

    std::vector<geom::Transform> GetActorBoneWorldTransforms(
        rpc::ActorId actor);
        
    std::vector<geom::Transform> GetVehicleBoneWorldTransforms(
        rpc::ActorId actor);

    std::vector<geom::Transform> GetActorBoneRelativeTransforms(
        rpc::ActorId actor);

    std::vector<std::string> GetActorComponentNames(
        rpc::ActorId actor);

    std::vector<std::string> GetActorBoneNames(
        rpc::ActorId actor);

    std::vector<geom::Transform> GetActorSocketWorldTransforms(
        rpc::ActorId actor);

    std::vector<geom::Transform> GetActorSocketRelativeTransforms(
        rpc::ActorId actor);

    std::vector<std::string> GetActorSocketNames(
        rpc::ActorId actor);

    void SetActorSimulatePhysics(
        rpc::ActorId actor,
        bool enabled);

    void SetActorCollisions(
        rpc::ActorId actor,
        bool enabled);

    void SetActorDead(
        rpc::ActorId actor);

    void SetActorEnableGravity(
        rpc::ActorId actor,
        bool enabled);

    void SetActorAutopilot(
        rpc::ActorId vehicle,
        bool enabled);

    rpc::VehicleTelemetryData GetVehicleTelemetryData(rpc::ActorId vehicle) const;

    void ShowVehicleDebugTelemetry(
        rpc::ActorId vehicle,
        bool enabled);

    void ApplyControlToVehicle(
        rpc::ActorId vehicle,
        const rpc::VehicleControl &control);

    void ApplyAckermannControlToVehicle(
        rpc::ActorId vehicle,
        const rpc::VehicleAckermannControl &control);

    rpc::AckermannControllerSettings GetAckermannControllerSettings(rpc::ActorId vehicle) const;

    void ApplyAckermannControllerSettings(
        rpc::ActorId vehicle,
        const rpc::AckermannControllerSettings &settings);

    void EnableCarSim(
        rpc::ActorId vehicle,
        std::string simfile_path);

    void UseCarSimRoad(
        rpc::ActorId vehicle,
        bool enabled);

    void SetWheelSteerDirection(
        rpc::ActorId vehicle,
        rpc::VehicleWheelLocation vehicle_wheel,
        float angle_in_deg
    );

    float GetWheelSteerAngle(
        rpc::ActorId vehicle,
        rpc::VehicleWheelLocation wheel_location
    );

    void EnableChronoPhysics(
        rpc::ActorId vehicle,
        uint64_t MaxSubsteps,
        float MaxSubstepDeltaTime,
        std::string VehicleJSON,
        std::string PowertrainJSON,
        std::string TireJSON,
        std::string BaseJSONPath);

    void RestorePhysXPhysics(rpc::ActorId vehicle);

    void ApplyControlToWalker(
        rpc::ActorId walker,
        const rpc::WalkerControl &control);

    rpc::WalkerBoneControlOut GetBonesTransform(
        rpc::ActorId walker);

    void SetBonesTransform(
        rpc::ActorId walker,
        const rpc::WalkerBoneControlIn &bones);

    void BlendPose(
        rpc::ActorId walker,
        float blend);

    void GetPoseFromAnimation(
        rpc::ActorId walker);

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

    void ResetTrafficLightGroup(
        rpc::ActorId traffic_light);

    void ResetAllTrafficLights();

    void FreezeAllTrafficLights(bool frozen);

    std::vector<geom::BoundingBox> GetLightBoxes(
        rpc::ActorId traffic_light) const;

    /// Returns a list of pairs where the firts element is the vehicle ID
    /// and the second one is the light state
    rpc::VehicleLightStateList GetVehiclesLightStates();

    std::vector<ActorId> GetGroupTrafficLights(
        rpc::ActorId traffic_light);

    std::string StartRecorder(std::string name, bool additional_data);

    void StopRecorder();

    std::string ShowRecorderFileInfo(std::string name, bool show_all);

    std::string ShowRecorderCollisions(std::string name, char type1, char type2);

    std::string ShowRecorderActorsBlocked(std::string name, double min_time, double min_distance);

    std::string ReplayFile(std::string name, double start, double duration,
        uint32_t follow_id, bool replay_sensors, geom::Transform offset);

    void SetReplayerTimeFactor(double time_factor);

    void SetReplayerIgnoreHero(bool ignore_hero);

    void SetReplayerIgnoreSpectator(bool ignore_spectator);

    void StopReplayer(bool keep_actors);

    void SubscribeToStream(
        const streaming::Token &token,
        std::function<void(Buffer)> callback);

    void EnableGBuffers(rpc::ActorId ActorId, bool bEnabled);
    
    bool AreGBuffersEnabled(rpc::ActorId ActorId);

    void SubscribeToGBuffer(
        rpc::ActorId ActorId,
        uint32_t GBufferId,
        std::function<void(Buffer)> callback);

    void UnSubscribeFromStream(const streaming::Token &token);

    void EnableForROS(const streaming::Token &token);

    void DisableForROS(const streaming::Token &token);

    bool IsEnabledForROS(const streaming::Token &token);

    void UnSubscribeFromGBuffer(
        rpc::ActorId ActorId,
        uint32_t GBufferId);

    void Send(rpc::ActorId ActorId, std::string message);

    void SetIgnoredVehicles(rpc::ActorId ActorId, const std::vector<rpc::ActorId>& vehicle_ids);

    void DrawDebugShape(const rpc::DebugShape &shape);

    void ClearDebugShape();

    void ClearDebugString();
    
    void ApplyBatch(
        std::vector<rpc::Command> commands,
        bool do_tick_cue);

    std::vector<rpc::CommandResponse> ApplyBatchSync(
        std::vector<rpc::Command> commands,
        bool do_tick_cue);

    uint64_t SendTickCue();

    std::vector<rpc::LightState> QueryLightsStateToServer() const;

    void UpdateServerLightsState(
        std::vector<rpc::LightState>& lights,
        bool discard_client = false) const;

    void UpdateDayNightCycle(const bool active) const;

    /// Returns all the BBs of all the elements of the level
    std::vector<geom::BoundingBox> GetLevelBBs(uint8_t queried_tag) const;

    std::vector<rpc::EnvironmentObject> GetEnvironmentObjects(uint8_t queried_tag) const;

    void EnableEnvironmentObjects(
      std::vector<uint64_t> env_objects_ids,
      bool enable) const;

    std::pair<bool,rpc::LabelledPoint> ProjectPoint(
        geom::Location location, geom::Vector3D direction, float search_distance) const;

    std::vector<rpc::LabelledPoint> CastRay(
        geom::Location start_location, geom::Location end_location) const;

    void SetAnnotationsTraverseTranslucency(
        bool enable);
  private:

    class Pimpl;
    const std::unique_ptr<Pimpl> _pimpl;
  };

} // namespace detail
} // namespace client
} // namespace carla
