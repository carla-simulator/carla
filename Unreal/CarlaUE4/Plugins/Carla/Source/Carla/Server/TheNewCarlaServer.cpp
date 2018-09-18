// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "Carla.h"
#include "TheNewCarlaServer.h"

#include "Carla/Sensor/Sensor.h"

#include "GameFramework/SpectatorPawn.h"

#include <compiler/disable-ue4-macros.h>
#include <carla/Version.h>
#include <carla/rpc/Actor.h>
#include <carla/rpc/ActorDefinition.h>
#include <carla/rpc/ActorDescription.h>
#include <carla/rpc/Server.h>
#include <carla/rpc/Transform.h>
#include <carla/rpc/VehicleControl.h>
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
// -- FStreamingSensorDataSink -------------------------------------------------
// =============================================================================

class FStreamingSensorDataSink : public ISensorDataSink {
public:

  FStreamingSensorDataSink(carla::streaming::Stream InStream)
    : TheStream(InStream) {}

  ~FStreamingSensorDataSink()
  {
    UE_LOG(LogCarlaServer, Log, TEXT("Destroying sensor data sink"));
  }

  void Write(const FSensorDataView &SensorData) final {
    auto MakeBuffer = [](FReadOnlyBufferView View) {
      return carla::Buffer(boost::asio::buffer(View.GetData(), View.GetSize()));
    };
    TheStream.Write(
        MakeBuffer(SensorData.GetHeader()),
        MakeBuffer(SensorData.GetData()));
  }

private:

  carla::streaming::Stream TheStream;
};

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

  carla::rpc::Actor SerializeActor(FActorView ActorView)
  {
    if (ActorView.IsValid())
    {
      auto *Sensor = Cast<ASensor>(ActorView.GetActor());
      if (Sensor != nullptr)
      {
        UE_LOG(LogCarlaServer, Log, TEXT("Making a new sensor stream for actor '%s'"), *ActorView.GetActorDescription()->Id);
        auto Stream = StreamingServer.MakeStream();
        Sensor->SetSensorDataSink(MakeShared<FStreamingSensorDataSink>(Stream));
        return {ActorView, Stream.token()};
      }
    }
    return ActorView;
  }
};

// =============================================================================
// -- FTheNewCarlaServer::FPimpl Bind Actions ----------------------------------
// =============================================================================

void FTheNewCarlaServer::FPimpl::BindActions()
{
  namespace cr = carla::rpc;

  Server.BindAsync("ping", []() { return true; });

  Server.BindAsync("version", []() { return std::string(carla::version()); });

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
    return ActorView;
  });

  Server.BindSync("spawn_actor", [this](
      const cr::Transform &Transform,
      cr::ActorDescription Description) -> cr::Actor {
    RequireEpisode();
    return SerializeActor(SpawnActor(Transform, Description));
  });

  Server.BindSync("spawn_actor_with_parent", [this](
      const cr::Transform &Transform,
      cr::ActorDescription Description,
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
    if (!ActorView.IsValid() || ActorView.GetActor()->IsPendingKill()) {
      RespondErrorStr("unable to destroy actor: actor not found");
    }
    Episode->DestroyActor(ActorView.GetActor());
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
      cr::Location Location) -> bool {
    RequireEpisode();
    auto ActorView = Episode->GetActorRegistry().Find(Actor.id);
    if (!ActorView.IsValid() || ActorView.GetActor()->IsPendingKill()) {
      RespondErrorStr("unable to set actor location: actor not found");
    }
    // This function only works with teleport physics, to reset speeds we need
    // another method.
    return ActorView.GetActor()->SetActorLocation(
        Location,
        false,
        nullptr,
        ETeleportType::TeleportPhysics);
  });

  Server.BindSync("set_actor_transform", [this](
      cr::Actor Actor,
      cr::Transform Transform) -> bool {
    RequireEpisode();
    auto ActorView = Episode->GetActorRegistry().Find(Actor.id);
    if (!ActorView.IsValid() || ActorView.GetActor()->IsPendingKill()) {
      RespondErrorStr("unable to set actor transform: actor not found");
    }
    // This function only works with teleport physics, to reset speeds we need
    // another method.
    return ActorView.GetActor()->SetActorTransform(
        Transform,
        false,
        nullptr,
        ETeleportType::TeleportPhysics);
  });

  Server.BindSync("apply_control_to_actor", [this](cr::Actor Actor, cr::VehicleControl Control) {
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
