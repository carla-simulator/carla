// Copyright (c) 2020 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "CoreMinimal.h"
#include "SignComponent.h"
#include "Carla/Vehicle/WheeledVehicleAIController.h"
#include "YieldSignComponent.generated.h"

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class CARLA_API UYieldSignComponent : public USignComponent
{
  GENERATED_BODY()

public:

  virtual void InitializeSign(const carla::road::Map &Map) override;

private:

  void GenerateYieldBox(const FTransform BoxTransform,
      float BoxSize);

  void GenerateCheckBox(const FTransform BoxTransform,
      float BoxSize);

  /// Try to give way to vehicles in the Yield
  /// checking the car count in the check boxes
  UFUNCTION(BlueprintCallable)
  void GiveWayIfPossible();

  void DelayedGiveWay(float Delay);

  UFUNCTION(BlueprintCallable)
  void OnOverlapBeginYieldEffectBox(UPrimitiveComponent *OverlappedComp,
      AActor *OtherActor,
      UPrimitiveComponent *OtherComp,
      int32 OtherBodyIndex,
      bool bFromSweep,
      const FHitResult &SweepResult);

  UFUNCTION(BlueprintCallable)
  void OnOverlapEndYieldEffectBox(UPrimitiveComponent *OverlappedComp,
      AActor *OtherActor,
      UPrimitiveComponent *OtherComp,
      int32 OtherBodyIndex);

  UFUNCTION(BlueprintCallable)
  void OnOverlapBeginYieldCheckBox(UPrimitiveComponent *OverlappedComp,
      AActor *OtherActor,
      UPrimitiveComponent *OtherComp,
      int32 OtherBodyIndex,
      bool bFromSweep,
      const FHitResult &SweepResult);

  UFUNCTION(BlueprintCallable)
  void OnOverlapEndYieldCheckBox(UPrimitiveComponent *OverlappedComp,
      AActor *OtherActor,
      UPrimitiveComponent *OtherComp,
      int32 OtherBodyIndex);

  void RemoveSameVehicleInBothLists();

  UPROPERTY()
  TSet<ACarlaWheeledVehicle*> VehiclesInYield;

  UPROPERTY()
  TMap<ACarlaWheeledVehicle*, int> VehiclesToCheck;
};
