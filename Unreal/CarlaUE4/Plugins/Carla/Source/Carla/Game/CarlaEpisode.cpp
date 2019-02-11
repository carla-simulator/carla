// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "Carla.h"
#include "Carla/Game/CarlaEpisode.h"

#include "Carla/Sensor/Sensor.h"
#include "Carla/Util/BoundingBoxCalculator.h"
#include "Carla/Vehicle/VehicleSpawnPoint.h"

#include "EngineUtils.h"
#include "GameFramework/SpectatorPawn.h"

#include <compiler/disable-ue4-macros.h>
#include <carla/recorder/Recorder.h>
#include <compiler/enable-ue4-macros.h>

static FString UCarlaEpisode_GetTrafficSignId(ETrafficSignState State)
{
  using TSS = ETrafficSignState;
  switch (State) {
    case TSS::TrafficLightRed:
    case TSS::TrafficLightYellow:
    case TSS::TrafficLightGreen:  return TEXT("traffic.traffic_light");
    case TSS::SpeedLimit_30:      return TEXT("traffic.speed_limit.30");
    case TSS::SpeedLimit_40:      return TEXT("traffic.speed_limit.40");
    case TSS::SpeedLimit_50:      return TEXT("traffic.speed_limit.50");
    case TSS::SpeedLimit_60:      return TEXT("traffic.speed_limit.60");
    case TSS::SpeedLimit_90:      return TEXT("traffic.speed_limit.90");
    case TSS::SpeedLimit_100:     return TEXT("traffic.speed_limit.100");
    case TSS::SpeedLimit_120:     return TEXT("traffic.speed_limit.120");
    case TSS::SpeedLimit_130:     return TEXT("traffic.speed_limit.130");
    default:                      return TEXT("traffic.unknown");
  }
}

UCarlaEpisode::UCarlaEpisode(const FObjectInitializer &ObjectInitializer)
  : Super(ObjectInitializer),
    Id([]() {
      static uint32 COUNTER = 0u;
      return ++COUNTER;
    }()) {
  ActorDispatcher = CreateDefaultSubobject<UActorDispatcher>(TEXT("ActorDispatcher"));
}

TArray<FTransform> UCarlaEpisode::GetRecommendedSpawnPoints() const
{
  TArray<FTransform> SpawnPoints;
  for (TActorIterator<AVehicleSpawnPoint> It(GetWorld()); It; ++It) {
    SpawnPoints.Add(It->GetActorTransform());
  }
  return SpawnPoints;
}

carla::rpc::Actor UCarlaEpisode::SerializeActor(FActorView ActorView) const
{
  carla::rpc::Actor Actor;
  if (ActorView.IsValid())
  {
    Actor = ActorView.GetActorInfo()->SerializedData;
    auto Parent = ActorView.GetActor()->GetOwner();
    if (Parent != nullptr)
    {
      Actor.parent_id = FindActor(Parent).GetActorId();
    }
  } else {
    UE_LOG(LogCarla, Warning, TEXT("Trying to serialize invalid actor"));
  }
  return Actor;
}

void UCarlaEpisode::AttachActors(AActor *Child, AActor *Parent)
{
  check(Child != nullptr);
  check(Parent != nullptr);
  Child->AttachToActor(Parent, FAttachmentTransformRules::KeepRelativeTransform);
  Child->SetOwner(Parent);

  // recorder event
  if (Recorder.isEnabled()) {
    crec::RecorderEventParent recEvent {
      FindActor(Child).GetActorId(),
      FindActor(Parent).GetActorId()
    };
    Recorder.addEvent(std::move(recEvent));
  }
}

void UCarlaEpisode::InitializeAtBeginPlay()
{
  auto World = GetWorld();
  check(World != nullptr);
  auto PlayerController = UGameplayStatics::GetPlayerController(GetWorld(), 0);
  if (PlayerController == nullptr)
  {
    UE_LOG(LogCarla, Error, TEXT("Can't find player controller!"));
    return;
  }
  Spectator = PlayerController->GetPawn();
  if (Spectator != nullptr)
  {
    FActorDescription Description;
    Description.Id = TEXT("spectator");
    Description.Class = Spectator->GetClass();
    ActorDispatcher->RegisterActor(*Spectator, Description);
  }
  else
  {
    UE_LOG(LogCarla, Error, TEXT("Can't find spectator!"));
  }

  for (TActorIterator<ATrafficSignBase> It(World); It; ++It)
  {
    ATrafficSignBase *Actor = *It;
    check(Actor != nullptr);
    FActorDescription Description;
    Description.Id = UCarlaEpisode_GetTrafficSignId(Actor->GetTrafficSignState());
    Description.Class = Actor->GetClass();
    ActorDispatcher->RegisterActor(*Actor, Description);
  }

  // replayer callbacks
  Recorder.getReplayer().setCallbackEventAdd([this](carla::geom::Transform transform,
    carla::recorder::RecorderActorDescription description, uint32_t desiredId) -> std::pair<int, uint32_t> {

    FActorDescription ActorDesc;
    ActorDesc.UId = description.uid;
    ActorDesc.Id = FString(description.id.size(), UTF8_TO_TCHAR(description.id.data()));
    for (const auto &item : description.attributes) {
      FActorAttribute attr;
      attr.Type = static_cast<EActorAttributeType>(item.type);
      attr.Id = FString(item.id.size(), UTF8_TO_TCHAR(item.id.data()));
      attr.Value = FString(item.value.size(), UTF8_TO_TCHAR(item.value.data()));
      ActorDesc.Variations.Add(attr.Id, std::move(attr));
    }

    auto result = TryToCreateReplayerActor(transform, ActorDesc, desiredId);

    if (result.first != 0) {
      // disable physics
      // SetActorSimulatePhysics(result.second, false);
      // disable autopilot
      auto Vehicle = Cast<ACarlaWheeledVehicle>(result.second.GetActor());
      if (Vehicle != nullptr) {
        auto Controller = Cast<AWheeledVehicleAIController>(Vehicle->GetController());
        if (Controller != nullptr) {
          Controller->SetAutopilot(false);
          UE_LOG(LogCarla, Log, TEXT("resetting autopilot to %d"), desiredId);
        }
      }
    }

    return std::make_pair(result.first, result.second.GetActorId());
  });

    // callback
    Recorder.getReplayer().setCallbackEventDel([this](uint32_t databaseId) -> bool {
      auto actor = GetActorRegistry().Find(databaseId).GetActor();
      if (actor == nullptr) return false;
      DestroyActor(actor);
      return true;
    });

    // callback
    Recorder.getReplayer().setCallbackEventParent([this](uint32_t childId, uint32_t parentId) -> bool {
      AActor *child = GetActorRegistry().Find(childId).GetActor();
      AActor *parent = GetActorRegistry().Find(parentId).GetActor();
      if (child && parent) {
        child->AttachToActor(parent, FAttachmentTransformRules::KeepRelativeTransform);
        child->SetOwner(parent);
        UE_LOG(LogCarla, Log, TEXT("Parenting Actor"));
        return true;
      }
      else {
        UE_LOG(LogCarla, Log, TEXT("Parenting Actors not found"));
        return false;
      }
    });

    // callback
    Recorder.getReplayer().setCallbackEventPosition([this](carla::recorder::RecorderPosition pos1,
      carla::recorder::RecorderPosition pos2, double per) -> bool {
      AActor *actor = GetActorRegistry().Find(pos1.databaseId).GetActor();
      if (actor && !actor->IsPendingKill())
      {
        // interpolate transform
        FVector location = FMath::Lerp(FVector(pos1.transform.location), FVector(pos2.transform.location), per);
        FRotator rotation = FMath::Lerp(FRotator(pos1.transform.rotation), FRotator(pos2.transform.rotation), per);
        FTransform trans(rotation, location, FVector(1,1,1));
        actor->SetActorRelativeTransform(trans, false, nullptr, ETeleportType::TeleportPhysics);
        return true;
      }
      return false;
    });

    // callback
    Recorder.getReplayer().setCallbackStateTrafficLight([this](carla::recorder::RecorderStateTrafficLight state) -> bool {
      AActor *Actor = GetActorRegistry().Find(state.databaseId).GetActor();
      if (Actor && !Actor->IsPendingKill()) {
        auto TrafficLight = Cast<ATrafficLightBase>(Actor);
        if (TrafficLight != nullptr)
        {
          TrafficLight->SetTrafficLightState(static_cast<ETrafficLightState>(state.state));
          TrafficLight->SetTimeIsFrozen(state.isFrozen);
          TrafficLight->SetElapsedTime(state.elapsedTime);
        }
        return true;
      }
      return false;
    });

    // callback
    Recorder.getReplayer().setCallbackEventFinish([this](bool applyAutopilot) -> bool {
      if (!applyAutopilot)
        return false;
      // set autopilo to all AI vehicles
      auto registry = GetActorRegistry();
      for (auto &&ActorView : registry) {
        if (!ActorView.IsValid() || ActorView.GetActor()->IsPendingKill()) {
          continue;
        }
        auto Vehicle = Cast<ACarlaWheeledVehicle>(ActorView.GetActor());
        if (Vehicle == nullptr) {
          continue;
        }
        // SetActorSimulatePhysics(ActorView, true);
        auto Controller = Cast<AWheeledVehicleAIController>(Vehicle->GetController());
        if (Controller == nullptr) {
          continue;
        }
        Controller->SetAutopilot(true);
        UE_LOG(LogCarla, Log, TEXT("setting autopilot to %d"), ActorView.GetActorId());
      }

      return true;
    });

}

void UCarlaEpisode::EndPlay(void) {
  // stop recorder and replayer
  if (Recorder.isEnabled())
    Recorder.stop();
  if (Recorder.getReplayer().isEnabled())
    Recorder.getReplayer().stop();
}

bool UCarlaEpisode::SetActorSimulatePhysics(FActorView &ActorView, bool bEnabled) {
  if (!ActorView.IsValid() || ActorView.GetActor()->IsPendingKill()) {
    return false;
  }
  auto RootComponent = Cast<UPrimitiveComponent>(ActorView.GetActor()->GetRootComponent());
  if (RootComponent == nullptr) {
    return false; // unable to set actor simulate physics: not supported by actor
  }
  RootComponent->SetSimulatePhysics(bEnabled);

  return true;
}

std::string UCarlaEpisode::StartRecorder(std::string name) {
  std::string result;

  // start
  result = Recorder.start(carla::rpc::FromFString(FPaths::ConvertRelativePathToFull(FPaths::ProjectSavedDir())), name, carla::rpc::FromFString(MapName));

  // registring all existing actors in first frame
  FActorRegistry Registry = GetActorRegistry();
  for (auto &&View : Registry) {
    const AActor *actor = View.GetActor();
    check(actor != nullptr);
    // create event
    CreateRecorderEventAdd(
      View.GetActorId(),
      actor->GetActorTransform(),
      View.GetActorInfo()->Description
    );
    };

  return result;
}

void UCarlaEpisode::CreateRecorderEventAdd(
    uint32_t databaseId,
    const FTransform &Transform,
    FActorDescription thisActorDescription)
{
    // convert from FActorDescription to crec::RecorderActorDescription
    crec::RecorderActorDescription description;
    description.uid = thisActorDescription.UId;
    description.id.copy_from(carla::rpc::FromFString(thisActorDescription.Id));

    description.attributes.reserve(thisActorDescription.Variations.Num());
    for (const auto &item : thisActorDescription.Variations) {
      crec::RecorderActorAttribute attr;
      attr.type = static_cast<carla::rpc::ActorAttributeType>(item.Value.Type);
      attr.id.copy_from(carla::rpc::FromFString(item.Value.Id));
      attr.value.copy_from(carla::rpc::FromFString(item.Value.Value));
      // check for empty attributes
      if (attr.id.size() > 0)
        description.attributes.emplace_back(std::move(attr));
    }

  // recorder event
  crec::RecorderEventAdd recEvent {
    databaseId,
    Transform,
    std::move(description)
  };
  Recorder.addEvent(std::move(recEvent));
}

// create or reuse an actor for replaying
std::pair<int, FActorView&> UCarlaEpisode::TryToCreateReplayerActor(carla::geom::Transform &transform, FActorDescription &ActorDesc, uint32_t desiredId) {
  FActorView view_empty;

  // check type of actor we need
  if (ActorDesc.Id.StartsWith("traffic.")) {
    auto World = GetWorld();
    check(World != nullptr);
    // get its position (truncated as int's, and in Cm)
    int x = static_cast<int>(transform.location.x);
    int y = static_cast<int>(transform.location.y);
    int z = static_cast<int>(transform.location.z);
    UE_LOG(LogCarla, Log, TEXT("Trying to find traffic: %s (%d) [%d,%d,%d]"), *ActorDesc.Id, desiredId, x, y, z);
    // search an "traffic." actor at that position
    for (TActorIterator<ATrafficSignBase> It(World); It; ++It)
    {
      ATrafficSignBase *Actor = *It;
      check(Actor != nullptr);
      FVector vec = Actor->GetTransform().GetTranslation();
      int x2 = static_cast<int>(vec.X / 100.0f);
      int y2 = static_cast<int>(vec.Y / 100.0f);
      int z2 = static_cast<int>(vec.Z / 100.0f);
      // UE_LOG(LogCarla, Log, TEXT(" Checking with [%d,%d,%d]"), x2, y2, z2);
      if ((x2 == x) && (y2 == y) && (z2 == z)) {
        // actor found
        auto view = ActorDispatcher->GetActorRegistry().Find(static_cast<AActor *>(Actor));
        // reuse that actor
        UE_LOG(LogCarla, Log, TEXT("Traffic found with id: %d"), view.GetActorId());
        return std::pair<int, FActorView&>(2, view);
      }
    }
    // actor not found
    UE_LOG(LogCarla, Log, TEXT("Traffic not found"));
    return std::pair<int, FActorView&>(0, view_empty);
  } else if (ActorDesc.Id.StartsWith("vehicle.")) {
    // check if an actor of that type already exist with same id
    UE_LOG(LogCarla, Log, TEXT("Trying to create actor: %s (%d)"), *ActorDesc.Id, desiredId);
    if (GetActorRegistry().Contains(desiredId)) {
      auto view = GetActorRegistry().Find(desiredId);
      const FActorDescription *desc = &view.GetActorInfo()->Description;
      UE_LOG(LogCarla, Log, TEXT("actor '%s' already exist with id %d"), *(desc->Id), view.GetActorId());
      if (desc->Id == ActorDesc.Id) {
        // we don't need to create, actor of same type already exist
        return std::pair<int, FActorView&>(2, view);
      }
    }
    // create as new actor
    TPair<EActorSpawnResultStatus, FActorView> Result = SpawnActorWithInfo(transform, ActorDesc, desiredId);
    if (Result.Key == EActorSpawnResultStatus::Success) {
      UE_LOG(LogCarla, Log, TEXT("Actor created by replayer with id %d"), Result.Value.GetActorId());
      return std::pair<int, FActorView&>(1, Result.Value);
    }
    else {
      UE_LOG(LogCarla, Log, TEXT("Actor could't be created by replayer"));
      return std::pair<int, FActorView&>(0, Result.Value);
    }
  }
  else {
    // actor ignored
    return std::pair<int, FActorView&>(0, view_empty);
  }
}
