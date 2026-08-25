// Copyright (c) 2026 TIER IV, Inc.
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

// TODO: Add gear-aware input (e.g. allow reverse when in reverse gear, keep forward-only in drive).

#include "VehicleAccelerationControl.h"
#include "Carla/Vehicle/CarlaWheeledVehicle.h"

UVehicleAccelerationControl::UVehicleAccelerationControl()
{
  PrimaryComponentTick.bCanEverTick = true;
}

void UVehicleAccelerationControl::BeginPlay()
{
  Super::BeginPlay();

  SetComponentTickEnabled(false);
  // Run after physics so our velocity override is applied after the vehicle simulation
  SetTickGroup(ETickingGroup::TG_PostPhysics);

  OwnerVehicle = GetOwner();
  ACarlaWheeledVehicle* CarlaVehicle = Cast<ACarlaWheeledVehicle>(OwnerVehicle);
  if (CarlaVehicle && CarlaVehicle->GetMesh())
  {
    PrimitiveComponent = Cast<UPrimitiveComponent>(CarlaVehicle->GetMesh());
  }
  if (PrimitiveComponent == nullptr)
  {
    PrimitiveComponent = Cast<UPrimitiveComponent>(OwnerVehicle->GetRootComponent());
  }
  ControlledForwardSpeed = 0.f;
}

void UVehicleAccelerationControl::Activate(bool bReset)
{
  Super::Activate(bReset);

  TargetAcceleration = FVector();
  SetComponentTickEnabled(true);
}

void UVehicleAccelerationControl::Activate(FVector Acceleration, bool bReset)
{
  Super::Activate(bReset);

  TargetAcceleration = Acceleration;
  if (PrimitiveComponent != nullptr)
  {
    const FVector Vel = PrimitiveComponent->GetPhysicsLinearVelocity();
    const FVector Forward = OwnerVehicle->GetActorTransform().TransformVectorNoScale(FVector(1, 0, 0));
    const FVector ForwardDir = Forward.GetSafeNormal();
    ControlledForwardSpeed = FVector::DotProduct(Vel, ForwardDir);
  }
  else
  {
    ControlledForwardSpeed = 0.f;
  }
  SetComponentTickEnabled(true);
}

void UVehicleAccelerationControl::Deactivate()
{
  SetComponentTickEnabled(false);
  Super::Deactivate();
}

void UVehicleAccelerationControl::TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction *ThisTickFunction)
{
  TRACE_CPUPROFILER_EVENT_SCOPE(UVehicleAccelerationControl::TickComponent);
  if (PrimitiveComponent == nullptr)
  {
    return;
  }
  PrimitiveComponent->WakeRigidBody(NAME_None);

  const FTransform Transf = OwnerVehicle->GetActorTransform();
  const FVector ForwardDir = Transf.TransformVectorNoScale(FVector(1, 0, 0)).GetSafeNormal();
  const FVector WorldAcceleration = Transf.TransformVector(TargetAcceleration);
  const float ForwardAccel = FVector::DotProduct(WorldAcceleration, ForwardDir);

  // Integrate only forward speed; preserve lateral velocity from physics so tires can generate cornering force
  ControlledForwardSpeed += ForwardAccel * DeltaTime;
  // Clamp to non-negative so we never go into reverse
  // TODO: Add gear-aware input (e.g. allow reverse when in reverse gear, keep forward-only in drive).
  ControlledForwardSpeed = FMath::Max(0.f, ControlledForwardSpeed);

  const FVector CurrentVel = PrimitiveComponent->GetPhysicsLinearVelocity();
  const FVector LateralVel = CurrentVel - FVector::DotProduct(CurrentVel, ForwardDir) * ForwardDir;
  const FVector NewVel = (ControlledForwardSpeed * ForwardDir) + LateralVel;

  PrimitiveComponent->SetPhysicsLinearVelocity(NewVel, false, "None");
}
