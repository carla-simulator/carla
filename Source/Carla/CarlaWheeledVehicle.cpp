// CARLA, Copyright (C) 2017 Computer Vision Center (CVC)

#include "Carla.h"
#include "CarlaWheeledVehicle.h"

#include "Components/BoxComponent.h"
#include "Engine/CollisionProfile.h"

// =============================================================================
// -- Constructor and destructor -----------------------------------------------
// =============================================================================

ACarlaWheeledVehicle::ACarlaWheeledVehicle(const FObjectInitializer& ObjectInitializer) :
  Super(ObjectInitializer)
{
  VehicleBounds = CreateDefaultSubobject<UBoxComponent>(TEXT("VehicleBounds"));
  VehicleBounds->SetupAttachment(RootComponent);
  VehicleBounds->SetHiddenInGame(true);
  VehicleBounds->SetCollisionProfileName(UCollisionProfile::NoCollision_ProfileName);
}

ACarlaWheeledVehicle::~ACarlaWheeledVehicle() {}

// =============================================================================
// -- Get functions ------------------------------------------------------------
// =============================================================================

float ACarlaWheeledVehicle::GetVehicleForwardSpeed() const
{
  return GetVehicleMovementComponent()->GetForwardSpeed() * 0.036f;
}

FVector ACarlaWheeledVehicle::GetVehicleOrientation() const
{
  return GetActorTransform().GetRotation().GetForwardVector();
}

int32 ACarlaWheeledVehicle::GetVehicleCurrentGear() const
{
  return GetVehicleMovementComponent()->GetCurrentGear();
}

FVector ACarlaWheeledVehicle::GetVehicleBoundsExtent() const
{
  return VehicleBounds->GetScaledBoxExtent();
}

// =============================================================================
// -- Set functions ------------------------------------------------------------
// =============================================================================

void ACarlaWheeledVehicle::SetThrottleInput(const float Value)
{
  GetVehicleMovementComponent()->SetThrottleInput(Value);
}

void ACarlaWheeledVehicle::SetSteeringInput(const float Value)
{
  GetVehicleMovementComponent()->SetSteeringInput(Value);
}

void ACarlaWheeledVehicle::SetBrakeInput(const float Value)
{
  GetVehicleMovementComponent()->SetBrakeInput(Value);
}

void ACarlaWheeledVehicle::SetReverse(const bool Value)
{
  if (Value != bIsInReverse) {
    bIsInReverse = Value;
    auto MovementComponent = GetVehicleMovementComponent();
    MovementComponent->SetUseAutoGears(!bIsInReverse);
    MovementComponent->SetTargetGear(bIsInReverse ? -1 : 1, true);
  }
}

void ACarlaWheeledVehicle::SetHandbrakeInput(const bool Value)
{
  GetVehicleMovementComponent()->SetHandbrakeInput(Value);
}
