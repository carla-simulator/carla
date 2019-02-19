// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "Carla.h"
#include "Carla/Server/TheNewCarlaServer.h"

#include "Carla/Util/DebugShapeDrawer.h"
#include "Carla/Util/OpenDrive.h"
#include "Carla/Vehicle/CarlaWheeledVehicle.h"
#include "Carla/Walker/WalkerController.h"

#include <compiler/disable-ue4-macros.h>
#include <carla/Version.h>
#include <carla/rpc/Actor.h>
#include <carla/rpc/ActorDefinition.h>
#include <carla/rpc/ActorDescription.h>
#include <carla/rpc/DebugShape.h>
#include <carla/rpc/EpisodeInfo.h>
#include <carla/rpc/MapInfo.h>
#include <carla/rpc/Response.h>
#include <carla/rpc/Server.h>
#include <carla/rpc/Transform.h>
#include <carla/rpc/Vector2D.h>
#include <carla/rpc/Vector3D.h>
#include <carla/rpc/VehicleControl.h>
#include <carla/rpc/VehiclePhysicsControl.h>
#include <carla/rpc/WalkerControl.h>
#include <carla/rpc/WeatherParameters.h>
#include <carla/streaming/Server.h>
#include <compiler/enable-ue4-macros.h>

#include <vector>

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
// -- FTheNewCarlaServer::FPimpl -----------------------------------------------
// =============================================================================

class FTheNewCarlaServer::FPimpl
{
public:

  FPimpl(uint16_t port)
    : Server(port),
      StreamingServer(port + 1u)
  {
    BindActions();
  }

  carla::rpc::Server Server;

  carla::streaming::Server StreamingServer;

  UCarlaEpisode *Episode = nullptr;

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

// =============================================================================
// -- Bind Actions -------------------------------------------------------------
// =============================================================================

void FTheNewCarlaServer::FPimpl::BindActions()
{
  namespace cr = carla::rpc;
  namespace cg = carla::geom;

  Server.BindAsync("version", []() -> R<std::string>
  {
    return carla::version();
  });

  Server.BindSync("get_episode_info", [this]() -> R<cr::EpisodeInfo>
  {
    REQUIRE_CARLA_EPISODE();
    auto WorldObserver = Episode->GetWorldObserver();
    if (WorldObserver == nullptr)
    {
      RESPOND_ERROR("internal error: missing world observer");
    }
    return cr::EpisodeInfo{
      Episode->GetId(),
      cr::FromFString(Episode->GetMapName()),
      WorldObserver->GetToken()};
  });

  Server.BindSync("get_map_info", [this]() -> R<cr::MapInfo>
  {
    REQUIRE_CARLA_EPISODE();
    auto FileContents = FOpenDrive::Load(Episode->GetMapName());
    const auto &SpawnPoints = Episode->GetRecommendedSpawnPoints();
    return cr::MapInfo{
      cr::FromFString(Episode->GetMapName()),
      cr::FromFString(FileContents),
      MakeVectorFromTArray<cg::Transform>(SpawnPoints)};
  });

  Server.BindSync("get_actor_definitions", [this]() -> R<std::vector<cr::ActorDefinition>>
  {
    REQUIRE_CARLA_EPISODE();
    return MakeVectorFromTArray<cr::ActorDefinition>(Episode->GetActorDefinitions());
  });

  Server.BindSync("get_spectator", [this]() -> R<cr::Actor> {
    REQUIRE_CARLA_EPISODE();
    auto ActorView = Episode->FindActor(Episode->GetSpectatorPawn());
    if (!ActorView.IsValid())
    {
      RESPOND_ERROR("internal error: unable to find spectator");
    }
    return Episode->SerializeActor(ActorView);
  });

  Server.BindSync("get_weather_parameters", [this]() -> R<cr::WeatherParameters>
  {
    REQUIRE_CARLA_EPISODE();
    auto *Weather = Episode->GetWeather();
    if (Weather == nullptr)
    {
      RESPOND_ERROR("internal error: unable to find weather");
    }
    return Weather->GetCurrentWeather();
  });

  Server.BindSync("set_weather_parameters", [this](
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
  });

  Server.BindSync("get_actors_by_id", [this](
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
  });

  Server.BindSync("spawn_actor", [this](
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
  });

  Server.BindSync("spawn_actor_with_parent", [this](
        cr::ActorDescription Description,
        const cr::Transform &Transform,
        cr::Actor Parent) -> R<cr::Actor>
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
    auto ParentActorView = Episode->FindActor(Parent.id);
    if (!ParentActorView.IsValid())
    {
      RESPOND_ERROR("unable to attach actor: parent actor not found");
    }
    Episode->AttachActors(Result.Value.GetActor(), ParentActorView.GetActor());
    return Episode->SerializeActor(Result.Value);
  });

  Server.BindSync("destroy_actor", [this](cr::Actor Actor) -> R<void>
  {
    REQUIRE_CARLA_EPISODE();
    auto ActorView = Episode->FindActor(Actor.id);
    if (!ActorView.IsValid())
    {
      RESPOND_ERROR("unable to destroy actor: not found");
    }
    if (!Episode->DestroyActor(ActorView.GetActor()))
    {
      RESPOND_ERROR("internal error: unable to destroy actor");
    }
    return R<void>::Success();
  });

  Server.BindSync("attach_actors", [this](
        cr::Actor Child,
        cr::Actor Parent) -> R<void>
  {
    REQUIRE_CARLA_EPISODE();
    auto ChildView = Episode->FindActor(Child.id);
    if (!ChildView.IsValid())
    {
      RESPOND_ERROR("unable to attach actor: child actor not found");
    }
    auto ParentView = Episode->FindActor(Parent.id);
    if (!ParentView.IsValid())
    {
      RESPOND_ERROR("unable to attach actor: parent actor not found");
    }
    Episode->AttachActors(ChildView.GetActor(), ParentView.GetActor());
    return R<void>::Success();
  });

  Server.BindSync("set_actor_location", [this](
        cr::Actor Actor,
        cr::Location Location) -> R<void>
  {
    REQUIRE_CARLA_EPISODE();
    auto ActorView = Episode->FindActor(Actor.id);
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
  });

  Server.BindSync("set_actor_transform", [this](
        cr::Actor Actor,
        cr::Transform Transform) -> R<void>
  {
    REQUIRE_CARLA_EPISODE();
    auto ActorView = Episode->FindActor(Actor.id);
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
  });

  Server.BindSync("set_actor_velocity", [this](
        cr::Actor Actor,
        cr::Vector3D vector) -> R<void>
  {
    REQUIRE_CARLA_EPISODE();
    auto ActorView = Episode->FindActor(Actor.id);
    if (!ActorView.IsValid())
    {
      RESPOND_ERROR("unable to set actor velocity: actor not found");
    }
    auto RootComponent = Cast<UPrimitiveComponent>(ActorView.GetActor()->GetRootComponent());
    if (RootComponent == nullptr)
    {
      RESPOND_ERROR("unable to set actor velocity: not supported by actor");
    }
    RootComponent->SetPhysicsLinearVelocity(
    vector.ToCentimeters(),
    false,
    "None");
    return R<void>::Success();
  });

  Server.BindSync("set_actor_angular_velocity", [this](
        cr::Actor Actor,
        cr::Vector3D vector) -> R<void>
  {
    REQUIRE_CARLA_EPISODE();
    auto ActorView = Episode->FindActor(Actor.id);
    if (!ActorView.IsValid())
    {
      RESPOND_ERROR("unable to set actor angular velocity: actor not found");
    }
    auto RootComponent = Cast<UPrimitiveComponent>(ActorView.GetActor()->GetRootComponent());
    if (RootComponent == nullptr)
    {
      RESPOND_ERROR("unable to set actor angular velocity: not supported by actor");
    }
    RootComponent->SetPhysicsAngularVelocityInDegrees(
    vector,
    false,
    "None");
    return R<void>::Success();
  });

  Server.BindSync("add_actor_impulse", [this](
        cr::Actor Actor,
        cr::Vector3D vector) -> R<void>
  {
    REQUIRE_CARLA_EPISODE();
    auto ActorView = Episode->FindActor(Actor.id);
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
    vector.ToCentimeters(),
    "None",
    false);
    return R<void>::Success();
  });


 Server.BindSync("get_physics_control", [this](
        int ActorId) -> R<cr::VehiclePhysicsControl>
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
    
    return cr::VehiclePhysicsControl(Vehicle->GetVehiclePhysicsControl());
  });

  Server.BindSync("set_physics_control", [this](
        int ActorId, cr::VehiclePhysicsControl PhysicsControl) -> R<void>
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
    
    Vehicle->SetVehiclePhysicsControl(FVehiclePhysicsControl(PhysicsControl));

    return R<void>::Success();
  });

  Server.BindSync("set_actor_simulate_physics", [this](
        cr::Actor Actor,
        bool bEnabled) -> R<void>
  {
    REQUIRE_CARLA_EPISODE();
    auto ActorView = Episode->FindActor(Actor.id);
    if (!ActorView.IsValid())
    {
      RESPOND_ERROR("unable to set actor simulate physics: actor not found");
    }
    auto RootComponent = Cast<UPrimitiveComponent>(ActorView.GetActor()->GetRootComponent());
    if (RootComponent == nullptr)
    {
      RESPOND_ERROR("unable to set actor simulate physics: not supported by actor");
    }
    RootComponent->SetSimulatePhysics(bEnabled);
    return R<void>::Success();
  });

  Server.BindSync("apply_control_to_vehicle", [this](
        cr::Actor Actor,
        cr::VehicleControl Control) -> R<void>
  {
    REQUIRE_CARLA_EPISODE();
    auto ActorView = Episode->FindActor(Actor.id);
    if (!ActorView.IsValid())
    {
      RESPOND_ERROR("unable to apply control: actor not found");
    }
    auto Vehicle = Cast<ACarlaWheeledVehicle>(ActorView.GetActor());
    if (Vehicle == nullptr)
    {
      RESPOND_ERROR("unable to apply control: actor is not a vehicle");
    }
    Vehicle->ApplyVehicleControl(Control);
    return R<void>::Success();
  });

  Server.BindSync("apply_control_to_walker", [this](
        cr::Actor Actor,
        cr::WalkerControl Control) -> R<void>
  {
    REQUIRE_CARLA_EPISODE();
    auto ActorView = Episode->FindActor(Actor.id);
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
  });

  Server.BindSync("set_actor_autopilot", [this](
        cr::Actor Actor,
        bool bEnabled) -> R<void>
  {
    REQUIRE_CARLA_EPISODE();
    auto ActorView = Episode->FindActor(Actor.id);
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
  });

  Server.BindSync("set_traffic_light_state", [this](
        cr::Actor Actor,
        cr::TrafficLightState trafficLightState) -> R<void>
  {
    REQUIRE_CARLA_EPISODE();
    auto ActorView = Episode->GetActorRegistry().Find(Actor.id);
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
  });

  Server.BindSync("set_traffic_light_green_time", [this](
        cr::Actor Actor,
        float GreenTime) -> R<void>
  {
    REQUIRE_CARLA_EPISODE();
    auto ActorView = Episode->GetActorRegistry().Find(Actor.id);
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
  });

  Server.BindSync("set_traffic_light_yellow_time", [this](
        cr::Actor Actor,
        float YellowTime) -> R<void>
  {
    REQUIRE_CARLA_EPISODE();
    auto ActorView = Episode->GetActorRegistry().Find(Actor.id);
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
  });

  Server.BindSync("set_traffic_light_red_time", [this](
        cr::Actor Actor,
        float RedTime) -> R<void>
  {
    REQUIRE_CARLA_EPISODE();
    auto ActorView = Episode->GetActorRegistry().Find(Actor.id);
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
  });

  Server.BindSync("freeze_traffic_light", [this](
        cr::Actor Actor,
        bool Freeze) -> R<void>
  {
    REQUIRE_CARLA_EPISODE();
    auto ActorView = Episode->GetActorRegistry().Find(Actor.id);
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
  });

  Server.BindSync("draw_debug_shape", [this](const cr::DebugShape &shape) -> R<void>
  {
    REQUIRE_CARLA_EPISODE();
    auto *World = Episode->GetWorld();
    check(World != nullptr);
    FDebugShapeDrawer Drawer(*World);
    Drawer.Draw(shape);
    return R<void>::Success();
  });
}

// =============================================================================
// -- Undef helper macros ------------------------------------------------------
// =============================================================================

#undef REQUIRE_CARLA_EPISODE
#undef RESPOND_ERROR_FSTRING
#undef RESPOND_ERROR
#undef CARLA_ENSURE_GAME_THREAD

// =============================================================================
// -- FTheNewCarlaServer -------------------------------------------------------
// =============================================================================

FTheNewCarlaServer::FTheNewCarlaServer() : Pimpl(nullptr) {}

FTheNewCarlaServer::~FTheNewCarlaServer() {}

void FTheNewCarlaServer::Start(uint16_t Port)
{
  UE_LOG(LogCarlaServer, Log, TEXT("Initializing rpc-server at port %d"), Port);
  Pimpl = MakeUnique<FPimpl>(Port);
}

void FTheNewCarlaServer::NotifyBeginEpisode(UCarlaEpisode &Episode)
{
  check(Pimpl != nullptr);
  UE_LOG(LogCarlaServer, Log, TEXT("New episode '%s' started"), *Episode.GetMapName());
  Pimpl->Episode = &Episode;
}

void FTheNewCarlaServer::NotifyEndEpisode()
{
  check(Pimpl != nullptr);
  Pimpl->Episode = nullptr;
}

void FTheNewCarlaServer::AsyncRun(uint32 NumberOfWorkerThreads)
{
  check(Pimpl != nullptr);
  /// @todo Define better the number of threads each server gets.
  auto RPCThreads = NumberOfWorkerThreads / 2u;
  auto StreamingThreads = NumberOfWorkerThreads - RPCThreads;
  Pimpl->Server.AsyncRun(std::max(2u, RPCThreads));
  Pimpl->StreamingServer.AsyncRun(std::max(2u, StreamingThreads));
}

void FTheNewCarlaServer::RunSome(uint32 Milliseconds)
{
  Pimpl->Server.SyncRunFor(carla::time_duration::milliseconds(Milliseconds));
}

void FTheNewCarlaServer::Stop()
{
  check(Pimpl != nullptr);
  Pimpl->Server.Stop();
}

FDataStream FTheNewCarlaServer::OpenStream() const
{
  check(Pimpl != nullptr);
  return Pimpl->StreamingServer.MakeStream();
}

FDataMultiStream FTheNewCarlaServer::OpenMultiStream() const
{
  check(Pimpl != nullptr);
  return Pimpl->StreamingServer.MakeMultiStream();
}
