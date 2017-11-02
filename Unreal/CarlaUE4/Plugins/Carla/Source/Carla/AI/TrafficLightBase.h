// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB), and the INTEL Visual Computing Lab.
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "TrafficSignBase.h"
#include "TrafficLightState.h"
#include "TrafficLightBase.generated.h"

class ACarlaWheeledVehicle;
class AWheeledVehicleAIController;

UCLASS()
class CARLA_API ATrafficLightBase : public ATrafficSignBase {

  GENERATED_BODY()

public:

  ATrafficLightBase();

protected:

  virtual void OnConstruction(const FTransform &Transform) override;

#if WITH_EDITOR
  virtual void PostEditChangeProperty(FPropertyChangedEvent &PropertyChangedEvent) override;
#endif // WITH_EDITOR

public:

  UFUNCTION(Category = "Traffic Light", BlueprintCallable)
  ETrafficLightState GetTrafficLightState() const
  {
    return State;
  }

  UFUNCTION(Category = "Traffic Light", BlueprintCallable)
  void SetTrafficLightState(ETrafficLightState State);

  /// Loop over traffic light states.
  UFUNCTION(Category = "Traffic Light", BlueprintCallable)
  void SwitchTrafficLightState();

  UFUNCTION(Category = "Traffic Light", BlueprintCallable)
  void NotifyWheeledVehicle(ACarlaWheeledVehicle *Vehicle);

protected:

  UFUNCTION(Category = "Traffic Light", BlueprintImplementableEvent)
  void OnTrafficLightStateChanged(ETrafficLightState TrafficLightState);

private:

  UPROPERTY(Category = "Traffic Light", EditAnywhere)
  ETrafficLightState State = ETrafficLightState::Red;

  UPROPERTY(Category = "Traffic Light", VisibleAnywhere)
  TArray<AWheeledVehicleAIController *> Vehicles;
};
