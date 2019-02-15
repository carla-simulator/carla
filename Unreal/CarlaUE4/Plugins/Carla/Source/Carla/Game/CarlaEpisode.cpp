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

static FString UCarlaEpisode_GetTrafficSignId(ETrafficSignState State)
{
  using TSS = ETrafficSignState;
  switch (State)
  {
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
} ()) {
  ActorDispatcher = CreateDefaultSubobject<UActorDispatcher>(TEXT("ActorDispatcher"));
}

TArray<FTransform> UCarlaEpisode::GetRecommendedSpawnPoints() const
{
  TArray<FTransform> SpawnPoints;
  for (TActorIterator<AVehicleSpawnPoint> It(GetWorld()); It; ++It)
  {
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
  }
  else
  {
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
  if (Recorder->IsEnabled())
  {
    CarlaRecorderEventParent RecEvent
    {
      FindActor(Child).GetActorId(),
      FindActor(Parent).GetActorId()
    };
    Recorder->AddEvent(std::move(RecEvent));
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
  Recorder->GetReplayer()->SetCallbackEventAdd([this](FVector Location, FVector Rotation,
      CarlaRecorderActorDescription Description, uint32_t DesiredId) -> std::pair<int, uint32_t> {

    FActorDescription ActorDesc;
    ActorDesc.UId = Description.UId;
    ActorDesc.Id = Description.Id;
    for (const auto &Item : Description.Attributes)
    {
      FActorAttribute Attr;
      Attr.Type = static_cast<EActorAttributeType>(Item.Type);
      Attr.Id = Item.Id;
      Attr.Value = Item.Value;
      ActorDesc.Variations.Add(Attr.Id, std::move(Attr));
    }

    auto result = TryToCreateReplayerActor(Location, Rotation, ActorDesc, DesiredId);

    if (result.first != 0)
    {
      // disable physics
      // SetActorSimulatePhysics(result.second, false);
      // disable autopilot
      auto Vehicle = Cast<ACarlaWheeledVehicle>(result.second.GetActor());
      if (Vehicle != nullptr)
      {
        auto Controller = Cast<AWheeledVehicleAIController>(Vehicle->GetController());
        if (Controller != nullptr)
        {
          Controller->SetAutopilot(false);
          UE_LOG(LogCarla, Log, TEXT("resetting autopilot to %d"), DesiredId);
        }
      }
    }

    return std::make_pair(result.first, result.second.GetActorId());
  });

  // callback
  Recorder->GetReplayer()->SetCallbackEventDel([this](uint32_t DatabaseId) -> bool {
    auto actor = GetActorRegistry().Find(DatabaseId).GetActor();
    if (actor == nullptr)
    {
      return false;
    }
    DestroyActor(actor);
    return true;
  });

  // callback
  Recorder->GetReplayer()->SetCallbackEventParent([this](uint32_t ChildId, uint32_t ParentId) -> bool {
    AActor *child = GetActorRegistry().Find(ChildId).GetActor();
    AActor *parent = GetActorRegistry().Find(ParentId).GetActor();
    if (child && parent)
    {
      child->AttachToActor(parent, FAttachmentTransformRules::KeepRelativeTransform);
      child->SetOwner(parent);
      UE_LOG(LogCarla, Log, TEXT("Parenting Actor"));
      return true;
    }
    else
    {
      UE_LOG(LogCarla, Log, TEXT("Parenting Actors not found"));
      return false;
    }
  });

  // callback
  Recorder->GetReplayer()->SetCallbackEventPosition([this](CarlaRecorderPosition Pos1,
      CarlaRecorderPosition Pos2, double Per) -> bool {
    AActor *actor = GetActorRegistry().Find(Pos1.DatabaseId).GetActor();
    if (actor && !actor->IsPendingKill())
    {
      // interpolate transform
      FVector Location = FMath::Lerp(FVector(Pos1.Location), FVector(Pos2.Location), Per);
      FRotator Rotation =
      FMath::Lerp(FRotator::MakeFromEuler(Pos1.Rotation), FRotator::MakeFromEuler(Pos2.Rotation), Per);
      FTransform Trans(Rotation, Location, FVector(1, 1, 1));
      actor->SetActorRelativeTransform(Trans, false, nullptr, ETeleportType::TeleportPhysics);
      return true;
    }
    return false;
  });

  // callback
  Recorder->GetReplayer()->SetCallbackStateTrafficLight([this](CarlaRecorderStateTrafficLight State) -> bool {
    AActor *Actor = GetActorRegistry().Find(State.DatabaseId).GetActor();
    if (Actor && !Actor->IsPendingKill())
    {
      auto TrafficLight = Cast<ATrafficLightBase>(Actor);
      if (TrafficLight != nullptr)
      {
        TrafficLight->SetTrafficLightState(static_cast<ETrafficLightState>(State.State));
        TrafficLight->SetTimeIsFrozen(State.IsFrozen);
        TrafficLight->SetElapsedTime(State.ElapsedTime);
      }
      return true;
    }
    return false;
  });

  // callback
  Recorder->GetReplayer()->SetCallbackEventFinish([this](bool applyAutopilot) -> bool {
    if (!applyAutopilot)
    {
      return false;
    }
    // set autopilo to all AI vehicles
    auto registry = GetActorRegistry();
    for (auto &&ActorView : registry)
    {
      if (!ActorView.IsValid() || ActorView.GetActor()->IsPendingKill())
      {
        continue;
      }
      auto Vehicle = Cast<ACarlaWheeledVehicle>(ActorView.GetActor());
      if (Vehicle == nullptr)
      {
        continue;
      }
      // SetActorSimulatePhysics(ActorView, true);
      auto Controller = Cast<AWheeledVehicleAIController>(Vehicle->GetController());
      if (Controller == nullptr)
      {
        continue;
      }
      Controller->SetAutopilot(true);
      UE_LOG(LogCarla, Log, TEXT("setting autopilot to %d"), ActorView.GetActorId());
    }

    return true;
  });
}

void UCarlaEpisode::EndPlay(void)
{
  // stop recorder and replayer
  if (Recorder)
  {
    Recorder->Stop();
    if (Recorder->GetReplayer()->IsEnabled())
    {
      Recorder->GetReplayer()->Stop();
    }
  }
}

bool UCarlaEpisode::SetActorSimulatePhysics(FActorView &ActorView, bool bEnabled)
{
  if (!ActorView.IsValid() || ActorView.GetActor()->IsPendingKill())
  {
    return false;
  }
  auto RootComponent = Cast<UPrimitiveComponent>(ActorView.GetActor()->GetRootComponent());
  if (RootComponent == nullptr)
  {
    return false; // unable to set actor simulate physics: not supported by
                  // actor
  }
  RootComponent->SetSimulatePhysics(bEnabled);

  return true;
}

std::string UCarlaEpisode::StartRecorder(std::string Name)
{
  std::string result;
  FString Name2(Name.c_str());

  if (Recorder)
  {
    result = Recorder->Start(FPaths::ConvertRelativePathToFull(FPaths::ProjectSavedDir()), Name2, MapName);
  }
  else
  {
    result = "Recorder is not ready";
  }

  return result;
}

// create or reuse an actor for replaying
std::pair<int, FActorView &> UCarlaEpisode::TryToCreateReplayerActor(
    FVector &Location,
    FVector &Rotation,
    FActorDescription &ActorDesc,
    uint32_t DesiredId)
{
  FActorView view_empty;

  // check type of actor we need
  if (ActorDesc.Id.StartsWith("traffic."))
  {
    auto World = GetWorld();
    check(World != nullptr);
    // get its position (truncated as int's, and in Cm)
    int x = static_cast<int>(Location.X);
    int y = static_cast<int>(Location.Y);
    int z = static_cast<int>(Location.Z);
    UE_LOG(LogCarla,
        Log,
        TEXT("Trying to find traffic: %s (%d) [%d,%d,%d]"),
        *ActorDesc.Id,
        DesiredId,
        x,
        y,
        z);
    // search an "traffic." actor at that position
    for (TActorIterator<ATrafficSignBase> It(World); It; ++It)
    {
      ATrafficSignBase *Actor = *It;
      check(Actor != nullptr);
      FVector vec = Actor->GetTransform().GetTranslation();
      int x2 = static_cast<int>(vec.X);
      int y2 = static_cast<int>(vec.Y);
      int z2 = static_cast<int>(vec.Z);
      UE_LOG(LogCarla, Log, TEXT(" Checking with [%d,%d,%d]"), x2, y2, z2);
      if ((x2 == x) && (y2 == y) && (z2 == z))
      {
        // actor found
        auto view = ActorDispatcher->GetActorRegistry().Find(static_cast<AActor *>(Actor));
        // reuse that actor
        UE_LOG(LogCarla, Log, TEXT("Traffic found with id: %d"), view.GetActorId());
        return std::pair<int, FActorView &>(2, view);
      }
    }
    // actor not found
    UE_LOG(LogCarla, Log, TEXT("Traffic not found"));
    return std::pair<int, FActorView &>(0, view_empty);
  }
  else if (ActorDesc.Id.StartsWith("vehicle."))
  {
    // check if an actor of that type already exist with same id
    UE_LOG(LogCarla, Log, TEXT("Trying to create actor: %s (%d)"), *ActorDesc.Id, DesiredId);
    if (GetActorRegistry().Contains(DesiredId))
    {
      auto view = GetActorRegistry().Find(DesiredId);
      const FActorDescription *desc = &view.GetActorInfo()->Description;
      UE_LOG(LogCarla, Log, TEXT("actor '%s' already exist with id %d"), *(desc->Id), view.GetActorId());
      if (desc->Id == ActorDesc.Id)
      {
        // we don't need to create, actor of same type already exist
        return std::pair<int, FActorView &>(2, view);
      }
    }
    // create the transform
    FRotator Rot = FRotator::MakeFromEuler(Rotation);
    FTransform Trans(Rot, Location, FVector(1, 1, 1));
    // create as new actor
    TPair<EActorSpawnResultStatus, FActorView> Result = SpawnActorWithInfo(Trans, ActorDesc, DesiredId);
    if (Result.Key == EActorSpawnResultStatus::Success)
    {
      UE_LOG(LogCarla, Log, TEXT("Actor created by replayer with id %d"), Result.Value.GetActorId());
      return std::pair<int, FActorView &>(1, Result.Value);
    }
    else
    {
      UE_LOG(LogCarla, Log, TEXT("Actor could't be created by replayer"));
      return std::pair<int, FActorView &>(0, Result.Value);
    }
  }
  else
  {
    // actor ignored
    return std::pair<int, FActorView &>(0, view_empty);
  }
}
