// Copyright (c) 2024 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "CoreMinimal.h"
#include "SignComponent.h"
#include "TrafficLightState.h"
#include "Carla/Vehicle/WheeledVehicleAIController.h"
#include "TrafficLightComponent.generated.h"

class ATrafficLightManager;
class ATrafficLightGroup;
class UTrafficLightController;

// Delegate to define dispatcher
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FLightChangeDispatcher);

/// Class representing an OpenDRIVE Traffic Signal
UCLASS(Blueprintable, ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class CARLA_API UTrafficLightComponent : public USignComponent
{
  GENERATED_BODY()

public:
  // Sets default values for this component's properties
  UTrafficLightComponent();

  UFUNCTION(Category = "Traffic Light", BlueprintCallable)
  void SetLightState(ETrafficLightState NewState);

  UFUNCTION(Category = "Traffic Light", BlueprintCallable)
  ETrafficLightState GetLightState() const;

  UFUNCTION(Category = "Traffic Light", BlueprintCallable)
  void SetFrozenGroup(bool InFreeze);

  UFUNCTION(Category = "Traffic Light", BlueprintPure)
  ATrafficLightGroup* GetGroup();

  const ATrafficLightGroup* GetGroup() const;

  void SetController(UTrafficLightController* Controller);

  UFUNCTION(Category = "Traffic Light", BlueprintPure)
  UTrafficLightController* GetController();

  const UTrafficLightController* GetController() const;

  virtual void InitializeSign(const carla::road::Map &Map) override;

protected:

  UFUNCTION(BlueprintCallable)
  void OnBeginOverlapTriggerBox(UPrimitiveComponent *OverlappedComp,
      AActor *OtherActor,
      UPrimitiveComponent *OtherComp,
      int32 OtherBodyIndex,
      bool bFromSweep,
      const FHitResult &SweepResult);

  UFUNCTION(BlueprintCallable)
  void OnEndOverlapTriggerBox(UPrimitiveComponent *OverlappedComp,
      AActor *OtherActor,
      UPrimitiveComponent *OtherComp,
      int32 OtherBodyIndex);



private:

  friend ATrafficLightManager;

  void GenerateTrafficLightBox(
      const FTransform BoxTransform,
      const FVector BoxSize);

  UPROPERTY(Category = "Traffic Light", EditAnywhere)
  ETrafficLightState LightState;

  UPROPERTY(Category = "Traffic Light", BlueprintAssignable)
  FLightChangeDispatcher LightChangeDispatcher;

  // UPROPERTY(Category = "Traffic Light", VisibleAnywhere)
  // ATrafficLightGroup *TrafficLightGroup = nullptr;

  UPROPERTY(Category = "Traffic Light", VisibleAnywhere)
  UTrafficLightController *TrafficLightController = nullptr;

  // Vehicles that have entered the trigger box of the traffic light
  UPROPERTY()
  TArray<AWheeledVehicleAIController*> Vehicles;

};
