// Copyright (c) 2021 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
// Copyright (c) 2019 Intel Corporation
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "BaseCarlaMovementComponent.h"
#include "Carla/Vehicle/VehicleControl.h"

#ifdef WITH_CARSIM
#include "CarSimMovementComponent.h"
#endif

#include "CarSimManagerComponent.generated.h"

class ACarlaWheeledVehicle;

UCLASS(Blueprintable, meta=(BlueprintSpawnableComponent) )
class CARLA_API UCarSimManagerComponent : public UBaseCarlaMovementComponent
{
  GENERATED_BODY()

  #ifdef WITH_CARSIM
  AActor* OffsetActor;

  UCarSimMovementComponent * CarSimMovementComponent;
  #endif

public:

  static void CreateCarsimComponent(
      ACarlaWheeledVehicle* Vehicle, FString Simfile);

  FString SimfilePath = "";

  virtual void BeginPlay() override;

  virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

  void ProcessControl(FVehicleControl &Control) override;

  FVector GetVelocity() const override;

  virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

  void UseCarSimRoad(bool bEnabled);

  int32 GetVehicleCurrentGear() const override;

  float GetVehicleForwardSpeed() const override;

private:

  // On car mesh hit, only works when carsim is enabled
  UFUNCTION()
  void OnCarSimHit(AActor *Actor,
      AActor *OtherActor,
      FVector NormalImpulse,
      const FHitResult &Hit);

  // On car mesh overlap, only works when carsim is enabled
  // (this event triggers when overlapping with static environment)
  UFUNCTION()
  void OnCarSimOverlap(UPrimitiveComponent* OverlappedComponent,
      AActor* OtherActor,
      UPrimitiveComponent* OtherComp,
      int32 OtherBodyIndex,
      bool bFromSweep,
      const FHitResult & SweepResult);

  // On car mesh overlap end, only works when carsim is enabled
  // (this event triggers when overlapping with static environment)
  UFUNCTION()
  void OnCarSimEndOverlap(UPrimitiveComponent* OverlappedComponent,
      AActor* OtherActor,
      UPrimitiveComponent* OtherComp,
      int32 OtherBodyIndex);
};
