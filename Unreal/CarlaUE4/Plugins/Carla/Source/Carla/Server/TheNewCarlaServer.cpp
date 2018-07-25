// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "Carla.h"
#include "TheNewCarlaServer.h"

#include <compiler/disable-ue4-macros.h>
#include <carla/Version.h>
#include <carla/rpc/Actor.h>
#include <carla/rpc/ActorDefinition.h>
#include <carla/rpc/ActorDescription.h>
#include <carla/rpc/Server.h>
#include <carla/rpc/Transform.h>
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

  FPimpl(uint16_t port) : Server(port) {
    BindActions();
  }

  carla::rpc::Server Server;

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
      RespondError("unable to attach actor: child actor not found");
    }
    if (!Parent.IsValid())
    {
      RespondError("unable to attach actor: parent actor not found");
    }
    ::AttachActors(Child.GetActor(), Parent.GetActor());
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

  Server.BindSync("spawn_actor", [this](
      const cr::Transform &Transform,
      cr::ActorDescription Description) -> cr::Actor {
    RequireEpisode();
    return SpawnActor(Transform, Description);
  });

  Server.BindSync("spawn_actor_with_parent", [this](
      const cr::Transform &Transform,
      cr::ActorDescription Description,
      cr::Actor Parent) -> cr::Actor {
    RequireEpisode();
    auto ActorView = SpawnActor(Transform, Description);
    auto ParentActorView = Episode->GetActorRegistry().Find(Parent.id);
    AttachActors(ActorView, ParentActorView);
    return ActorView;
  });

  Server.BindSync("attach_actors", [this](cr::Actor Child, cr::Actor Parent) {
    RequireEpisode();
    auto &Registry = Episode->GetActorRegistry();
    AttachActors(Registry.Find(Child.id), Registry.Find(Parent.id));
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
  Pimpl->Server.AsyncRun(NumberOfWorkerThreads);
}

void FTheNewCarlaServer::RunSome(uint32 Milliseconds)
{
  Pimpl->Server.SyncRunFor(carla::time_duration::milliseconds(Milliseconds));
}

void FTheNewCarlaServer::Stop()
{
  Pimpl->Server.Stop();
}
