// Copyright (c) 2020 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "CoreMinimal.h"
#include "SignComponent.h"
#include "TrafficLightState.h"
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

  UFUNCTION(Category = "Traffic Light", BlueprintPure)
  UTrafficLightController* GetController();

protected:
  // Called when the game starts
  virtual void BeginPlay() override;

  // Called every frame
  virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:

  friend ATrafficLightManager;
  UPROPERTY(Category = "Traffic Light", EditAnywhere)
  ETrafficLightState LightState;

  UPROPERTY(Category = "Traffic Light", BlueprintAssignable)
  FLightChangeDispatcher LightChangeDispatcher;

  UPROPERTY()
  ATrafficLightGroup *TrafficLightGroup = nullptr;

  UPROPERTY()
  UTrafficLightController *TrafficLightController = nullptr;

};
