// Copyright (c) 2024 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include <util/ue-header-guard-begin.h>
#include "Components/ActorComponent.h"
#include "Components/PrimitiveComponent.h"
#include "CoreMinimal.h"
#include <util/ue-header-guard-end.h>

#include "VehicleVelocityControl.generated.h"

/// Component that controls that the velocity of an actor is constant.
UCLASS(Blueprintable, BlueprintType, ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class CARLA_API UVehicleVelocityControl : public UActorComponent
{
  GENERATED_BODY()

  // ===========================================================================
  /// @name Constructor and destructor
  // ===========================================================================
  /// @{
public:
  UVehicleVelocityControl();

  /// @}
  // ===========================================================================
  /// @name Get functions
  // ===========================================================================
  /// @{
public:

  void BeginPlay() override;

  virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction *ThisTickFunction) override;

  // Activate the component setting the target velocity
  virtual void Activate(bool bReset=false) override;

  // Activate the component setting the target velocity
  virtual void Activate(FVector Velocity, bool bReset=false);

  // Deactivate the component
  virtual void Deactivate() override;

private:
  ///
  UPROPERTY(Category = "Vehicle Velocity Control", VisibleAnywhere)
  FVector TargetVelocity;

  UPrimitiveComponent* PrimitiveComponent;
  AActor* OwnerVehicle;

};
