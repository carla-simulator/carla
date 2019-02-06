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
  crec::RecorderEventParent recEvent { 
    FindActor(Child).GetActorId(),
    FindActor(Parent).GetActorId()
  };
  Recorder.addEvent(recEvent);
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
    carla::recorder::RecorderActorDescription description, unsigned int desiredId) -> std::pair<int, unsigned int> {

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

    // check if an actor of that type already exist with same id
    UE_LOG(LogCarla, Log, TEXT("Trying to create actor: %s (%d)"), *ActorDesc.Id, desiredId);
    if (GetActorRegistry().Contains(desiredId)) {
      FActorView view = GetActorRegistry().Find(desiredId);
      const FActorDescription *desc = view.GetActorDescription();
      UE_LOG(LogCarla, Log, TEXT("actor '%s' already exist with id %d"), *(desc->Id), view.GetActorId());
      if (desc->Id == ActorDesc.Id)
        // disable physics
        // SetActorSimulatePhysics(view, false);
        // we don't need to create, actor of same type already exist
        return std::make_pair(2, desiredId);
    }

    // create actor
    TPair<EActorSpawnResultStatus, FActorView> Result = SpawnActorWithInfo(transform, ActorDesc, desiredId);
    if (Result.Key == EActorSpawnResultStatus::Success) {
      // disable physics
      // SetActorSimulatePhysics(Result.Value, false);
      UE_LOG(LogCarla, Log, TEXT("Actor created by replayer with id %d"), Result.Value.GetActorId());
      // for a sensor we need to create the stream
      if (ActorDesc.Id.StartsWith(TEXT("sensor."))) {
        UE_LOG(LogCarla, Log, TEXT("Checking sensor stream"));
        if (server != nullptr)
          server->CheckSensorStream(Result.Value);
        else
          UE_LOG(LogCarla, Log, TEXT("Server not ready"));
      }
      return std::make_pair(1, Result.Value.GetActorId());
    }
    else {
      UE_LOG(LogCarla, Log, TEXT("Actor could't be created by replayer"));
      return std::make_pair(0, 0);
    }
  });

    // callback
    Recorder.getReplayer().setCallbackEventDel([this](unsigned int databaseId) -> bool {
      DestroyActor(GetActorRegistry().FindActor(databaseId));
      return true;
    });

    // callback
    Recorder.getReplayer().setCallbackEventParent([this](unsigned int childId, unsigned int parentId) -> bool {
      AActor *child = GetActorRegistry().FindActor(childId);
      AActor *parent = GetActorRegistry().FindActor(parentId);
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
      AActor *actor = GetActorRegistry().FindActor(pos1.databaseId);
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
    Recorder.getReplayer().setCallbackEventFinish([this](bool applyAutopilot) -> bool {
      if (!applyAutopilot)
        return false;
      // set autopilo to all AI vehicles
      auto registry = GetActorRegistry();
      for (auto &&pair : registry) {
        FActorView ActorView = pair.second;
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
  for (auto &&pair : Registry) {
    auto &&actor_view = pair.second;
    const AActor *actor = actor_view.GetActor();
    check(actor != nullptr);
    // create event
    CreateRecorderEventAdd(
      actor_view.GetActorId(),
      actor->GetActorTransform(),
      *actor_view.GetActorDescription()
    );
    };

  return result;
}

void UCarlaEpisode::CreateRecorderEventAdd(
    unsigned int databaseId,
    const FTransform &Transform,
    FActorDescription thisActorDescription)
{
    // convert from FActorDescription to crec::RecorderActorDescription
    crec::RecorderActorDescription description;
    description.uid = thisActorDescription.UId;
    description.id.copy_from(reinterpret_cast<const unsigned char *>(carla::rpc::FromFString(thisActorDescription.Id).c_str()), thisActorDescription.Id.Len());
    description.attributes.reserve(thisActorDescription.Variations.Num());
    for (const auto &item : thisActorDescription.Variations) {
      crec::RecorderActorAttribute attr;
      attr.type = static_cast<carla::rpc::ActorAttributeType>(item.Value.Type);
      attr.id.copy_from(reinterpret_cast<const unsigned char *>(carla::rpc::FromFString(item.Value.Id).c_str()), item.Value.Id.Len());
      attr.value.copy_from(reinterpret_cast<const unsigned char *>(carla::rpc::FromFString(item.Value.Value).c_str()), item.Value.Value.Len());
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
