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
    carla::recorder::RecorderActorDescription description, unsigned int desiredId) -> bool {

    FActorDescription ActorDesc;
    ActorDesc.UId = description.uid;
    ActorDesc.Id = FString(description.id.size(), UTF8_TO_TCHAR(description.id.data()));
    //UE_LOG(LogCarla, Log, TEXT("1: %s"), *ActorDesc.Id);
    for (const auto &item : description.attributes) {
      FActorAttribute attr;
      attr.Type = static_cast<EActorAttributeType>(item.type);
      attr.Id = FString(item.id.size(), UTF8_TO_TCHAR(item.id.data()));
      //UE_LOG(LogCarla, Log, TEXT("2: %s"), *attr.Id);
      attr.Value = FString(item.value.size(), UTF8_TO_TCHAR(item.value.data()));
      //UE_LOG(LogCarla, Log, TEXT("3: %s"), *attr.Value);
      ActorDesc.Variations.Add(attr.Id, std::move(attr));
    }

    TPair<EActorSpawnResultStatus, FActorView> Result = SpawnActorWithInfo(transform, ActorDesc, desiredId);
    if (Result.Key == EActorSpawnResultStatus::Success) {
      //UE_LOG(LogCarla, Log, TEXT("Actor created by replayer"));
      return true;
    }
    else {
      UE_LOG(LogCarla, Log, TEXT("Actor could't be created by replayer"));
      return false;
    }
  });

    // callback
    Recorder.getReplayer().setCallbackEventDel([this](unsigned int databaseId) -> bool {
      DestroyActor(GetActorRegistry().FindActor(databaseId));
      //UE_LOG(LogCarla, Log, TEXT("Destroy Actor (TODO)"));
      return true;
    });

    // callback
    Recorder.getReplayer().setCallbackEventParent([this](unsigned int childId, unsigned int parentId) -> bool {
      UE_LOG(LogCarla, Log, TEXT("Parenting Actor (TODO)"));
      return false;
    });

    // callback
    Recorder.getReplayer().setCallbackEventPosition([this](carla::recorder::RecorderPosition pos1, 
      carla::recorder::RecorderPosition pos2, double per) -> bool {
      AActor *actor = GetActorRegistry().FindActor(pos1.databaseId);
      if (actor)
      {
        // interpolate transform
        FVector location = FMath::Lerp(FVector(pos1.transform.location), FVector(pos2.transform.location), per);
        FRotator rotation = FMath::Lerp(FRotator(pos1.transform.rotation), FRotator(pos2.transform.rotation), per);
        FTransform trans(rotation, location, FVector(1,1,1));
        actor->SetActorRelativeTransform(trans, false, nullptr, ETeleportType::TeleportPhysics);

        // TODO: interpolate velocity

        return true;
      }
      return false;
    });
}
