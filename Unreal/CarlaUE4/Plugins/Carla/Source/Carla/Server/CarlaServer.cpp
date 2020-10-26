// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "Carla.h"
#include "Carla/Server/CarlaServer.h"

#include "Carla/OpenDrive/OpenDrive.h"
#include "Carla/Util/DebugShapeDrawer.h"
#include "Carla/Util/NavigationMesh.h"
#include "Carla/Util/RayTracer.h"
#include "Carla/Vehicle/CarlaWheeledVehicle.h"
#include "Carla/Walker/WalkerController.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Carla/Game/Tagger.h"

#include <compiler/disable-ue4-macros.h>
#include <carla/Functional.h>
#include <carla/Version.h>
#include <carla/rpc/Actor.h>
#include <carla/rpc/ActorDefinition.h>
#include <carla/rpc/ActorDescription.h>
#include <carla/rpc/Command.h>
#include <carla/rpc/CommandResponse.h>
#include <carla/rpc/DebugShape.h>
#include <carla/rpc/EpisodeInfo.h>
#include <carla/rpc/EpisodeSettings.h>
#include "carla/rpc/LabelledPoint.h"
#include <carla/rpc/LightState.h>
#include <carla/rpc/MapInfo.h>
#include <carla/rpc/EnvironmentObject.h>
#include <carla/rpc/Response.h>
#include <carla/rpc/Server.h>
#include <carla/rpc/String.h>
#include <carla/rpc/Transform.h>
#include <carla/rpc/Vector2D.h>
#include <carla/rpc/Vector3D.h>
#include <carla/rpc/VehicleControl.h>
#include <carla/rpc/VehiclePhysicsControl.h>
#include <carla/rpc/VehicleLightState.h>
#include <carla/rpc/VehicleLightStateList.h>
#include <carla/rpc/WalkerBoneControl.h>
#include <carla/rpc/WalkerControl.h>
#include <carla/rpc/WeatherParameters.h>
#include <carla/streaming/Server.h>
#include <compiler/enable-ue4-macros.h>

#include <vector>
#include <map>
#include <tuple>

template <typename T>
using R = carla::rpc::Response<T>;

// =============================================================================
// -- Static local functions ---------------------------------------------------
// =============================================================================

template <typename T, typename Other>
static std::vector<T> MakeVectorFromTArray(const TArray<Other> &Array)
{
  return {Array.GetData(), Array.GetData() + Array.Num()};
}

// =============================================================================
// -- FCarlaServer::FPimpl -----------------------------------------------
// =============================================================================

class FCarlaServer::FPimpl
{
public:

  FPimpl(uint16_t RPCPort, uint16_t StreamingPort)
    : Server(RPCPort),
      StreamingServer(StreamingPort),
      BroadcastStream(StreamingServer.MakeStream())
  {
    BindActions();
  }

  /// Map of pairs < port , ip > with all the Traffic Managers active in the simulation
  std::map<uint16_t, std::string> TrafficManagerInfo;

  carla::rpc::Server Server;

  carla::streaming::Server StreamingServer;

  carla::streaming::Stream BroadcastStream;

  UCarlaEpisode *Episode = nullptr;

  size_t TickCuesReceived = 0u;

private:

  void BindActions();
};

// =============================================================================
// -- Define helper macros -----------------------------------------------------
// =============================================================================

#if WITH_EDITOR
#  define CARLA_ENSURE_GAME_THREAD() check(IsInGameThread());
#else
#  define CARLA_ENSURE_GAME_THREAD()
#endif // WITH_EDITOR

#define RESPOND_ERROR(str) {                                              \
    UE_LOG(LogCarlaServer, Log, TEXT("Responding error: %s"), TEXT(str)); \
    return carla::rpc::ResponseError(str); }

#define RESPOND_ERROR_FSTRING(fstr) {                                 \
    UE_LOG(LogCarlaServer, Log, TEXT("Responding error: %s"), *fstr); \
    return carla::rpc::ResponseError(carla::rpc::FromFString(fstr)); }

#define REQUIRE_CARLA_EPISODE() \
    CARLA_ENSURE_GAME_THREAD();   \
    if (Episode == nullptr) { RESPOND_ERROR("episode not ready"); }

class ServerBinder
{
public:

  constexpr ServerBinder(const char *name, carla::rpc::Server &srv, bool sync)
    : _name(name),
      _server(srv),
      _sync(sync) {}

  template <typename FuncT>
  auto operator<<(FuncT func)
  {
    if (_sync)
    {
      _server.BindSync(_name, func);
    }
    else
    {
      _server.BindAsync(_name, func);
    }
    return func;
  }

private:

  const char *_name;

  carla::rpc::Server &_server;

  bool _sync;
};

#define BIND_SYNC(name)   auto name = ServerBinder(# name, Server, true)
#define BIND_ASYNC(name)  auto name = ServerBinder(# name, Server, false)

// =============================================================================
// -- Bind Actions -------------------------------------------------------------
// =============================================================================

void FCarlaServer::FPimpl::BindActions()
{
  namespace cr = carla::rpc;
  namespace cg = carla::geom;

  /// Looks for a Traffic Manager running on port
  BIND_SYNC(is_traffic_manager_running) << [this] (uint16_t port) ->R<bool>
  {
    return (TrafficManagerInfo.find(port) != TrafficManagerInfo.end());
  };

  /// Gets a pair filled with the <IP, port> of the Trafic Manager running on port.
  /// If there is no Traffic Manager running the pair will be ("", 0)
  BIND_SYNC(get_traffic_manager_running) << [this] (uint16_t port) ->R<std::pair<std::string, uint16_t>>
  {
    auto it = TrafficManagerInfo.find(port);
    if(it != TrafficManagerInfo.end()) {
      return std::pair<std::string, uint16_t>(it->second, it->first);
    }
    return std::pair<std::string, uint16_t>("",0);
  };

  /// Add a new Traffic Manager running on <IP, port>
  BIND_SYNC(add_traffic_manager_running) << [this] (std::pair<std::string, uint16_t> trafficManagerInfo) ->R<bool>
  {
    uint16_t port = trafficManagerInfo.second;
    auto it = TrafficManagerInfo.find(port);
    if(it == TrafficManagerInfo.end()) {
      TrafficManagerInfo.insert(
        std::pair<uint16_t, std::string>(port, trafficManagerInfo.first));
      return true;
    }
    return false;

  };

  BIND_SYNC(destroy_traffic_manager) << [this] (uint16_t port) ->R<bool>
  {
    auto it = TrafficManagerInfo.find(port);
    if(it != TrafficManagerInfo.end()) {
      TrafficManagerInfo.erase(it);
      return true;
    }
    return false;
  };

  BIND_ASYNC(version) << [] () -> R<std::string>
  {
    return carla::version();
  };

  // ~~ Tick ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

  BIND_SYNC(tick_cue) << [this]() -> R<uint64_t>
  {
    ++TickCuesReceived;
    return GFrameCounter + 1u;
  };

  // ~~ Load new episode ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

  BIND_ASYNC(get_available_maps) << [this]() -> R<std::vector<std::string>>
  {
    const auto MapNames = UCarlaStatics::GetAllMapNames();
    std::vector<std::string> result;
    result.reserve(MapNames.Num());
    for (const auto &MapName : MapNames)
    {
      if (MapName.Contains("/Sublevels/"))
        continue;
      if (MapName.Contains("/BaseMap/"))
        continue;

      result.emplace_back(cr::FromFString(MapName));
    }
    return result;
  };

  BIND_SYNC(load_new_episode) << [this](const std::string &map_name) -> R<void>
  {
    REQUIRE_CARLA_EPISODE();
    if(!Episode->LoadNewEpisode(cr::ToFString(map_name)))
    {
      RESPOND_ERROR("map not found");
    }
    return R<void>::Success();
  };

  BIND_SYNC(copy_opendrive_to_file) << [this](const std::string &opendrive, cr::OpendriveGenerationParameters Params) -> R<void>
  {
    REQUIRE_CARLA_EPISODE();
    if (!Episode->LoadNewOpendriveEpisode(cr::ToLongFString(opendrive), Params))
    {
      RESPOND_ERROR("opendrive could not be correctly parsed");
    }
    return R<void>::Success();
  };

  // ~~ Episode settings and info ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

  BIND_SYNC(get_episode_info) << [this]() -> R<cr::EpisodeInfo>
  {
    REQUIRE_CARLA_EPISODE();
    return cr::EpisodeInfo{Episode->GetId(), BroadcastStream.token()};
  };

  BIND_SYNC(get_map_info) << [this]() -> R<cr::MapInfo>
  {
    REQUIRE_CARLA_EPISODE();
    auto FileContents = UOpenDrive::LoadXODR(Episode->GetMapName());
    const auto &SpawnPoints = Episode->GetRecommendedSpawnPoints();
    return cr::MapInfo{
      cr::FromFString(Episode->GetMapName()),
      cr::FromLongFString(FileContents),
      MakeVectorFromTArray<cg::Transform>(SpawnPoints)};
  };

  BIND_SYNC(get_navigation_mesh) << [this]() -> R<std::vector<uint8_t>>
  {
    REQUIRE_CARLA_EPISODE();
    auto FileContents = FNavigationMesh::Load(Episode->GetMapName());
    // make a mem copy (from TArray to std::vector)
    std::vector<uint8_t> Result(FileContents.Num());
    memcpy(&Result[0], FileContents.GetData(), FileContents.Num());
    return Result;
  };

  BIND_SYNC(get_episode_settings) << [this]() -> R<cr::EpisodeSettings>
  {
    REQUIRE_CARLA_EPISODE();
    return cr::EpisodeSettings{Episode->GetSettings()};
  };

  BIND_SYNC(set_episode_settings) << [this](
      const cr::EpisodeSettings &settings) -> R<uint64_t>
  {
    REQUIRE_CARLA_EPISODE();
    Episode->ApplySettings(settings);
    StreamingServer.SetSynchronousMode(settings.synchronous_mode);
    return GFrameCounter;
  };

  BIND_SYNC(get_actor_definitions) << [this]() -> R<std::vector<cr::ActorDefinition>>
  {
    REQUIRE_CARLA_EPISODE();
    return MakeVectorFromTArray<cr::ActorDefinition>(Episode->GetActorDefinitions());
  };

  BIND_SYNC(get_spectator) << [this]() -> R<cr::Actor>
  {
    REQUIRE_CARLA_EPISODE();
    auto ActorView = Episode->FindActor(Episode->GetSpectatorPawn());
    if (!ActorView.IsValid())
    {
      RESPOND_ERROR("internal error: unable to find spectator");
    }
    return Episode->SerializeActor(ActorView);
  };

  BIND_SYNC(get_all_level_BBs) << [this](uint8 QueriedTag) -> R<std::vector<cg::BoundingBox>>
  {
    REQUIRE_CARLA_EPISODE();
    TArray<FBoundingBox> Result;
    ACarlaGameModeBase* GameMode = UCarlaStatics::GetGameMode(Episode->GetWorld());
    if (!GameMode)
    {
      RESPOND_ERROR("unable to find CARLA game mode");
    }
    Result = GameMode->GetAllBBsOfLevel(QueriedTag);
    return MakeVectorFromTArray<cg::BoundingBox>(Result);
  };

  BIND_SYNC(get_environment_objects) << [this]() -> R<std::vector<cr::EnvironmentObject>>
  {
    REQUIRE_CARLA_EPISODE();
    ACarlaGameModeBase* GameMode = UCarlaStatics::GetGameMode(Episode->GetWorld());
    if (!GameMode)
    {
      RESPOND_ERROR("unable to find CARLA game mode");
    }
    TArray<FEnvironmentObject> Result = GameMode->GetEnvironmentObjects();
    return MakeVectorFromTArray<cr::EnvironmentObject>(Result);
  };

  BIND_SYNC(enable_environment_objects) << [this](std::vector<uint64_t> EnvObjectIds, bool Enable) -> R<void>
  {
    REQUIRE_CARLA_EPISODE();
    ACarlaGameModeBase* GameMode = UCarlaStatics::GetGameMode(Episode->GetWorld());
    if (!GameMode)
    {
      RESPOND_ERROR("unable to find CARLA game mode");
    }

    TSet<uint64> EnvObjectIdsSet;
    for(uint64 Id : EnvObjectIds)
    {
      EnvObjectIdsSet.Emplace(Id);
    }

    GameMode->EnableEnvironmentObjects(EnvObjectIdsSet, Enable);
    return R<void>::Success();
  };

  // ~~ Weather ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

  BIND_SYNC(get_weather_parameters) << [this]() -> R<cr::WeatherParameters>
  {
    REQUIRE_CARLA_EPISODE();
    auto *Weather = Episode->GetWeather();
    if (Weather == nullptr)
    {
      RESPOND_ERROR("internal error: unable to find weather");
    }
    return Weather->GetCurrentWeather();
  };

  BIND_SYNC(set_weather_parameters) << [this](
      const cr::WeatherParameters &weather) -> R<void>
  {
    REQUIRE_CARLA_EPISODE();
    auto *Weather = Episode->GetWeather();
    if (Weather == nullptr)
    {
      RESPOND_ERROR("internal error: unable to find weather");
    }
    Weather->ApplyWeather(weather);
    return R<void>::Success();
  };

  // ~~ Actor operations ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

  BIND_SYNC(get_actors_by_id) << [this](
      const std::vector<FActorView::IdType> &ids) -> R<std::vector<cr::Actor>>
  {
    REQUIRE_CARLA_EPISODE();
    std::vector<cr::Actor> Result;
    Result.reserve(ids.size());
    for (auto &&Id : ids)
    {
      auto View = Episode->FindActor(Id);
      if (View.IsValid())
      {
        Result.emplace_back(Episode->SerializeActor(View));
      }
    }
    return Result;
  };

  BIND_SYNC(spawn_actor) << [this](
      cr::ActorDescription Description,
      const cr::Transform &Transform) -> R<cr::Actor>
  {
    REQUIRE_CARLA_EPISODE();
    auto Result = Episode->SpawnActorWithInfo(Transform, std::move(Description));
    if (Result.Key != EActorSpawnResultStatus::Success)
    {
      RESPOND_ERROR_FSTRING(FActorSpawnResult::StatusToString(Result.Key));
    }
    if (!Result.Value.IsValid())
    {
      RESPOND_ERROR("internal error: actor could not be spawned");
    }
    return Episode->SerializeActor(Result.Value);
  };

  BIND_SYNC(spawn_actor_with_parent) << [this](
      cr::ActorDescription Description,
      const cr::Transform &Transform,
      cr::ActorId ParentId,
      cr::AttachmentType InAttachmentType) -> R<cr::Actor>
  {
    REQUIRE_CARLA_EPISODE();
    auto Result = Episode->SpawnActorWithInfo(Transform, std::move(Description));
    if (Result.Key != EActorSpawnResultStatus::Success)
    {
      RESPOND_ERROR_FSTRING(FActorSpawnResult::StatusToString(Result.Key));
    }
    if (!Result.Value.IsValid())
    {
      RESPOND_ERROR("internal error: actor could not be spawned");
    }
    auto ParentActorView = Episode->FindActor(ParentId);
    if (!ParentActorView.IsValid())
    {
      RESPOND_ERROR("unable to attach actor: parent actor not found");
    }
    Episode->AttachActors(
        Result.Value.GetActor(),
        ParentActorView.GetActor(),
        static_cast<EAttachmentType>(InAttachmentType));
    return Episode->SerializeActor(Result.Value);
  };

  BIND_SYNC(destroy_actor) << [this](cr::ActorId ActorId) -> R<void>
  {
    REQUIRE_CARLA_EPISODE();
    auto ActorView = Episode->FindActor(ActorId);
    if (!ActorView.IsValid())
    {
      RESPOND_ERROR("unable to destroy actor: not found");
    }
    if (!Episode->DestroyActor(ActorView.GetActor()))
    {
      RESPOND_ERROR("internal error: unable to destroy actor");
    }
    return R<void>::Success();
  };

  // ~~ Actor physics ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

  BIND_SYNC(set_actor_location) << [this](
      cr::ActorId ActorId,
      cr::Location Location) -> R<void>
  {
    REQUIRE_CARLA_EPISODE();
    auto ActorView = Episode->FindActor(ActorId);
    if (!ActorView.IsValid())
    {
      RESPOND_ERROR("unable to set actor location: actor not found");
    }
    ActorView.GetActor()->SetActorRelativeLocation(
        Location,
        false,
        nullptr,
        ETeleportType::TeleportPhysics);
    return R<void>::Success();
  };

  BIND_SYNC(set_actor_transform) << [this](
      cr::ActorId ActorId,
      cr::Transform Transform) -> R<void>
  {
    REQUIRE_CARLA_EPISODE();
    auto ActorView = Episode->FindActor(ActorId);
    if (!ActorView.IsValid())
    {
      RESPOND_ERROR("unable to set actor transform: actor not found");
    }
    ActorView.GetActor()->SetActorRelativeTransform(
        Transform,
        false,
        nullptr,
        ETeleportType::TeleportPhysics);
    return R<void>::Success();
  };

  BIND_SYNC(set_walker_state) << [this] (
      cr::ActorId ActorId,
      cr::Transform Transform,
      float Speed) -> R<void>
  {
    REQUIRE_CARLA_EPISODE();
    auto ActorView = Episode->FindActor(ActorId);
    if (!ActorView.IsValid())
    {
      RESPOND_ERROR("unable to set walker state: actor not found");
    }

    // apply walker transform
    FTransform NewTransform = Transform;
    FVector NewLocation = NewTransform.GetLocation();

    FTransform CurrentTransform = ActorView.GetActor()->GetTransform();
    FVector CurrentLocation = CurrentTransform.GetLocation();
    NewLocation.Z += 90.0f; // move point up because in Unreal walker is centered in the middle height

    // if difference between Z position is small, then we keep current, otherwise we set the new one
    // (to avoid Z fighting position and falling pedestrians)
    if (NewLocation.Z - CurrentLocation.Z < 100.0f)
      NewLocation.Z = CurrentLocation.Z;

    NewTransform.SetLocation(NewLocation);

    ActorView.GetActor()->SetActorRelativeTransform(
    NewTransform,
    false,
    nullptr,
    ETeleportType::TeleportPhysics);

    // apply walker speed
    auto Pawn = Cast<APawn>(ActorView.GetActor());
    if (Pawn == nullptr)
    {
      RESPOND_ERROR("unable to set walker state: actor is not a walker");
    }
    auto Controller = Cast<AWalkerController>(Pawn->GetController());
    if (Controller == nullptr)
    {
      RESPOND_ERROR("unable to set walker state: walker has an incompatible controller");
    }
    cr::WalkerControl Control(Transform.GetForwardVector(), Speed, false);
    Controller->ApplyWalkerControl(Control);

    return R<void>::Success();
  };


  BIND_SYNC(set_actor_target_velocity) << [this](
      cr::ActorId ActorId,
      cr::Vector3D vector) -> R<void>
  {
    REQUIRE_CARLA_EPISODE();
    auto ActorView = Episode->FindActor(ActorId);
    if (!ActorView.IsValid())
    {
      RESPOND_ERROR("unable to set actor target velocity: actor not found");
    }
    auto RootComponent = Cast<UPrimitiveComponent>(ActorView.GetActor()->GetRootComponent());
    if (RootComponent == nullptr)
    {
      RESPOND_ERROR("unable to set actor target velocity: not supported by actor");
    }
    RootComponent->SetPhysicsLinearVelocity(
        vector.ToCentimeters().ToFVector(),
        false,
        "None");
    return R<void>::Success();
  };

  BIND_SYNC(set_actor_target_angular_velocity) << [this](
      cr::ActorId ActorId,
      cr::Vector3D vector) -> R<void>
  {
    REQUIRE_CARLA_EPISODE();
    auto ActorView = Episode->FindActor(ActorId);
    if (!ActorView.IsValid())
    {
      RESPOND_ERROR("unable to set actor target angular velocity: actor not found");
    }
    auto RootComponent = Cast<UPrimitiveComponent>(ActorView.GetActor()->GetRootComponent());
    if (RootComponent == nullptr)
    {
      RESPOND_ERROR("unable to set actor target angular velocity: not supported by actor");
    }
    RootComponent->SetPhysicsAngularVelocityInDegrees(
        vector.ToFVector(),
        false,
        "None");
    return R<void>::Success();
  };

  BIND_SYNC(enable_actor_constant_velocity) << [this](
      cr::ActorId ActorId,
      cr::Vector3D vector) -> R<void>
  {
    REQUIRE_CARLA_EPISODE();
    auto ActorView = Episode->FindActor(ActorId);
    if (!ActorView.IsValid())
    {
      RESPOND_ERROR("unable to set actor velocity: actor not found");
    }
    auto CarlaVehicle = Cast<ACarlaWheeledVehicle>(ActorView.GetActor());
    if (CarlaVehicle == nullptr)
    {
      RESPOND_ERROR("unable to set actor velocity: not supported by actor");
    }

    CarlaVehicle->ActivateVelocityControl(vector.ToCentimeters().ToFVector());

    return R<void>::Success();
  };

  BIND_SYNC(disable_actor_constant_velocity) << [this](
      cr::ActorId ActorId) -> R<void>
  {
    REQUIRE_CARLA_EPISODE();
    auto ActorView = Episode->FindActor(ActorId);
    if (!ActorView.IsValid())
    {
      RESPOND_ERROR("unable to set actor velocity: actor not found");
    }
    auto CarlaVehicle = Cast<ACarlaWheeledVehicle>(ActorView.GetActor());
    if (CarlaVehicle == nullptr)
    {
      RESPOND_ERROR("unable to set actor velocity: not supported by actor");
    }

    CarlaVehicle->DeactivateVelocityControl();

    return R<void>::Success();
  };

  BIND_SYNC(add_actor_impulse) << [this](
      cr::ActorId ActorId,
      cr::Vector3D vector) -> R<void>
  {
    REQUIRE_CARLA_EPISODE();
    auto ActorView = Episode->FindActor(ActorId);
    if (!ActorView.IsValid())
    {
      RESPOND_ERROR("unable to add actor impulse: actor not found");
    }
    auto RootComponent = Cast<UPrimitiveComponent>(ActorView.GetActor()->GetRootComponent());
    if (RootComponent == nullptr)
    {
      RESPOND_ERROR("unable to add actor impulse: not supported by actor");
    }
    RootComponent->AddImpulse(
        vector.ToCentimeters().ToFVector(),
        "None",
        false);
    return R<void>::Success();
  };

  BIND_SYNC(add_actor_impulse_at_location) << [this](
      cr::ActorId ActorId,
      cr::Vector3D impulse,
      cr::Vector3D location) -> R<void>
  {
    REQUIRE_CARLA_EPISODE();
    auto ActorView = Episode->FindActor(ActorId);
    if (!ActorView.IsValid())
    {
      RESPOND_ERROR("unable to add actor impulse: actor not found");
    }
    auto RootComponent = Cast<UPrimitiveComponent>(ActorView.GetActor()->GetRootComponent());
    if (RootComponent == nullptr)
    {
      RESPOND_ERROR("unable to add actor impulse: not supported by actor");
    }

    UE_LOG(LogCarla, Warning, TEXT("AddImpulseAtLocation: Experimental feature, use carefully."));

    RootComponent->AddImpulseAtLocation(
        impulse.ToCentimeters().ToFVector(),
        location.ToCentimeters().ToFVector(),
        "None");
    return R<void>::Success();
  };

  BIND_SYNC(add_actor_force) << [this](
      cr::ActorId ActorId,
      cr::Vector3D vector) -> R<void>
  {
    REQUIRE_CARLA_EPISODE();
    auto ActorView = Episode->FindActor(ActorId);
    if (!ActorView.IsValid())
    {
      RESPOND_ERROR("unable to add actor impulse: actor not found");
    }
    auto RootComponent = Cast<UPrimitiveComponent>(ActorView.GetActor()->GetRootComponent());
    if (RootComponent == nullptr)
    {
      RESPOND_ERROR("unable to add actor impulse: not supported by actor");
    }
    RootComponent->AddForce(
        vector.ToCentimeters().ToFVector(),
        "None",
        false);
    return R<void>::Success();
  };

  BIND_SYNC(add_actor_force_at_location) << [this](
      cr::ActorId ActorId,
      cr::Vector3D force,
      cr::Vector3D location) -> R<void>
  {
    REQUIRE_CARLA_EPISODE();
    auto ActorView = Episode->FindActor(ActorId);
    if (!ActorView.IsValid())
    {
      RESPOND_ERROR("unable to add actor impulse: actor not found");
    }
    auto RootComponent = Cast<UPrimitiveComponent>(ActorView.GetActor()->GetRootComponent());
    if (RootComponent == nullptr)
    {
      RESPOND_ERROR("unable to add actor impulse: not supported by actor");
    }

    UE_LOG(LogCarla, Warning, TEXT("AddImpulseAtLocation: Experimental feature, use carefully."));

    RootComponent->AddForceAtLocation(
        force.ToCentimeters().ToFVector(),
        location.ToCentimeters().ToFVector(),
        "None");
    return R<void>::Success();
  };

  BIND_SYNC(add_actor_angular_impulse) << [this](
      cr::ActorId ActorId,
      cr::Vector3D vector) -> R<void>
  {
    REQUIRE_CARLA_EPISODE();
    auto ActorView = Episode->FindActor(ActorId);
    if (!ActorView.IsValid())
    {
      RESPOND_ERROR("unable to add actor angular impulse: actor not found");
    }
    auto RootComponent = Cast<UPrimitiveComponent>(ActorView.GetActor()->GetRootComponent());
    if (RootComponent == nullptr)
    {
      RESPOND_ERROR("unable to add actor angular impulse: not supported by actor");
    }
    RootComponent->AddAngularImpulseInDegrees(
        vector.ToFVector(),
        "None",
        false);
    return R<void>::Success();
  };

  BIND_SYNC(add_actor_torque) << [this](
      cr::ActorId ActorId,
      cr::Vector3D vector) -> R<void>
  {
    REQUIRE_CARLA_EPISODE();
    auto ActorView = Episode->FindActor(ActorId);
    if (!ActorView.IsValid())
    {
      RESPOND_ERROR("unable to add actor torque: actor not found");
    }
    auto RootComponent = Cast<UPrimitiveComponent>(ActorView.GetActor()->GetRootComponent());
    if (RootComponent == nullptr)
    {
      RESPOND_ERROR("unable to add actor torque: not supported by actor");
    }
    RootComponent->AddTorqueInDegrees(
        vector.ToFVector(),
        "None",
        false);
    return R<void>::Success();
  };

  BIND_SYNC(get_physics_control) << [this](
      cr::ActorId ActorId) -> R<cr::VehiclePhysicsControl>
  {
    REQUIRE_CARLA_EPISODE();
    auto ActorView = Episode->FindActor(ActorId);
    if (!ActorView.IsValid())
    {
      RESPOND_ERROR("unable to get actor physics control: actor not found");
    }
    auto Vehicle = Cast<ACarlaWheeledVehicle>(ActorView.GetActor());
    if (Vehicle == nullptr)
    {
      RESPOND_ERROR("unable to get actor physics control: actor is not a vehicle");
    }

    return cr::VehiclePhysicsControl(Vehicle->GetVehiclePhysicsControl());
  };

  BIND_SYNC(get_vehicle_light_state) << [this](
      cr::ActorId ActorId) -> R<cr::VehicleLightState>
  {
    REQUIRE_CARLA_EPISODE();
    auto ActorView = Episode->FindActor(ActorId);
    if (!ActorView.IsValid())
    {
      RESPOND_ERROR("unable to get actor physics control: actor not found");
    }
    auto Vehicle = Cast<ACarlaWheeledVehicle>(ActorView.GetActor());
    if (Vehicle == nullptr)
    {
      RESPOND_ERROR("unable to get actor physics control: actor is not a vehicle");
    }

    return cr::VehicleLightState(Vehicle->GetVehicleLightState());
  };

  BIND_SYNC(apply_physics_control) << [this](
      cr::ActorId ActorId,
      cr::VehiclePhysicsControl PhysicsControl) -> R<void>
  {
    REQUIRE_CARLA_EPISODE();
    auto ActorView = Episode->FindActor(ActorId);
    if (!ActorView.IsValid())
    {
      RESPOND_ERROR("unable to apply actor physics control: actor not found");
    }
    auto Vehicle = Cast<ACarlaWheeledVehicle>(ActorView.GetActor());
    if (Vehicle == nullptr)
    {
      RESPOND_ERROR("unable to apply actor physics control: actor is not a vehicle");
    }

    Vehicle->ApplyVehiclePhysicsControl(FVehiclePhysicsControl(PhysicsControl));

    return R<void>::Success();
  };

  BIND_SYNC(set_vehicle_light_state) << [this](
      cr::ActorId ActorId,
      cr::VehicleLightState LightState) -> R<void>
  {
    REQUIRE_CARLA_EPISODE();
    auto ActorView = Episode->FindActor(ActorId);
    if (!ActorView.IsValid())
    {
      RESPOND_ERROR("unable to apply actor light state: actor not found");
    }
    auto Vehicle = Cast<ACarlaWheeledVehicle>(ActorView.GetActor());
    if (Vehicle == nullptr)
    {
      RESPOND_ERROR("unable to apply actor light state: actor is not a vehicle");
    }

    Vehicle->SetVehicleLightState(FVehicleLightState(LightState));

    return R<void>::Success();
  };

  BIND_SYNC(set_actor_simulate_physics) << [this](
      cr::ActorId ActorId,
      bool bEnabled) -> R<void>
  {
    REQUIRE_CARLA_EPISODE();
    auto ActorView = Episode->FindActor(ActorId);
    if (!ActorView.IsValid())
    {
      RESPOND_ERROR("unable to set actor simulate physics: actor not found");
    }

    auto Character = Cast<ACharacter>(ActorView.GetActor());
    // The physics in the walkers works in a different way so to disable them,
    // we need to do it in the UCharacterMovementComponent.
    if (Character != nullptr)
    {
      auto CharacterMovement = Cast<UCharacterMovementComponent>(Character->GetCharacterMovement());

      if(bEnabled) {
        CharacterMovement->SetDefaultMovementMode();
      }
      else {
        CharacterMovement->DisableMovement();
      }
    }
    // In the rest of actors, the physics is controlled with the UPrimitiveComponent, so we use
    // that for disable it.
    else
    {
      auto RootComponent = Cast<UPrimitiveComponent>(ActorView.GetActor()->GetRootComponent());
      if (RootComponent == nullptr)
      {
        RESPOND_ERROR("unable to set actor simulate physics: not supported by actor");
      }
      RootComponent->SetSimulatePhysics(bEnabled);
      RootComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);

      auto Vehicle = Cast<ACarlaWheeledVehicle>(ActorView.GetActor());
      if(Vehicle)
        Vehicle->SetActorEnableCollision(true);
    }

    return R<void>::Success();
  };

  BIND_SYNC(set_actor_enable_gravity) << [this](
      cr::ActorId ActorId,
      bool bEnabled) -> R<void>
  {
    REQUIRE_CARLA_EPISODE();
    auto ActorView = Episode->FindActor(ActorId);
    if (!ActorView.IsValid())
    {
      RESPOND_ERROR("unable to set actor enable gravity: actor not found");
    }

    auto Character = Cast<ACharacter>(ActorView.GetActor());
    // The physics in the walkers works in a different way so to disable them,
    // we need to do it in the UCharacterMovementComponent.
    if (Character != nullptr)
    {
      auto CharacterMovement = Cast<UCharacterMovementComponent>(Character->GetCharacterMovement());

      if(bEnabled) {
        CharacterMovement->SetDefaultMovementMode();
      }
      else {
        if (CharacterMovement->IsFlying() || CharacterMovement->IsFalling())
          CharacterMovement->DisableMovement();
      }
    }
    // In the rest of actors, the physics is controlled with the UPrimitiveComponent, so we use
    // that for disable it.
    else
    {
      auto RootComponent = Cast<UPrimitiveComponent>(ActorView.GetActor()->GetRootComponent());
      if (RootComponent == nullptr)
      {
        RESPOND_ERROR("unable to set actor enable gravity: not supported by actor");
      }
      RootComponent->SetEnableGravity(bEnabled);
    }

    return R<void>::Success();
  };

  // ~~ Apply control ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

  BIND_SYNC(apply_control_to_vehicle) << [this](
      cr::ActorId ActorId,
      cr::VehicleControl Control) -> R<void>
  {
    REQUIRE_CARLA_EPISODE();
    auto ActorView = Episode->FindActor(ActorId);
    if (!ActorView.IsValid())
    {
      RESPOND_ERROR("unable to apply control: actor not found");
    }
    auto Vehicle = Cast<ACarlaWheeledVehicle>(ActorView.GetActor());
    if (Vehicle == nullptr)
    {
      RESPOND_ERROR("unable to apply control: actor is not a vehicle");
    }
    Vehicle->ApplyVehicleControl(Control, EVehicleInputPriority::Client);
    return R<void>::Success();
  };

  BIND_SYNC(apply_control_to_walker) << [this](
      cr::ActorId ActorId,
      cr::WalkerControl Control) -> R<void>
  {
    REQUIRE_CARLA_EPISODE();
    auto ActorView = Episode->FindActor(ActorId);
    if (!ActorView.IsValid())
    {
      RESPOND_ERROR("unable to apply control: actor not found");
    }
    auto Pawn = Cast<APawn>(ActorView.GetActor());
    if (Pawn == nullptr)
    {
      RESPOND_ERROR("unable to apply control: actor is not a walker");
    }
    auto Controller = Cast<AWalkerController>(Pawn->GetController());
    if (Controller == nullptr)
    {
      RESPOND_ERROR("unable to apply control: walker has an incompatible controller");
    }
    Controller->ApplyWalkerControl(Control);
    return R<void>::Success();
  };

  BIND_SYNC(apply_bone_control_to_walker) << [this](
      cr::ActorId ActorId,
      cr::WalkerBoneControl Control) -> R<void>
  {
    REQUIRE_CARLA_EPISODE();
    auto ActorView = Episode->FindActor(ActorId);
    if (!ActorView.IsValid())
    {
      RESPOND_ERROR("unable to apply control: actor not found");
    }
    auto Pawn = Cast<APawn>(ActorView.GetActor());
    if (Pawn == nullptr)
    {
      RESPOND_ERROR("unable to apply control: actor is not a walker");
    }
    auto Controller = Cast<AWalkerController>(Pawn->GetController());
    if (Controller == nullptr)
    {
      RESPOND_ERROR("unable to apply control: walker has an incompatible controller");
    }
    Controller->ApplyWalkerControl(Control);
    return R<void>::Success();
  };

  BIND_SYNC(set_actor_autopilot) << [this](
      cr::ActorId ActorId,
      bool bEnabled) -> R<void>
  {
    REQUIRE_CARLA_EPISODE();
    auto ActorView = Episode->FindActor(ActorId);
    if (!ActorView.IsValid())
    {
      RESPOND_ERROR("unable to set autopilot: actor not found");
    }
    auto Vehicle = Cast<ACarlaWheeledVehicle>(ActorView.GetActor());
    if (Vehicle == nullptr)
    {
      RESPOND_ERROR("unable to set autopilot: actor does not support autopilot");
    }
    auto Controller = Cast<AWheeledVehicleAIController>(Vehicle->GetController());
    if (Controller == nullptr)
    {
      RESPOND_ERROR("unable to set autopilot: vehicle controller does not support autopilot");
    }
    Controller->SetAutopilot(bEnabled);
    return R<void>::Success();
  };

  // ~~ Traffic lights ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

  BIND_SYNC(set_traffic_light_state) << [this](
      cr::ActorId ActorId,
      cr::TrafficLightState trafficLightState) -> R<void>
  {
    REQUIRE_CARLA_EPISODE();
    auto ActorView = Episode->GetActorRegistry().Find(ActorId);
    if (!ActorView.IsValid() || ActorView.GetActor()->IsPendingKill())
    {
      RESPOND_ERROR("unable to set state: actor not found");
    }
    auto TrafficLight = Cast<ATrafficLightBase>(ActorView.GetActor());
    if (TrafficLight == nullptr)
    {
      RESPOND_ERROR("unable to set state: actor is not a traffic light");
    }
    TrafficLight->SetTrafficLightState(static_cast<ETrafficLightState>(trafficLightState));
    return R<void>::Success();
  };

  BIND_SYNC(set_traffic_light_green_time) << [this](
      cr::ActorId ActorId,
      float GreenTime) -> R<void>
  {
    REQUIRE_CARLA_EPISODE();
    auto ActorView = Episode->GetActorRegistry().Find(ActorId);
    if (!ActorView.IsValid() || ActorView.GetActor()->IsPendingKill())
    {
      RESPOND_ERROR("unable to set green time: actor not found");
    }
    auto TrafficLight = Cast<ATrafficLightBase>(ActorView.GetActor());
    if (TrafficLight == nullptr)
    {
      RESPOND_ERROR("unable to set green time: actor is not a traffic light");
    }
    TrafficLight->SetGreenTime(GreenTime);
    return R<void>::Success();
  };

  BIND_SYNC(set_traffic_light_yellow_time) << [this](
      cr::ActorId ActorId,
      float YellowTime) -> R<void>
  {
    REQUIRE_CARLA_EPISODE();
    auto ActorView = Episode->GetActorRegistry().Find(ActorId);
    if (!ActorView.IsValid() || ActorView.GetActor()->IsPendingKill())
    {
      RESPOND_ERROR("unable to set yellow time: actor not found");
    }
    auto TrafficLight = Cast<ATrafficLightBase>(ActorView.GetActor());
    if (TrafficLight == nullptr)
    {
      RESPOND_ERROR("unable to set yellow time: actor is not a traffic light");
    }
    TrafficLight->SetYellowTime(YellowTime);
    return R<void>::Success();
  };

  BIND_SYNC(set_traffic_light_red_time) << [this](
      cr::ActorId ActorId,
      float RedTime) -> R<void>
  {
    REQUIRE_CARLA_EPISODE();
    auto ActorView = Episode->GetActorRegistry().Find(ActorId);
    if (!ActorView.IsValid() || ActorView.GetActor()->IsPendingKill())
    {
      RESPOND_ERROR("unable to set red time: actor not found");
    }
    auto TrafficLight = Cast<ATrafficLightBase>(ActorView.GetActor());
    if (TrafficLight == nullptr)
    {
      RESPOND_ERROR("unable to set red time: actor is not a traffic light");
    }
    TrafficLight->SetRedTime(RedTime);
    return R<void>::Success();
  };

  BIND_SYNC(freeze_traffic_light) << [this](
      cr::ActorId ActorId,
      bool Freeze) -> R<void>
  {
    REQUIRE_CARLA_EPISODE();
    auto ActorView = Episode->GetActorRegistry().Find(ActorId);
    if (!ActorView.IsValid() || ActorView.GetActor()->IsPendingKill())
    {
      RESPOND_ERROR("unable to alter frozen state: actor not found");
    }
    auto TrafficLight = Cast<ATrafficLightBase>(ActorView.GetActor());
    if (TrafficLight == nullptr)
    {
      RESPOND_ERROR("unable to alter frozen state: actor is not a traffic light");
    }
    TrafficLight->SetTimeIsFrozen(Freeze);
    return R<void>::Success();
  };

  BIND_SYNC(reset_traffic_light_group) << [this](
      cr::ActorId ActorId) -> R<void>
  {
    REQUIRE_CARLA_EPISODE();
    auto ActorView = Episode->GetActorRegistry().Find(ActorId);
    if (!ActorView.IsValid() || ActorView.GetActor()->IsPendingKill())
    {
      RESPOND_ERROR("unable to reset traffic lights: actors not found");
    }
    auto TrafficLight = Cast<ATrafficLightBase>(ActorView.GetActor());
    if (TrafficLight == nullptr)
    {
      RESPOND_ERROR("unable to reset traffic lights: actor is not a traffic light");
    }
    TrafficLight->GetTrafficLightComponent()->GetGroup()->ResetGroup();
    return R<void>::Success();
  };

  BIND_SYNC(reset_all_traffic_lights) << [this]() -> R<void>
  {
    REQUIRE_CARLA_EPISODE();
    for (TActorIterator<ATrafficLightGroup> It(Episode->GetWorld()); It; ++It)
    {
      It->ResetGroup();
    }
    return R<void>::Success();
  };

  BIND_SYNC(freeze_all_traffic_lights) << [this]
      (bool frozen) -> R<void>
  {
    REQUIRE_CARLA_EPISODE();
    auto* GameMode = UCarlaStatics::GetGameMode(Episode->GetWorld());
    if (!GameMode)
    {
      RESPOND_ERROR("unable to find CARLA game mode");
    }
    auto* TraffiLightManager = GameMode->GetTrafficLightManager();
    TraffiLightManager->SetFrozen(frozen);
    return R<void>::Success();
  };

  BIND_SYNC(get_vehicle_light_states) << [this]() -> R<cr::VehicleLightStateList>
  {
    REQUIRE_CARLA_EPISODE();
    cr::VehicleLightStateList List;

    auto It = Episode->GetActorRegistry().begin();
    for (; It != Episode->GetActorRegistry().end(); ++It)
    {
      auto Actor = It->GetActor();
      if (!Actor->IsPendingKill() && It->GetActorType() == FActorView::ActorType::Vehicle)
      {
        const ACarlaWheeledVehicle *Vehicle = Cast<ACarlaWheeledVehicle>(Actor);
        List.emplace_back(
            It->GetActorId(),
            cr::VehicleLightState(Vehicle->GetVehicleLightState()).GetLightStateAsValue());
      }
    }

    return List;
  };

  BIND_SYNC(get_group_traffic_lights) << [this](
      const cr::ActorId ActorId) -> R<std::vector<cr::ActorId>>
  {
    REQUIRE_CARLA_EPISODE();
    auto ActorView = Episode->GetActorRegistry().Find(ActorId);
    if (!ActorView.IsValid() || ActorView.GetActor()->IsPendingKill())
    {
      RESPOND_ERROR("unable to get group traffic lights: actor not found");
    }
    auto TrafficLight = Cast<ATrafficLightBase>(ActorView.GetActor());
    if (TrafficLight == nullptr)
    {
      RESPOND_ERROR("unable to get group traffic lights: actor is not a traffic light");
    }
    std::vector<cr::ActorId> Result;
    for (auto TLight : TrafficLight->GetGroupTrafficLights())
    {
      auto View = Episode->FindActor(TLight);
      if (View.IsValid())
      {
        Result.push_back(View.GetActorId());
      }
    }
    return Result;
  };

  // ~~ Logging and playback ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

  BIND_SYNC(start_recorder) << [this](std::string name, bool AdditionalData) -> R<std::string>
  {
    REQUIRE_CARLA_EPISODE();
    return R<std::string>(Episode->StartRecorder(name, AdditionalData));
  };

  BIND_SYNC(stop_recorder) << [this]() -> R<void>
  {
    REQUIRE_CARLA_EPISODE();
    Episode->GetRecorder()->Stop();
    return R<void>::Success();
  };

  BIND_SYNC(show_recorder_file_info) << [this](
      std::string name,
      bool show_all) -> R<std::string>
  {
    REQUIRE_CARLA_EPISODE();
    return R<std::string>(Episode->GetRecorder()->ShowFileInfo(
        name,
        show_all));
  };

  BIND_SYNC(show_recorder_collisions) << [this](
      std::string name,
      char type1,
      char type2) -> R<std::string>
  {
    REQUIRE_CARLA_EPISODE();
    return R<std::string>(Episode->GetRecorder()->ShowFileCollisions(
        name,
        type1,
        type2));
  };

  BIND_SYNC(show_recorder_actors_blocked) << [this](
      std::string name,
      double min_time,
      double min_distance) -> R<std::string>
  {
    REQUIRE_CARLA_EPISODE();
    return R<std::string>(Episode->GetRecorder()->ShowFileActorsBlocked(
        name,
        min_time,
        min_distance));
  };

  BIND_SYNC(replay_file) << [this](
      std::string name,
      double start,
      double duration,
      uint32_t follow_id) -> R<std::string>
  {
    REQUIRE_CARLA_EPISODE();
    return R<std::string>(Episode->GetRecorder()->ReplayFile(
        name,
        start,
        duration,
        follow_id));
  };

  BIND_SYNC(set_replayer_time_factor) << [this](double time_factor) -> R<void>
  {
    REQUIRE_CARLA_EPISODE();
    Episode->GetRecorder()->SetReplayerTimeFactor(time_factor);
    return R<void>::Success();
  };

  BIND_SYNC(set_replayer_ignore_hero) << [this](bool ignore_hero) -> R<void>
  {
    REQUIRE_CARLA_EPISODE();
    Episode->GetRecorder()->SetReplayerIgnoreHero(ignore_hero);
    return R<void>::Success();
  };

  BIND_SYNC(stop_replayer) << [this](bool keep_actors) -> R<void>
  {
    REQUIRE_CARLA_EPISODE();
    Episode->GetRecorder()->StopReplayer(keep_actors);
    return R<void>::Success();
  };

  // ~~ Draw debug shapes ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

  BIND_SYNC(draw_debug_shape) << [this](const cr::DebugShape &shape) -> R<void>
  {
    REQUIRE_CARLA_EPISODE();
    auto *World = Episode->GetWorld();
    check(World != nullptr);
    FDebugShapeDrawer Drawer(*World);
    Drawer.Draw(shape);
    return R<void>::Success();
  };

  // ~~ Apply commands in batch ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

  using C = cr::Command;
  using CR = cr::CommandResponse;
  using ActorId = carla::ActorId;

  auto parse_result = [](ActorId id, const auto &response) {
    return response.HasError() ? CR{response.GetError()} : CR{id};
  };

#define MAKE_RESULT(operation) return parse_result(c.actor, operation);

  auto command_visitor = carla::Functional::MakeRecursiveOverload(
      [=](auto self, const C::SpawnActor &c) -> CR {
        auto result = c.parent.has_value() ?
        spawn_actor_with_parent(
            c.description,
            c.transform,
            *c.parent,
            cr::AttachmentType::Rigid) :
        spawn_actor(c.description, c.transform);
        if (!result.HasError())
        {
          ActorId id = result.Get().id;
          auto set_id = carla::Functional::MakeOverload(
              [](C::SpawnActor &) {},
              [id](auto &s) { s.actor = id; });
          for (auto command : c.do_after)
          {
            boost::apply_visitor(set_id, command.command);
            boost::apply_visitor(self, command.command);
          }
          return id;
        }
        return result.GetError();
      },
      [=](auto, const C::DestroyActor &c) {         MAKE_RESULT(destroy_actor(c.actor)); },
      [=](auto, const C::ApplyVehicleControl &c) {  MAKE_RESULT(apply_control_to_vehicle(c.actor, c.control)); },
      [=](auto, const C::ApplyWalkerControl &c) {   MAKE_RESULT(apply_control_to_walker(c.actor, c.control)); },
      [=](auto, const C::ApplyTransform &c) {       MAKE_RESULT(set_actor_transform(c.actor, c.transform)); },
      [=](auto, const C::ApplyTargetVelocity &c) {  MAKE_RESULT(set_actor_target_velocity(c.actor, c.velocity)); },
      [=](auto, const C::ApplyTargetAngularVelocity &c) { MAKE_RESULT(set_actor_target_angular_velocity(c.actor, c.angular_velocity)); },
      [=](auto, const C::ApplyImpulse &c) {         MAKE_RESULT(add_actor_impulse(c.actor, c.impulse)); },
      [=](auto, const C::ApplyForce &c) {           MAKE_RESULT(add_actor_force(c.actor, c.force)); },
      [=](auto, const C::ApplyAngularImpulse &c) {  MAKE_RESULT(add_actor_angular_impulse(c.actor, c.impulse)); },
      [=](auto, const C::ApplyTorque &c) {          MAKE_RESULT(add_actor_torque(c.actor, c.torque)); },
      [=](auto, const C::SetSimulatePhysics &c) {   MAKE_RESULT(set_actor_simulate_physics(c.actor, c.enabled)); },
      [=](auto, const C::SetEnableGravity &c) {   MAKE_RESULT(set_actor_enable_gravity(c.actor, c.enabled)); },
      // TODO: SetAutopilot should be removed. This is the old way to control the vehicles
      [=](auto, const C::SetAutopilot &c) {         MAKE_RESULT(set_actor_autopilot(c.actor, c.enabled)); },
      [=](auto, const C::SetVehicleLightState &c) { MAKE_RESULT(set_vehicle_light_state(c.actor, c.light_state)); },
      [=](auto, const C::ApplyWalkerState &c) {     MAKE_RESULT(set_walker_state(c.actor, c.transform, c.speed)); });

#undef MAKE_RESULT

  BIND_SYNC(apply_batch) << [=](
      const std::vector<cr::Command> &commands,
      bool do_tick_cue)
  {
    std::vector<CR> result;
    result.reserve(commands.size());
    for (const auto &command : commands)
    {
      result.emplace_back(boost::apply_visitor(command_visitor, command.command));
    }
    if (do_tick_cue)
    {
      tick_cue();
    }
    return result;
  };

  // ~~ Light Subsystem ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

  BIND_SYNC(query_lights_state) << [this](std::string client) -> R<std::vector<cr::LightState>>
  {
    REQUIRE_CARLA_EPISODE();
    std::vector<cr::LightState> result;
    auto *World = Episode->GetWorld();
    if(World) {
      UCarlaLightSubsystem* CarlaLightSubsystem = World->GetSubsystem<UCarlaLightSubsystem>();
      result = CarlaLightSubsystem->GetLights(FString(client.c_str()));
    }
    return result;
  };

  BIND_SYNC(update_lights_state) << [this]
    (std::string client, const std::vector<cr::LightState>& lights, bool discard_client) -> R<void>
  {
    REQUIRE_CARLA_EPISODE();
    auto *World = Episode->GetWorld();
    if(World) {
      UCarlaLightSubsystem* CarlaLightSubsystem = World->GetSubsystem<UCarlaLightSubsystem>();
      CarlaLightSubsystem->SetLights(FString(client.c_str()), lights, discard_client);
    }
    return R<void>::Success();
  };


  // ~~ Ray Casting ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

  BIND_SYNC(project_point) << [this]
      (cr::Location Location, cr::Vector3D Direction, float SearchDistance)
      -> R<std::pair<bool,cr::LabelledPoint>>
  {
    REQUIRE_CARLA_EPISODE();
    auto *World = Episode->GetWorld();
    constexpr float meter_to_centimeter = 100.0f;
    return URayTracer::ProjectPoint(Location, Direction.ToFVector(),
        meter_to_centimeter * SearchDistance, World);
  };

  BIND_SYNC(cast_ray) << [this]
      (cr::Location StartLocation, cr::Location EndLocation)
      -> R<std::vector<cr::LabelledPoint>>
  {
    REQUIRE_CARLA_EPISODE();
    auto *World = Episode->GetWorld();
    return URayTracer::CastRay(StartLocation, EndLocation, World);
  };

}

// =============================================================================
// -- Undef helper macros ------------------------------------------------------
// =============================================================================

#undef BIND_ASYNC
#undef BIND_SYNC
#undef REQUIRE_CARLA_EPISODE
#undef RESPOND_ERROR_FSTRING
#undef RESPOND_ERROR
#undef CARLA_ENSURE_GAME_THREAD

// =============================================================================
// -- FCarlaServer -------------------------------------------------------
// =============================================================================

FCarlaServer::FCarlaServer() : Pimpl(nullptr) {}

FCarlaServer::~FCarlaServer() {}

FDataMultiStream FCarlaServer::Start(uint16_t RPCPort, uint16_t StreamingPort)
{
  Pimpl = MakeUnique<FPimpl>(RPCPort, StreamingPort);
  StreamingPort = Pimpl->StreamingServer.GetLocalEndpoint().port();
  UE_LOG(
      LogCarlaServer,
      Log,
      TEXT("Initialized CarlaServer: Ports(rpc=%d, streaming=%d)"),
      RPCPort,
      StreamingPort);
  return Pimpl->BroadcastStream;
}

void FCarlaServer::NotifyBeginEpisode(UCarlaEpisode &Episode)
{
  check(Pimpl != nullptr);
  UE_LOG(LogCarlaServer, Log, TEXT("New episode '%s' started"), *Episode.GetMapName());
  Pimpl->Episode = &Episode;
}

void FCarlaServer::NotifyEndEpisode()
{
  check(Pimpl != nullptr);
  Pimpl->Episode = nullptr;
}

void FCarlaServer::AsyncRun(uint32 NumberOfWorkerThreads)
{
  check(Pimpl != nullptr);
  /// @todo Define better the number of threads each server gets.
  auto RPCThreads = NumberOfWorkerThreads / 2u;
  auto StreamingThreads = NumberOfWorkerThreads - RPCThreads;
  Pimpl->Server.AsyncRun(std::max(2u, RPCThreads));
  Pimpl->StreamingServer.AsyncRun(std::max(2u, StreamingThreads));
}

void FCarlaServer::RunSome(uint32 Milliseconds)
{
  Pimpl->Server.SyncRunFor(carla::time_duration::milliseconds(Milliseconds));
}

bool FCarlaServer::TickCueReceived()
{
  if (Pimpl->TickCuesReceived > 0u)
  {
    --(Pimpl->TickCuesReceived);
    return true;
  }
  return false;
}

void FCarlaServer::Stop()
{
  check(Pimpl != nullptr);
  Pimpl->Server.Stop();
}

FDataStream FCarlaServer::OpenStream() const
{
  check(Pimpl != nullptr);
  return Pimpl->StreamingServer.MakeStream();
}
