// Copyright (c) 2020 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "Kismet/KismetMathLibrary.h"

#include "VehicleVelocityControl.h"


UVehicleVelocityControl::UVehicleVelocityControl()
{
  PrimaryComponentTick.bCanEverTick = true;
}

void UVehicleVelocityControl::BeginPlay()
{
  Super::BeginPlay();

  SetComponentTickEnabled(false);
  SetTickGroup(ETickingGroup::TG_PrePhysics);

  OwnerVehicle = GetOwner();
  PrimitiveComponent = Cast<UPrimitiveComponent>(OwnerVehicle->GetRootComponent());
}

void UVehicleVelocityControl::Activate(bool bReset)
{
  Super::Activate(bReset);

  TargetVelocity = FVector();
  SetComponentTickEnabled(true);
}

void UVehicleVelocityControl::Activate(FVector Velocity, bool bReset)
{
  Super::Activate(bReset);

  TargetVelocity = Velocity;
  SetComponentTickEnabled(true);
}

void UVehicleVelocityControl::Deactivate()
{
  SetComponentTickEnabled(false);
  Super::Deactivate();
}

void UVehicleVelocityControl::TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction *ThisTickFunction)
{
  FTransform Transf = OwnerVehicle->GetActorTransform();
  const FVector LocVel = Transf.TransformVector(TargetVelocity);
  PrimitiveComponent->SetPhysicsLinearVelocity(LocVel, false, "None");
}
