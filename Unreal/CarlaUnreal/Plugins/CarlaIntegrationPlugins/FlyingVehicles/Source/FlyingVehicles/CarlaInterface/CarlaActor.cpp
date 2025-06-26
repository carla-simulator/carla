// Copyright (c) 2024 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "CarlaActor.h"
#include "FlyingVehicles/MultirotorPawn.h"

const FString FMultirotorActor::CustomType = "FlyingVehicle_Multirotor";

FMultirotorActor::FMultirotorActor(
    IdType ActorId,
    AActor* Actor,
    TSharedPtr<const FActorInfo> Info,
    carla::rpc::ActorState InState,
    UWorld* World)
    : FCarlaActor(ActorId, Actor, Info, InState, World)
{
  Type = ActorType::Custom;
  ActorData = MakeShared<FMultirotorData>();
}

TSharedPtr<FMultirotorActor> FMultirotorActor::CreateInstance(
    FCarlaActor::IdType ActorId, 
    AActor* Actor, 
    TSharedPtr<const FActorInfo> Info, 
    carla::rpc::ActorState InState, 
    UWorld* World)
  {
    return MakeShared<FMultirotorActor>(ActorId, Actor, std::move(Info), InState, World);
  }

void FMultirotorActor::RegisterClassWithFactory()
  {
    CarlaActorConstructorFactory::Instance().Register(TEXT("FMultirotorActor"), AMultirotorPawn::StaticClass(), CreateInstance);
  }

// FMultirotorActor Functions ----------------

ECarlaServerResponse FMultirotorActor::ApplyMultirotorPhysicsControl(
      const FMultirotorPhysicsControl& PhysicsControl)
{
  if (IsDormant())
  {
    FMultirotorData* lActorData = GetActorData<FMultirotorData>();
    lActorData->PhysicsControl = PhysicsControl;
  }
  else
  {
    auto Multirotor = Cast<AMultirotorPawn>(GetActor());
    if (Multirotor == nullptr)
    {
      return ECarlaServerResponse::NotAVehicle;
    }

    Multirotor->ApplyMultirotorPhysicsControl(PhysicsControl);
  }
  return ECarlaServerResponse::Success;
}

ECarlaServerResponse FMultirotorActor::GetMultirotorPhysicsControl(FMultirotorPhysicsControl &PhysicsControl)
{
    if (IsDormant())
    {
        FMultirotorData* lActorData = GetActorData<FMultirotorData>();
        PhysicsControl = lActorData->PhysicsControl;
    }
    else
    {
        auto Multirotor = Cast<AMultirotorPawn>(GetActor());
        if (Multirotor == nullptr)
        {
          return ECarlaServerResponse::NotAVehicle;
        }
        PhysicsControl = Multirotor->GetMultirotorPhysicsControl();
    }
    return ECarlaServerResponse::Success;
}

ECarlaServerResponse FMultirotorActor::ApplyControlToMultirotor(
      const FMultirotorControl& Control, const EVehicleInputPriority& Priority)
{
  if (IsDormant())
  {
    FMultirotorData* lActorData = GetActorData<FMultirotorData>();
    lActorData->Control = Control;
  }
  else
  {
    auto Multirotor = Cast<AMultirotorPawn>(GetActor());
    if (Multirotor == nullptr)
    {
      return ECarlaServerResponse::NotAVehicle;
    }
    Multirotor->ApplyMultirotorControl(Control);
  }
  return ECarlaServerResponse::Success;
}

ECarlaServerResponse FMultirotorActor::GetMultirotorControl(FMultirotorControl& Control)
{
  if (IsDormant())
  {
    FMultirotorData* lActorData = GetActorData<FMultirotorData>();
    Control = lActorData->Control;
  }
  else
  {
    auto Multirotor = Cast<AMultirotorPawn>(GetActor());
    if (Multirotor == nullptr)
    {
      return ECarlaServerResponse::NotAVehicle;
    }
    Control = Multirotor->GetMultirotorControl();
  }
  return ECarlaServerResponse::Success;
}
