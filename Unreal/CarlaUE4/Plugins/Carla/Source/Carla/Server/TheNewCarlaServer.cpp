// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "Carla.h"
#include "Carla/Server/TheNewCarlaServer.h"

#include "Carla/Sensor/Sensor.h"
#include "Carla/Util/DebugShapeDrawer.h"
#include "Carla/Util/OpenDrive.h"
#include "Carla/Vehicle/CarlaWheeledVehicle.h"
#include "Carla/Walker/WalkerController.h"

#include "GameFramework/SpectatorPawn.h"

#include <compiler/disable-ue4-macros.h>
#include <carla/Version.h>
#include <carla/rpc/Actor.h>
#include <carla/rpc/ActorDefinition.h>
#include <carla/rpc/ActorDescription.h>
#include <carla/rpc/DebugShape.h>
#include <carla/rpc/EpisodeInfo.h>
#include <carla/rpc/MapInfo.h>
#include <carla/rpc/Server.h>
#include <carla/rpc/Transform.h>
#include <carla/rpc/VehicleControl.h>
#include <carla/rpc/WalkerControl.h>
#include <carla/rpc/WeatherParameters.h>
#include <carla/streaming/Server.h>
#include <compiler/enable-ue4-macros.h>

#include <vector>

// =============================================================================
// -- Static local functions ---------------------------------------------------
// =============================================================================

template <typename T, typename Other>
static std::vector<T> MakeVectorFromTArray(const TArray<Other> &Array)
{
  return {Array.GetData(), Array.GetData() + Array.Num()};
}

static void AttachActors(AActor *Child, AActor *Parent)
{
  Child->AttachToActor(Parent, FAttachmentTransformRules::KeepRelativeTransform);
  Child->SetOwner(Parent);
}

// =============================================================================
// -- FTheNewCarlaServer::FPimpl -----------------------------------------------
// =============================================================================

class FTheNewCarlaServer::FPimpl
{
public:

  FPimpl(uint16_t port)
    : Server(port),
      StreamingServer(port + 1u) {
    BindActions();
  }

  carla::rpc::Server Server;

  carla::streaming::Server StreamingServer;

  UCarlaEpisode *Episode = nullptr;

private:

  void BindActions();

  void RespondErrorStr(const std::string &ErrorMessage) {
    UE_LOG(LogCarlaServer, Log, TEXT("Responding error, %s"), *carla::rpc::ToFString(ErrorMessage));
    carla::rpc::Server::RespondError(ErrorMessage);
  }

  void RespondError(const FString &ErrorMessage) {
    RespondErrorStr(carla::rpc::FromFString(ErrorMessage));
  }

  void RequireEpisode()
  {
    if (Episode == nullptr)
    {
      RespondErrorStr("episode not ready");
    }
  }

  auto SpawnActor(const FTransform &Transform, FActorDescription Description)
  {
    auto Result = Episode->SpawnActorWithInfo(Transform, std::move(Description));
    if (Result.Key != EActorSpawnResultStatus::Success)
    {
      RespondError(FActorSpawnResult::StatusToString(Result.Key));
    }
    check(Result.Value.IsValid());
    return Result.Value;
  }

  void AttachActors(FActorView Child, FActorView Parent)
  {
    if (!Child.IsValid())
    {
      RespondErrorStr("unable to attach actor: child actor not found");
    }
    if (!Parent.IsValid())
    {
      RespondErrorStr("unable to attach actor: parent actor not found");
    }
    ::AttachActors(Child.GetActor(), Parent.GetActor());
  }

  carla::geom::BoundingBox GetActorBoundingBox(const AActor &Actor)
  {
    /// @todo Bounding boxes only available for vehicles.
    auto Vehicle = Cast<ACarlaWheeledVehicle>(&Actor);
    if (Vehicle != nullptr)
    {
      FVector Location = Vehicle->GetVehicleBoundingBoxTransform().GetTranslation();
      FVector Extent = Vehicle->GetVehicleBoundingBoxExtent();
      return {Location, Extent};
    }
    return {};
  }

public:

  carla::rpc::Actor SerializeActor(FActorView ActorView)
  {
    carla::rpc::Actor Actor;
    Actor.id = ActorView.GetActorId();
    if (ActorView.IsValid())
    {
      Actor.parent_id = Episode->GetActorRegistry().Find(ActorView.GetActor()->GetOwner()).GetActorId();
      Actor.description = *ActorView.GetActorDescription();
      Actor.bounding_box = GetActorBoundingBox(*ActorView.GetActor());
      Actor.semantic_tags.reserve(ActorView.GetSemanticTags().Num());
      using tag_t = decltype(Actor.semantic_tags)::value_type;
      for (auto &&Tag : ActorView.GetSemanticTags())
      {
        Actor.semantic_tags.emplace_back(static_cast<tag_t>(Tag));
      }
      auto *Sensor = Cast<ASensor>(ActorView.GetActor());
      if (Sensor != nullptr)
      {
        auto Stream = GetSensorStream(ActorView, *Sensor);
        const auto &Token = Stream.token();
        Actor.stream_token = decltype(Actor.stream_token)(std::begin(Token.data), std::end(Token.data));
      }
    } else {
      UE_LOG(LogCarla, Warning, TEXT("Trying to serialize invalid actor"));
    }
    return Actor;
  }

private:

  carla::streaming::Stream GetSensorStream(FActorView ActorView, ASensor &Sensor) {
    auto id = ActorView.GetActorId();
    auto it = _StreamMap.find(id);
    if (it == _StreamMap.end()) {
      UE_LOG(LogCarlaServer, Log, TEXT("Making a new sensor stream for '%s'"), *ActorView.GetActorDescription()->Id);
      auto result = _StreamMap.emplace(id, StreamingServer.MakeStream());
      check(result.second);
      it = result.first;
      Sensor.SetDataStream(it->second);
    }
    return it->second;
  }

  std::unordered_map<FActorView::IdType, carla::streaming::Stream> _StreamMap;
};

// =============================================================================
// -- FTheNewCarlaServer::FPimpl Bind Actions ----------------------------------
// =============================================================================

void FTheNewCarlaServer::FPimpl::BindActions()
{
  namespace cr = carla::rpc;

  Server.BindAsync("ping", []() { return true; });

  Server.BindAsync("version", []() -> std::string { return carla::version(); });

  Server.BindSync("get_episode_info", [this]() -> cr::EpisodeInfo {
    RequireEpisode();
    auto WorldObserver = Episode->GetWorldObserver();
    if (WorldObserver == nullptr) {
      WorldObserver = Episode->StartWorldObserver(StreamingServer.MakeMultiStream());
    }
    return {Episode->GetId(), cr::FromFString(Episode->GetMapName()), WorldObserver->GetStreamToken()};
  });

  Server.BindSync("get_map_info", [this]() -> cr::MapInfo {
    RequireEpisode();
    auto FileContents = FOpenDrive::Load(Episode->GetMapName());
    const auto &SpawnPoints = Episode->GetRecommendedStartTransforms();
    std::vector<carla::geom::Transform> spawn_points;
    spawn_points.reserve(SpawnPoints.Num());
    for (const auto &Transform : SpawnPoints)
    {
      spawn_points.emplace_back(Transform);
    }
    return {
        cr::FromFString(Episode->GetMapName()),
        cr::FromFString(FileContents),
        spawn_points};
  });

  Server.BindSync("get_actor_definitions", [this]() {
    RequireEpisode();
    return MakeVectorFromTArray<cr::ActorDefinition>(Episode->GetActorDefinitions());
  });

  Server.BindSync("get_spectator", [this]() -> cr::Actor {
    RequireEpisode();
    auto ActorView = Episode->GetActorRegistry().Find(Episode->GetSpectatorPawn());
    if (!ActorView.IsValid() || ActorView.GetActor()->IsPendingKill()) {
      RespondErrorStr("unable to find spectator");
    }
    return SerializeActor(ActorView);
  });

  Server.BindSync("get_weather_parameters", [this]() -> cr::WeatherParameters {
    RequireEpisode();
    auto *Weather = Episode->GetWeather();
    if (Weather == nullptr) {
      RespondErrorStr("unable to find weather");
    }
    return Weather->GetCurrentWeather();
  });

  Server.BindSync("set_weather_parameters", [this](const cr::WeatherParameters &weather) {
    RequireEpisode();
    auto *Weather = Episode->GetWeather();
    if (Weather == nullptr) {
      RespondErrorStr("unable to find weather");
    }
    Weather->ApplyWeather(weather);
  });

  Server.BindSync("get_actors_by_id", [this](const std::vector<FActorView::IdType> &ids) {
    RequireEpisode();
    std::vector<cr::Actor> Result;
    Result.reserve(ids.size());
    const auto &Registry = Episode->GetActorRegistry();
    for (auto &&Id : ids) {
      auto View = Registry.Find(Id);
      if (View.IsValid()) {
        Result.emplace_back(SerializeActor(View));
      }
    }
    return Result;
  });

  Server.BindSync("spawn_actor", [this](
      cr::ActorDescription Description,
      const cr::Transform &Transform) -> cr::Actor {
    RequireEpisode();
    return SerializeActor(SpawnActor(Transform, Description));
  });

  Server.BindSync("spawn_actor_with_parent", [this](
      cr::ActorDescription Description,
      const cr::Transform &Transform,
      cr::Actor Parent) -> cr::Actor {
    RequireEpisode();
    auto ActorView = SpawnActor(Transform, Description);
    auto ParentActorView = Episode->GetActorRegistry().Find(Parent.id);
    AttachActors(ActorView, ParentActorView);
    return SerializeActor(ActorView);
  });

  Server.BindSync("destroy_actor", [this](cr::Actor Actor) {
    RequireEpisode();
    auto ActorView = Episode->GetActorRegistry().Find(Actor.id);
    if (!ActorView.IsValid()) {
      UE_LOG(LogCarlaServer, Warning, TEXT("unable to destroy actor: not found"));
      return false;
    }
    return Episode->DestroyActor(ActorView.GetActor());
  });

  Server.BindSync("attach_actors", [this](cr::Actor Child, cr::Actor Parent) {
    RequireEpisode();
    auto &Registry = Episode->GetActorRegistry();
    AttachActors(Registry.Find(Child.id), Registry.Find(Parent.id));
  });

  Server.BindSync("get_actor_location", [this](cr::Actor Actor) -> cr::Location {
    RequireEpisode();
    auto ActorView = Episode->GetActorRegistry().Find(Actor.id);
    if (!ActorView.IsValid() || ActorView.GetActor()->IsPendingKill()) {
      RespondErrorStr("unable to get actor location: actor not found");
    }
    return {ActorView.GetActor()->GetActorLocation()};
  });

  Server.BindSync("get_actor_transform", [this](cr::Actor Actor) -> cr::Transform {
    RequireEpisode();
    auto ActorView = Episode->GetActorRegistry().Find(Actor.id);
    if (!ActorView.IsValid() || ActorView.GetActor()->IsPendingKill()) {
      RespondErrorStr("unable to get actor transform: actor not found");
    }
    return {ActorView.GetActor()->GetActorTransform()};
  });

  Server.BindSync("set_actor_location", [this](
      cr::Actor Actor,
      cr::Location Location) {
    RequireEpisode();
    auto ActorView = Episode->GetActorRegistry().Find(Actor.id);
    if (!ActorView.IsValid() || ActorView.GetActor()->IsPendingKill()) {
      RespondErrorStr("unable to set actor location: actor not found");
    }
    // This function only works with teleport physics, to reset speeds we need
    // another method.
    /// @todo print error instead of returning false.
    ActorView.GetActor()->SetActorRelativeLocation(
        Location,
        false,
        nullptr,
        ETeleportType::TeleportPhysics);
  });

  Server.BindSync("set_actor_transform", [this](
      cr::Actor Actor,
      cr::Transform Transform) {
    RequireEpisode();
    auto ActorView = Episode->GetActorRegistry().Find(Actor.id);
    if (!ActorView.IsValid() || ActorView.GetActor()->IsPendingKill()) {
      RespondErrorStr("unable to set actor transform: actor not found");
    }
    // This function only works with teleport physics, to reset speeds we need
    // another method.
    ActorView.GetActor()->SetActorRelativeTransform(
        Transform,
        false,
        nullptr,
        ETeleportType::TeleportPhysics);
  });

  Server.BindSync("set_actor_simulate_physics", [this](cr::Actor Actor, bool bEnabled) {
    RequireEpisode();
    auto ActorView = Episode->GetActorRegistry().Find(Actor.id);
    if (!ActorView.IsValid() || ActorView.GetActor()->IsPendingKill()) {
      RespondErrorStr("unable to set actor simulate physics: actor not found");
    }
    auto RootComponent = Cast<UPrimitiveComponent>(ActorView.GetActor()->GetRootComponent());
    if (RootComponent == nullptr) {
      RespondErrorStr("unable to set actor simulate physics: not supported by actor");
    }
    RootComponent->SetSimulatePhysics(bEnabled);
  });

  Server.BindSync("apply_control_to_vehicle", [this](cr::Actor Actor, cr::VehicleControl Control) {
    RequireEpisode();
    auto ActorView = Episode->GetActorRegistry().Find(Actor.id);
    if (!ActorView.IsValid() || ActorView.GetActor()->IsPendingKill()) {
      RespondErrorStr("unable to apply control: actor not found");
    }
    auto Vehicle = Cast<ACarlaWheeledVehicle>(ActorView.GetActor());
    if (Vehicle == nullptr) {
      RespondErrorStr("unable to apply control: actor is not a vehicle");
    }
    Vehicle->ApplyVehicleControl(Control);
  });

  Server.BindSync("apply_control_to_walker", [this](cr::Actor Actor, cr::WalkerControl Control) {
    RequireEpisode();
    auto ActorView = Episode->GetActorRegistry().Find(Actor.id);
    if (!ActorView.IsValid() || ActorView.GetActor()->IsPendingKill()) {
      RespondErrorStr("unable to apply control: actor not found");
    }
    auto Pawn = Cast<APawn>(ActorView.GetActor());
    if (Pawn == nullptr) {
      RespondErrorStr("unable to apply control: actor is not a walker");
    }
    auto Controller = Cast<AWalkerController>(Pawn->GetController());
    if (Controller == nullptr) {
      RespondErrorStr("unable to apply control: walker has an incompatible controller");
    }
    Controller->ApplyWalkerControl(Control);
  });

  Server.BindSync("set_actor_autopilot", [this](cr::Actor Actor, bool bEnabled) {
    RequireEpisode();
    auto ActorView = Episode->GetActorRegistry().Find(Actor.id);
    if (!ActorView.IsValid() || ActorView.GetActor()->IsPendingKill()) {
      RespondErrorStr("unable to set autopilot: actor not found");
    }
    auto Vehicle = Cast<ACarlaWheeledVehicle>(ActorView.GetActor());
    if (Vehicle == nullptr) {
      RespondErrorStr("unable to set autopilot: actor is not a vehicle");
    }
    auto Controller = Cast<AWheeledVehicleAIController>(Vehicle->GetController());
    if (Controller == nullptr) {
      RespondErrorStr("unable to set autopilot: vehicle has an incompatible controller");
    }
    Controller->SetAutopilot(bEnabled);
  });

  Server.BindSync("draw_debug_shape", [this](const cr::DebugShape &shape) {
    RequireEpisode();
    auto *World = Episode->GetWorld();
    check(World != nullptr);
    FDebugShapeDrawer Drawer(*World);
    Drawer.Draw(shape);
  });
}

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
  UE_LOG(LogCarlaServer, Log, TEXT("New episode '%s' started"), *Episode.GetMapName());
  Pimpl->Episode = &Episode;
}

void FTheNewCarlaServer::NotifyEndEpisode()
{
  Pimpl->Episode = nullptr;
}

void FTheNewCarlaServer::AsyncRun(uint32 NumberOfWorkerThreads)
{
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
  Pimpl->Server.Stop();
}

carla::rpc::Actor FTheNewCarlaServer::SerializeActor(FActorView View) const
{
  return Pimpl->SerializeActor(View);
}
