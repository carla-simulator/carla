// Copyright (c) 2020 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "CarlaActor.h"

#include "Carla/OpenDrive/OpenDrive.h"
#include "Carla/Util/NavigationMesh.h"
#include "Carla/Vehicle/CarlaWheeledVehicle.h"
#include "Carla/Walker/WalkerController.h"
#include "Carla/Walker/WalkerBase.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Carla/Game/Tagger.h"
#include "Carla/Vehicle/MovementComponents/CarSimManagerComponent.h"
#include "Carla/Vehicle/MovementComponents/ChronoMovementComponent.h"

#include <compiler/disable-ue4-macros.h>
#include "carla/rpc/LabelledPoint.h"
#include <carla/rpc/LightState.h>
#include <carla/rpc/MapInfo.h>
#include <carla/rpc/MapLayer.h>
#include <carla/rpc/VehicleControl.h>
#include <carla/rpc/VehiclePhysicsControl.h>
#include <carla/rpc/VehicleLightState.h>
#include <carla/rpc/VehicleLightStateList.h>
#include <carla/rpc/WalkerBoneControl.h>
#include <carla/rpc/WalkerControl.h>
#include <carla/rpc/VehicleWheels.h>
#include <carla/rpc/WeatherParameters.h>
#include <compiler/enable-ue4-macros.h>


FCarlaActor::FCarlaActor(
    IdType ActorId,
    AActor* Actor,
    TSharedPtr<const FActorInfo> Info,
    carla::rpc::ActorState InState)
    : TheActor(Actor),
      Info(std::move(Info)),
      Id(ActorId),
      State(InState)
{
}
FVehicleActor::FVehicleActor(
    IdType ActorId,
    AActor* Actor,
    TSharedPtr<const FActorInfo> Info,
    carla::rpc::ActorState InState)
    : FCarlaActor(ActorId, Actor, Info, InState)
{
  Type = ActorType::Vehicle;
  ActorData = MakeShared<FVehicleData>();
}
FSensorActor::FSensorActor(
    IdType ActorId,
    AActor* Actor,
    TSharedPtr<const FActorInfo> Info,
    carla::rpc::ActorState InState)
    : FCarlaActor(ActorId, Actor, Info, InState)
{
  Type = ActorType::Sensor;
  ActorData = MakeShared<FActorSensorData>();
}
FTrafficSignActor::FTrafficSignActor(
    IdType ActorId,
    AActor* Actor,
    TSharedPtr<const FActorInfo> Info,
    carla::rpc::ActorState InState)
    : FCarlaActor(ActorId, Actor, Info, InState)
{
  Type = ActorType::TrafficSign;
  ActorData = MakeShared<FTrafficSignData>();
}
FTrafficLightActor::FTrafficLightActor(
    IdType ActorId,
    AActor* Actor,
    TSharedPtr<const FActorInfo> Info,
    carla::rpc::ActorState InState)
    : FCarlaActor(ActorId, Actor, Info, InState)
{
  Type = ActorType::TrafficLight;
  ActorData = MakeShared<FTrafficLightData>();
}
FWalkerActor::FWalkerActor(
    IdType ActorId,
    AActor* Actor,
    TSharedPtr<const FActorInfo> Info,
    carla::rpc::ActorState InState)
    : FCarlaActor(ActorId, Actor, Info, InState)
{
  Type = ActorType::Walker;
  ActorData = MakeShared<FWalkerData>();
}
FOtherActor::FOtherActor(
    IdType ActorId,
    AActor* Actor,
    TSharedPtr<const FActorInfo> Info,
    carla::rpc::ActorState InState)
    : FCarlaActor(ActorId, Actor, Info, InState)
{
  Type = ActorType::Other;
  ActorData = MakeShared<FActorData>();
}

TSharedPtr<FCarlaActor> FCarlaActor::ConstructCarlaActor(
      IdType ActorId,
      AActor* Actor,
      TSharedPtr<const FActorInfo> Info,
      ActorType Type,
      carla::rpc::ActorState InState)
{
  switch(Type)
  {
  case ActorType::TrafficSign:
    return MakeShared<FTrafficSignActor>(ActorId, Actor, std::move(Info), InState);
    break;
  case ActorType::TrafficLight:
    return MakeShared<FTrafficLightActor>(ActorId, Actor, std::move(Info), InState);
    break;
  case ActorType::Vehicle:
    return MakeShared<FVehicleActor>(ActorId, Actor, std::move(Info), InState);
    break;
  case ActorType::Walker:
    return MakeShared<FWalkerActor>(ActorId, Actor, std::move(Info), InState);
    break;
  case ActorType::Sensor:
    return MakeShared<FSensorActor>(ActorId, Actor, std::move(Info), InState);
    break;
  default:
    return MakeShared<FOtherActor>(ActorId, Actor, std::move(Info), InState);
    break;
  }
}

// Base FCarlaActor functions ---------------------

void FCarlaActor::PutActorToSleep(UCarlaEpisode* CarlaEpisode)
{
  State = carla::rpc::ActorState::Dormant;
  if (ActorData)
  {
    ActorData->RecordActorData(TheActor, CarlaEpisode);
  }
  TheActor->Destroy();
  TheActor = nullptr;
}

void FCarlaActor::WakeActorUp(UCarlaEpisode* CarlaEpisode)
{
  TheActor = ActorData->RespawnActor(CarlaEpisode, *Info);
  if (TheActor == nullptr)
  {
    State = carla::rpc::ActorState::Invalid;
    UE_LOG(LogCarla, Error, TEXT("Could not wake up dormant actor %d at location %s"), GetActorId(), *(ActorData->GetLocalTransform(CarlaEpisode).GetLocation().ToString()));
    return;
  }
  State = carla::rpc::ActorState::Alive;
  ActorData->RestoreActorData(TheActor, CarlaEpisode);
}

ECarlaServerResponse FCarlaActor::SetActorLocation(const FVector& Location, ETeleportType TeleportType)
{
  if (IsDormant())
  {
    ActorData->Location = FDVector(Location);
  }
  else
  {
    GetActor()->SetActorRelativeLocation(
        Location,
        false,
        nullptr,
        TeleportType);
  }
  return ECarlaServerResponse::Success;
}

ECarlaServerResponse FCarlaActor::SetActorTransform(const FTransform& Transform, ETeleportType TeleportType)
{
  if (IsDormant())
  {
    ActorData->Location = FDVector(Transform.GetLocation());
    ActorData->Rotation = Transform.GetRotation();
    ActorData->Scale = Transform.GetScale3D();
  }
  else
  {
    GetActor()->SetActorRelativeTransform(
        Transform,
        false,
        nullptr,
        TeleportType);
  }
  return ECarlaServerResponse::Success;
}

ECarlaServerResponse FCarlaActor::SetActorTargetVelocity(const FVector& Velocity)
{
  if (IsDormant())
  {
    ActorData->Velocity = Velocity;
  }
  else
  {
    auto RootComponent = Cast<UPrimitiveComponent>(GetActor()->GetRootComponent());
    if (RootComponent == nullptr)
    {
      return ECarlaServerResponse::FunctionNotSupported;
    }
    RootComponent->SetPhysicsLinearVelocity(
        Velocity,
        false,
        "None");
  }
  return ECarlaServerResponse::Success;
}

ECarlaServerResponse FCarlaActor::SetActorTargetAngularVelocity(const FVector& AngularVelocity)
{
  if (IsDormant())
  {
    ActorData->AngularVelocity = AngularVelocity;
  }
  else
  {
    UPrimitiveComponent* RootComponent = Cast<UPrimitiveComponent>(GetActor()->GetRootComponent());
    if (RootComponent == nullptr)
    {
      return ECarlaServerResponse::FunctionNotSupported;
    }
    RootComponent->SetPhysicsAngularVelocityInDegrees(
        AngularVelocity,
        false,
        "None");
  }
  return ECarlaServerResponse::Success;
}

ECarlaServerResponse FCarlaActor::AddActorImpulse(const FVector& Impulse)
{
  if (IsDormant())
  {
  }
  else
  {
    auto RootComponent = Cast<UPrimitiveComponent>(GetActor()->GetRootComponent());
    if (RootComponent == nullptr)
    {
      return ECarlaServerResponse::FunctionNotSupported;
    }
    RootComponent->AddImpulse(
        Impulse,
        "None",
        false);
  }
  return ECarlaServerResponse::Success;
}

ECarlaServerResponse FCarlaActor::AddActorImpulseAtLocation(
    const FVector& Impulse, const FVector& Location)
{
  if (IsDormant())
  {
  }
  else
  {
    auto RootComponent = Cast<UPrimitiveComponent>(GetActor()->GetRootComponent());
    if (RootComponent == nullptr)
    {
      return ECarlaServerResponse::FunctionNotSupported;
    }

    UE_LOG(LogCarla, Warning, TEXT("AddImpulseAtLocation: Experimental feature, use carefully."));

    RootComponent->AddImpulseAtLocation(
        Impulse,
        Location,
        "None");
  }
  return ECarlaServerResponse::Success;
}

ECarlaServerResponse FCarlaActor::AddActorForce(const FVector& Force)
{
  if (IsDormant())
  {
  }
  else
  {
    auto RootComponent = Cast<UPrimitiveComponent>(GetActor()->GetRootComponent());
    if (RootComponent == nullptr)
    {
      return ECarlaServerResponse::FunctionNotSupported;
    }
    RootComponent->AddForce(
        Force,
        "None",
        false);
  }
  return ECarlaServerResponse::Success;
}

ECarlaServerResponse FCarlaActor::AddActorForceAtLocation(
    const FVector& Force, const FVector& Location)
{
  if (IsDormant())
  {
  }
  else
  {
    auto RootComponent = Cast<UPrimitiveComponent>(GetActor()->GetRootComponent());
    if (RootComponent == nullptr)
    {
      return ECarlaServerResponse::FunctionNotSupported;
    }

    UE_LOG(LogCarla, Warning, TEXT("AddForceAtLocation: Experimental feature, use carefully."));

    RootComponent->AddForceAtLocation(
        Force,
        Location,
        "None");
  }
  return ECarlaServerResponse::Success;
}

ECarlaServerResponse FCarlaActor::AddActorAngularImpulse(const FVector& AngularInpulse)
{
  if (IsDormant())
  {
  }
  else
  {
    auto RootComponent = Cast<UPrimitiveComponent>(GetActor()->GetRootComponent());
    if (RootComponent == nullptr)
    {
      return ECarlaServerResponse::FunctionNotSupported;
    }
    RootComponent->AddAngularImpulseInDegrees(
        AngularInpulse,
        "None",
        false);
  }
  return ECarlaServerResponse::Success;
}

ECarlaServerResponse FCarlaActor::AddActorTorque(const FVector& Torque)
{
  if (IsDormant())
  {
  }
  else
  {
    auto RootComponent = Cast<UPrimitiveComponent>(GetActor()->GetRootComponent());
    if (RootComponent == nullptr)
    {
      return ECarlaServerResponse::FunctionNotSupported;
    }
    RootComponent->AddTorqueInDegrees(
        Torque,
        "None",
        false);
  }
  return ECarlaServerResponse::Success;
}

// FVehicleActor functions ---------------------

ECarlaServerResponse FVehicleActor::EnableActorConstantVelocity(const FVector& Velocity)
{
  if (IsDormant())
  {
  }
  else
  {
    auto CarlaVehicle = Cast<ACarlaWheeledVehicle>(GetActor());
    if (CarlaVehicle == nullptr)
    {
      return ECarlaServerResponse::NullActor;
    }
    CarlaVehicle->ActivateVelocityControl(Velocity);
  }
  return ECarlaServerResponse::Success;
}

ECarlaServerResponse FVehicleActor::DisableActorConstantVelocity()
{
  if (IsDormant())
  {
  }
  else
  {
    auto CarlaVehicle = Cast<ACarlaWheeledVehicle>(GetActor());
    if (CarlaVehicle == nullptr)
    {
      return ECarlaServerResponse::NullActor;
    }
    CarlaVehicle->DeactivateVelocityControl();
  }
  return ECarlaServerResponse::Success;
}

ECarlaServerResponse FVehicleActor::GetPhysicsControl(FVehiclePhysicsControl& PhysicsControl)
{
  if (IsDormant())
  {
    FVehicleData* ActorData = GetActorData<FVehicleData>();
    PhysicsControl = ActorData->PhysicsControl;
  }
  else
  {
    auto Vehicle = Cast<ACarlaWheeledVehicle>(GetActor());
    if (Vehicle == nullptr)
    {
      return ECarlaServerResponse::NotAVehicle;
    }
    PhysicsControl = Vehicle->GetVehiclePhysicsControl();
  }
  return ECarlaServerResponse::Success;
}

ECarlaServerResponse FVehicleActor::GetVehicleLightState(FVehicleLightState& LightState)
{
  if (IsDormant())
  {
    FVehicleData* ActorData = GetActorData<FVehicleData>();
    LightState = ActorData->LightState;
  }
  else
  {
    auto Vehicle = Cast<ACarlaWheeledVehicle>(GetActor());
    if (Vehicle == nullptr)
    {
      return ECarlaServerResponse::NotAVehicle;
    }

    LightState = Vehicle->GetVehicleLightState();
  }
  return ECarlaServerResponse::Success;
}

// FSensor functions ---------------------

// FtrafficSignActor functions ---------------------

// FTrafficLightActor functions ---------------------

// FWalkerActor functions ---------------------

ECarlaServerResponse FWalkerActor::SetWalkerState(
    const FTransform& Transform,
    carla::rpc::WalkerControl WalkerControl,
    float Speed)
{
  if (IsDormant())
  {
    FWalkerData* WalkerData = GetActorData<FWalkerData>();
    WalkerData->WalkerControl = WalkerControl;
    WalkerData->Location = FDVector(Transform.GetLocation());
    WalkerData->Rotation = Transform.GetRotation();
    WalkerData->Scale = Transform.GetScale3D();
  }
  else
  {
    auto * Walker = Cast<AWalkerBase>(GetActor());
    if (Walker && !Walker->bAlive)
    {
      return ECarlaServerResponse::WalkerDead;
    }
    GetActor()->SetActorRelativeTransform(
        Transform,
        false,
        nullptr,
        ETeleportType::TeleportPhysics);

    // apply walker speed
    auto Pawn = Cast<APawn>(GetActor());
    if (Pawn == nullptr)
    {
      return ECarlaServerResponse::ActorTypeMismatch;
    }
    auto Controller = Cast<AWalkerController>(Pawn->GetController());
    if (Controller == nullptr)
    {
      return ECarlaServerResponse::WalkerIncompatibleController;
    }
    Controller->ApplyWalkerControl(WalkerControl);
  }
  return ECarlaServerResponse::Success;
}
