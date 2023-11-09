// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "Carla.h"
#include "Carla/Recorder/CarlaReplayerHelper.h"

#include "Carla/Actor/ActorDescription.h"
#include "Carla/Actor/ActorRegistry.h"
#include "Carla/Actor/ActorSpawnResult.h"
#include "Carla/Actor/CarlaActor.h"
#include "Carla/Game/CarlaEpisode.h"
#include "Carla/Game/CarlaStatics.h"
#include "Carla/Lights/CarlaLight.h"
#include "Carla/Lights/CarlaLightSubsystem.h"
#include "Carla/MapGen/LargeMapManager.h"
#include "Carla/Traffic/TrafficLightBase.h"
#include "Carla/Traffic/TrafficLightController.h"
#include "Carla/Traffic/TrafficLightGroup.h"
#include "Carla/Traffic/TrafficSignBase.h"
#include "Carla/Vehicle/CarlaWheeledVehicle.h"
#include "Carla/Vehicle/WheeledVehicleAIController.h"
#include "Carla/Walker/WalkerControl.h"
#include "Carla/Walker/WalkerController.h"
#include "Components/BoxComponent.h"
#include "Engine/StaticMeshActor.h"

#include <compiler/disable-ue4-macros.h>
#include <carla/rpc/VehicleLightState.h>
#include <compiler/enable-ue4-macros.h>


#include "EngineUtils.h"

// create or reuse an actor for replaying
std::pair<int, FCarlaActor*>CarlaReplayerHelper::TryToCreateReplayerActor(
    FVector &Location,
    FVector &Rotation,
    FActorDescription &ActorDesc,
    uint32_t DesiredId,
    bool SpawnSensors)
{
  check(Episode != nullptr);

  // check type of actor we need
  if (ActorDesc.Id.StartsWith("traffic."))
  {
    FCarlaActor* CarlaActor = FindTrafficLightAt(Location);
    if (CarlaActor != nullptr)
    {
      // reuse that actor
      return std::pair<int, FCarlaActor*>(2, CarlaActor);
    }
    else
    {
      // actor not found
      UE_LOG(LogCarla, Log, TEXT("TrafficLight not found"));
      return std::pair<int, FCarlaActor*>(0, nullptr);
    }
  }
  else if (SpawnSensors || !ActorDesc.Id.StartsWith("sensor."))
  {
    // check if an actor of that type already exist with same id
    if (Episode->GetActorRegistry().Contains(DesiredId))
    {
      auto* CarlaActor = Episode->FindCarlaActor(DesiredId);
      const FActorDescription *desc = &CarlaActor->GetActorInfo()->Description;
      if (desc->Id == ActorDesc.Id)
      {
        // we don't need to create, actor of same type already exist
        // relocate
        FRotator Rot = FRotator::MakeFromEuler(Rotation);
        FTransform Trans2(Rot, Location, FVector(1, 1, 1));
        CarlaActor->SetActorGlobalTransform(Trans2);
        return std::pair<int, FCarlaActor*>(2, CarlaActor);
      }
    }
    // create the transform
    FRotator Rot = FRotator::MakeFromEuler(Rotation);
    FTransform Trans(Rot, FVector(0, 0, 100000), FVector(1, 1, 1));
    // create as new actor
    TPair<EActorSpawnResultStatus, FCarlaActor*> Result = Episode->SpawnActorWithInfo(Trans, ActorDesc, DesiredId);
    if (Result.Key == EActorSpawnResultStatus::Success)
    {
      // relocate
      FTransform Trans2(Rot, Location, FVector(1, 1, 1));
      Result.Value->SetActorGlobalTransform(Trans2);
      ALargeMapManager * LargeMapManager = UCarlaStatics::GetLargeMapManager(Episode->GetWorld());
      if (LargeMapManager)
      {
        LargeMapManager->OnActorSpawned(*Result.Value);
      }
      return std::pair<int, FCarlaActor*>(1, Result.Value);
    }
    else
    {
      UE_LOG(LogCarla, Log, TEXT("Actor could't be created by replayer"));
      return std::pair<int, FCarlaActor*>(0, Result.Value);
    }
  }
  else
  {
    // actor ignored
    return std::pair<int, FCarlaActor*>(0, nullptr);
  }
}

FCarlaActor *CarlaReplayerHelper::FindTrafficLightAt(FVector Location)
{
  check(Episode != nullptr);
  auto World = Episode->GetWorld();
  check(World != nullptr);

  // get its position (truncated as int's)
  int x = static_cast<int>(Location.X);
  int y = static_cast<int>(Location.Y);
  int z = static_cast<int>(Location.Z);

  const FActorRegistry &Registry = Episode->GetActorRegistry();
  // through all actors in registry
  for (auto It = Registry.begin(); It != Registry.end(); ++It)
  {
    FCarlaActor* CarlaActor = It.Value().Get();
    if(CarlaActor->GetActorType() == FCarlaActor::ActorType::TrafficLight)
    {
      FVector vec = CarlaActor->GetActorGlobalLocation();
      int x2 = static_cast<int>(vec.X);
      int y2 = static_cast<int>(vec.Y);
      int z2 = static_cast<int>(vec.Z);
      if ((x2 == x) && (y2 == y) && (z2 == z))
      {
        // actor found
        return CarlaActor;
      }
    }
  }
  // actor not found
  return nullptr;
}

// enable / disable physics for an actor
bool CarlaReplayerHelper::SetActorSimulatePhysics(FCarlaActor* CarlaActor, bool bEnabled)
{
  if (!CarlaActor)
  {
    return false;
  }
  ECarlaServerResponse Response =
      CarlaActor->SetActorSimulatePhysics(bEnabled);
  if (Response != ECarlaServerResponse::Success)
  {
    return false;
  }
  return true;
}

// enable / disable autopilot for an actor
bool CarlaReplayerHelper::SetActorAutopilot(FCarlaActor* CarlaActor, bool bEnabled, bool bKeepState)
{
  if (!CarlaActor)
  {
    return false;
  }
  ECarlaServerResponse Response =
      CarlaActor->SetActorAutopilot(bEnabled, bKeepState);
  if (Response != ECarlaServerResponse::Success)
  {
    return false;
  }
  return true;
}

// replay event for creating actor
std::pair<int, uint32_t> CarlaReplayerHelper::ProcessReplayerEventAdd(
    FVector Location,
    FVector Rotation,
    CarlaRecorderActorDescription Description,
    uint32_t DesiredId,
    bool bIgnoreHero,
    bool bIgnoreSpectator,
    bool ReplaySensors)
{
  check(Episode != nullptr);
  FActorDescription ActorDesc;
  bool IsHero = false;

  // prepare actor description
  ActorDesc.UId = Description.UId;
  ActorDesc.Id = Description.Id;
  for (const auto &Item : Description.Attributes)
  {
    FActorAttribute Attr;
    Attr.Type = static_cast<EActorAttributeType>(Item.Type);
    Attr.Id = Item.Id;
    Attr.Value = Item.Value;
    ActorDesc.Variations.Add(Attr.Id, std::move(Attr));
    // check for hero
    if (Item.Id == "role_name" && Item.Value == "hero")
      IsHero = true;
  }

  // check to ignore Hero or Spectator
  if ((bIgnoreHero && IsHero) || 
      (bIgnoreSpectator && ActorDesc.Id.StartsWith("spectator")))
  {
    return std::make_pair(3, 0);
  }

  auto result = TryToCreateReplayerActor(
      Location,
      Rotation,
      ActorDesc,
      DesiredId,
      ReplaySensors);

  if (result.first != 0)
  {
    // disable physics and autopilot on vehicles
    if (result.second->GetActorType() == FCarlaActor::ActorType::Vehicle ||
        result.second->GetActorType() == FCarlaActor::ActorType::Walker)
    {
      // ignore hero ?
      if (!(bIgnoreHero && IsHero))
      {
        // disable physics
        SetActorSimulatePhysics(result.second, false);
        // disable collisions
        result.second->GetActor()->SetActorEnableCollision(false);
        // disable autopilot for vehicles
        if (result.second->GetActorType() == FCarlaActor::ActorType::Vehicle)
          SetActorAutopilot(result.second, false, false);
      }
      else
      {
        // enable physics just in case
        SetActorSimulatePhysics(result.second, true);
        // enable collisions
        result.second->GetActor()->SetActorEnableCollision(true);
      }
    }
    return std::make_pair(result.first, result.second->GetActorId());
  }
  return std::make_pair(result.first, 0);
}

// replay event for removing actor
bool CarlaReplayerHelper::ProcessReplayerEventDel(uint32_t DatabaseId)
{
  check(Episode != nullptr);
  FCarlaActor* CarlaActor = Episode->FindCarlaActor(DatabaseId);
  if (CarlaActor == nullptr)
  {
    UE_LOG(LogCarla, Log, TEXT("Actor %d not found to destroy"), DatabaseId);
    return false;
  }
  Episode->DestroyActor(CarlaActor->GetActorId());
  return true;
}

// replay event for parenting actors
bool CarlaReplayerHelper::ProcessReplayerEventParent(uint32_t ChildId, uint32_t ParentId)
{
  check(Episode != nullptr);
  FCarlaActor * Child = Episode->FindCarlaActor(ChildId);
  FCarlaActor * Parent = Episode->FindCarlaActor(ParentId);
  if(!Child)
  {
    UE_LOG(LogCarla, Log, TEXT("Parenting Child actors not found"));
    return false;
  }
  if(!Parent)
  {
    UE_LOG(LogCarla, Log, TEXT("Parenting Parent actors not found"));
    return false;
  }
  Child->SetParent(ParentId);
  Child->SetAttachmentType(carla::rpc::AttachmentType::Rigid);
  Parent->AddChildren(Child->GetActorId());
  if(!Parent->IsDormant())
  {
    if(!Child->IsDormant())
    {
      Episode->AttachActors(
          Child->GetActor(),
          Parent->GetActor(),
          static_cast<EAttachmentType>(carla::rpc::AttachmentType::Rigid));
    }
  }
  else
  {
    Episode->PutActorToSleep(Child->GetActorId());
  }
  return true;
}

// reposition actors
bool CarlaReplayerHelper::ProcessReplayerPosition(CarlaRecorderPosition Pos1, CarlaRecorderPosition Pos2, double Per, double DeltaTime)
{
  check(Episode != nullptr);
  FCarlaActor* CarlaActor = Episode->FindCarlaActor(Pos1.DatabaseId);
  FVector Location;
  FRotator Rotation;
  if(CarlaActor)
  {
    // check to assign first position or interpolate between both
    if (Per == 0.0)
    {
      // assign position 1
      Location = FVector(Pos1.Location);
      Rotation = FRotator::MakeFromEuler(Pos1.Rotation);
    }
    else
    {
      // interpolate positions
      Location = FMath::Lerp(FVector(Pos1.Location), FVector(Pos2.Location), Per);
      Rotation = FMath::Lerp(FRotator::MakeFromEuler(Pos1.Rotation), FRotator::MakeFromEuler(Pos2.Rotation), Per);
    }
    // set new transform
    FTransform Trans(Rotation, Location, FVector(1, 1, 1));
    CarlaActor->SetActorGlobalTransform(Trans, ETeleportType::None);
    return true;
  }
  return false;
}

void CarlaReplayerHelper::ProcessReplayerAnimVehicleWheels(CarlaRecorderAnimWheels VehicleAnimWheels)
{
  check(Episode != nullptr)
  FCarlaActor *CarlaActor = Episode->FindCarlaActor(VehicleAnimWheels.DatabaseId);
  if (CarlaActor == nullptr)
    return;
  if (CarlaActor->GetActorType() != FCarlaActor::ActorType::Vehicle)
    return;
  ACarlaWheeledVehicle* CarlaVehicle = Cast<ACarlaWheeledVehicle>(CarlaActor->GetActor());
  check(CarlaVehicle != nullptr)
  USkeletalMeshComponent* SkeletalMesh = CarlaVehicle->GetMesh();
  check(SkeletalMesh != nullptr)
  UVehicleAnimInstance* VehicleAnim = Cast<UVehicleAnimInstance>(SkeletalMesh->GetAnimInstance());
  check(VehicleAnim != nullptr)

  for (uint32_t i = 0; i < VehicleAnimWheels.WheelValues.size(); ++i)
  {
    const WheelInfo& Element = VehicleAnimWheels.WheelValues[i];
    VehicleAnim->SetWheelRotYaw(static_cast<uint8>(Element.Location), Element.SteeringAngle);
    VehicleAnim->SetWheelPitchAngle(static_cast<uint8>(Element.Location), Element.TireRotation);
  }
}

// reposition the camera
bool CarlaReplayerHelper::SetCameraPosition(uint32_t Id, FVector Offset, FQuat Rotation)
{
  check(Episode != nullptr);

  // get the actor to follow
  FCarlaActor* CarlaActor = Episode->FindCarlaActor(Id);
  if (!CarlaActor)
    return false;
  // get specator pawn
  APawn *Spectator = Episode->GetSpectatorPawn();
  if (!Spectator)
   return false;

  FCarlaActor* CarlaSpectator = Episode->FindCarlaActor(Spectator);
  if (!CarlaSpectator)
    return false;

  FTransform ActorTransform = CarlaActor->GetActorGlobalTransform();
  // set the new position
  FQuat ActorRot = ActorTransform.GetRotation();
  FVector Pos = ActorTransform.GetTranslation() + (ActorRot.RotateVector(Offset));
  CarlaSpectator->SetActorGlobalTransform(FTransform(ActorRot * Rotation, Pos, FVector(1,1,1)));

  return true;
}

bool CarlaReplayerHelper::ProcessReplayerStateTrafficLight(CarlaRecorderStateTrafficLight State)
{
  check(Episode != nullptr);
  FCarlaActor* CarlaActor = Episode->FindCarlaActor(State.DatabaseId);
  if(CarlaActor)
  {
    CarlaActor->SetTrafficLightState(static_cast<ETrafficLightState>(State.State));
    UTrafficLightController* Controller = CarlaActor->GetTrafficLightController();
    if(Controller)
    {
      Controller->SetElapsedTime(State.ElapsedTime);
      ATrafficLightGroup* Group = Controller->GetGroup();
      if (Group)
      {
        Group->SetFrozenGroup(State.IsFrozen);
      }
    }
     return true;
  }
  return false;
}

// set the animation for Vehicles
void CarlaReplayerHelper::ProcessReplayerAnimVehicle(CarlaRecorderAnimVehicle Vehicle)
{
  check(Episode != nullptr);
  FCarlaActor *CarlaActor = Episode->FindCarlaActor(Vehicle.DatabaseId);
  if (CarlaActor)
  {
    FVehicleControl Control;
    Control.Throttle = Vehicle.Throttle;
    Control.Steer = Vehicle.Steering;
    Control.Brake = Vehicle.Brake;
    Control.bHandBrake = Vehicle.bHandbrake;
    Control.bReverse = (Vehicle.Gear < 0);
    Control.Gear = Vehicle.Gear;
    Control.bManualGearShift = false;
    CarlaActor->ApplyControlToVehicle(Control, EVehicleInputPriority::User);
  }
}

// set the lights for vehicles
void CarlaReplayerHelper::ProcessReplayerLightVehicle(CarlaRecorderLightVehicle LightVehicle)
{
  check(Episode != nullptr);
  FCarlaActor * CarlaActor = Episode->FindCarlaActor(LightVehicle.DatabaseId);
  if (CarlaActor)
  {
    carla::rpc::VehicleLightState LightState(LightVehicle.State);
    CarlaActor->SetVehicleLightState(FVehicleLightState(LightState));
  }
}

void CarlaReplayerHelper::ProcessReplayerLightScene(CarlaRecorderLightScene LightScene)
{
  check(Episode != nullptr);
  UWorld* World = Episode->GetWorld();
  if(World)
  {
    UCarlaLightSubsystem* CarlaLightSubsystem = World->GetSubsystem<UCarlaLightSubsystem>();
    if (!CarlaLightSubsystem)
    {
      return;
    }
    auto* CarlaLight = CarlaLightSubsystem->GetLight(LightScene.LightId);
    if (CarlaLight)
    {
      CarlaLight->SetLightIntensity(LightScene.Intensity);
      CarlaLight->SetLightColor(LightScene.Color);
      CarlaLight->SetLightOn(LightScene.bOn);
      CarlaLight->SetLightType(static_cast<ELightType>(LightScene.Type));
    }
  }
}

// set the animation for walkers
void CarlaReplayerHelper::ProcessReplayerAnimWalker(CarlaRecorderAnimWalker Walker)
{
  SetWalkerSpeed(Walker.DatabaseId, Walker.Speed);
}

void CarlaReplayerHelper::ProcessReplayerAnimBiker(CarlaRecorderAnimBiker Biker)
{
  check(Episode != nullptr);
  FCarlaActor * CarlaActor = Episode->FindCarlaActor(Biker.DatabaseId);
  if (CarlaActor == nullptr)
    return;
  ACarlaWheeledVehicle* CarlaVehicle = Cast<ACarlaWheeledVehicle>(CarlaActor->GetActor());
  check(CarlaVehicle != nullptr)
  CarlaVehicle->SetSpeedAnim(Biker.ForwardSpeed);
  CarlaVehicle->SetRotationAnim(Biker.EngineRotation);
}

// set walker bones
void CarlaReplayerHelper::ProcessReplayerWalkerBones(const CarlaRecorderWalkerBones &WalkerBones)
{
  check(Episode != nullptr);

  FCarlaActor* CarlaActor = Episode->FindCarlaActor(WalkerBones.DatabaseId);
  if (!CarlaActor) return;

  AActor* Actor = CarlaActor->GetActor();
  auto Walker = Cast<APawn>(Actor);
  if (!Walker) return;

  AWalkerController *Controller = Cast<AWalkerController>(Walker->GetController());
  if (!Controller) return;

  // build bones structure
  FWalkerBoneControlIn BonesIn;
  for (const auto &Bone : WalkerBones.Bones)
  {
    FTransform Trans(FRotator::MakeFromEuler(Bone.Rotation), Bone.Location, FVector(1, 1, 1));
    BonesIn.BoneTransforms.Add(Bone.Name, Trans);
  }

  // set the pose and blend
  Controller->SetBonesTransform(BonesIn);
  Controller->BlendPose(1.0f);
}

// replay finish
bool CarlaReplayerHelper::ProcessReplayerFinish(bool bApplyAutopilot, bool bIgnoreHero, std::unordered_map<uint32_t, bool> &IsHero)
{
  // set autopilot and physics to all AI vehicles
  const FActorRegistry& Registry = Episode->GetActorRegistry();
  for (auto& It : Registry)
  {
    FCarlaActor* CarlaActor = It.Value.Get();

    // enable physics only on vehicles
    switch (CarlaActor->GetActorType())
    {

      // vehicles
      case FCarlaActor::ActorType::Vehicle:
        // check for hero
        if (!(bIgnoreHero && IsHero[CarlaActor->GetActorId()]))
        {
            // stop all vehicles
            SetActorSimulatePhysics(CarlaActor, true);
            SetActorVelocity(CarlaActor, FVector(0, 0, 0));
            FVehicleControl Control;
            Control.Throttle = 0.0f;
            Control.Steer = 0.0f;
            Control.Brake = 0.0f;
            Control.bHandBrake = false;
            Control.bReverse = false;
            Control.Gear = 1;
            Control.bManualGearShift = false;
            CarlaActor->ApplyControlToVehicle(Control, EVehicleInputPriority::User);
        }
        break;

      // walkers
      case FCarlaActor::ActorType::Walker:
        // stop walker
        SetWalkerSpeed(CarlaActor->GetActorId(), 0.0f);
        break;
    }
  }
  return true;
}

void CarlaReplayerHelper::SetActorVelocity(FCarlaActor *CarlaActor, FVector Velocity)
{
  if (!CarlaActor)
  {
    return;
  }
  CarlaActor->SetActorTargetVelocity(Velocity);
}

// set the animation speed for walkers
void CarlaReplayerHelper::SetWalkerSpeed(uint32_t ActorId, float Speed)
{
  check(Episode != nullptr);
  FCarlaActor * CarlaActor = Episode->FindCarlaActor(ActorId);
  if (!CarlaActor)
  {
    return;
  }
  FWalkerControl Control;
  Control.Speed = Speed;
  CarlaActor->ApplyControlToWalker(Control);
}

void CarlaReplayerHelper::RemoveStaticProps()
{
  check(Episode != nullptr);
  auto World = Episode->GetWorld();
  for (TActorIterator<AStaticMeshActor> It(World); It; ++It)
  {
    auto Actor = *It;
    check(Actor != nullptr);
    auto MeshComponent = Actor->GetStaticMeshComponent();
    check(MeshComponent != nullptr);
    if (MeshComponent->Mobility == EComponentMobility::Movable)
    {
      Actor->Destroy();
    }
  }
}
