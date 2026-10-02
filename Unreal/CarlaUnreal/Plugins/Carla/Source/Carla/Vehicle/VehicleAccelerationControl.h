// Copyright (c) 2026 TIER IV, Inc.
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include <util/ue-header-guard-begin.h>
#include "Components/ActorComponent.h"
#include "Components/PrimitiveComponent.h"
#include "CoreMinimal.h"
#include <util/ue-header-guard-end.h>

#include "VehicleAccelerationControl.generated.h"

/// Component that controls the actor with a constant acceleration (directly applied,
/// no pedal input). Similar to VehicleVelocityControl but for acceleration.
UCLASS(Blueprintable, BlueprintType, ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class CARLA_API UVehicleAccelerationControl : public UActorComponent
{
  GENERATED_BODY()

  // ===========================================================================
  /// @name Constructor and destructor
  // ===========================================================================
  /// @{
public:
  UVehicleAccelerationControl();

  /// @}
  // ===========================================================================
  /// @name Get functions
  // ===========================================================================
  /// @{
public:

  void BeginPlay() override;

  virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction *ThisTickFunction) override;

  // Activate the component setting the target acceleration
  virtual void Activate(bool bReset=false) override;

  // Activate the component setting the target acceleration (in vehicle local space, e.g. X=forward)
  virtual void Activate(FVector Acceleration, bool bReset=false);

  // Deactivate the component
  virtual void Deactivate() override;

private:
  // Target acceleration in vehicle local space (e.g. X forward, Y right, Z up). Units: cm/s^2
  UPROPERTY(Category = "Vehicle Acceleration Control", VisibleAnywhere)
  FVector TargetAcceleration;

  /// Integrated forward speed [cm/s] under our control; lateral component is left to physics for cornering
  float ControlledForwardSpeed = 0.0f;

  UPROPERTY(Transient, DuplicateTransient)
  TObjectPtr<UPrimitiveComponent> PrimitiveComponent;

  UPROPERTY(Transient, DuplicateTransient)
  TObjectPtr<AActor> OwnerVehicle;

};
