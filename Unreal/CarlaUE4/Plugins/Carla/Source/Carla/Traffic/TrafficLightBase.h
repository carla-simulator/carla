// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "Traffic/TrafficSignBase.h"

#include "Traffic/TrafficLightState.h"
#include "Traffic/TrafficLightComponent.h"

#include "TrafficLightBase.generated.h"

class ACarlaWheeledVehicle;
class AWheeledVehicleAIController;

UCLASS()
class CARLA_API ATrafficLightBase : public ATrafficSignBase
{

  GENERATED_BODY()

public:

  ATrafficLightBase(const FObjectInitializer &ObjectInitializer);

  UFUNCTION(Category = "Traffic Light", BlueprintCallable)
  ETrafficLightState GetTrafficLightState() const;

  UFUNCTION(Category = "Traffic Light", BlueprintCallable)
  void SetTrafficLightState(ETrafficLightState State);

  UFUNCTION(Category = "Traffic Light", BlueprintCallable)
  void NotifyWheeledVehicle(ACarlaWheeledVehicle *Vehicle);

  UFUNCTION(Category = "Traffic Light", BlueprintCallable)
  void UnNotifyWheeledVehicle(ACarlaWheeledVehicle *Vehicle);

  UFUNCTION(Category = "Traffic Light", BlueprintCallable)
  void SetGreenTime(float InGreenTime);

  UFUNCTION(Category = "Traffic Light", BlueprintCallable)
  float GetGreenTime() const;

  UFUNCTION(Category = "Traffic Light", BlueprintCallable)
  void SetYellowTime(float InYellowTime);

  UFUNCTION(Category = "Traffic Light", BlueprintCallable)
  float GetYellowTime() const;

  UFUNCTION(Category = "Traffic Light", BlueprintCallable)
  void SetRedTime(float InRedTime);

  UFUNCTION(Category = "Traffic Light", BlueprintCallable)
  float GetRedTime() const;

  UFUNCTION(Category = "Traffic Light", BlueprintCallable)
  float GetElapsedTime() const;

  UFUNCTION(Category = "Traffic Light", BlueprintCallable)
  void SetTimeIsFrozen(bool InTimeIsFrozen);

  UFUNCTION(Category = "Traffic Light", BlueprintCallable)
  bool GetTimeIsFrozen() const;

  UFUNCTION(Category = "Traffic Light", BlueprintCallable)
  void SetPoleIndex(int InPoleIndex);

  UFUNCTION(Category = "Traffic Light", BlueprintCallable)
  int GetPoleIndex() const;

  UFUNCTION(Category = "Traffic Light", BlueprintCallable)
  TArray<ATrafficLightBase *> GetGroupTrafficLights() const;

  UFUNCTION(Category = "Traffic Light", BlueprintCallable)
  void SetGroupTrafficLights(TArray<ATrafficLightBase *> InGroupTrafficLights);

  // used from replayer
  void SetElapsedTime(float InElapsedTime);

  UFUNCTION(Category = "Traffic Light", BlueprintPure)
  UTrafficLightComponent* GetTrafficLightComponent();

  const UTrafficLightComponent* GetTrafficLightComponent() const;

  // Compatibility old traffic light system with traffic light components
  void LightChangedCompatibility(ETrafficLightState NewLightState);

  void AddTimeToRecorder();

protected:

  UFUNCTION(Category = "Traffic Light", BlueprintImplementableEvent)
  void OnTrafficLightStateChanged(ETrafficLightState TrafficLightState);

private:

  UPROPERTY(Category = "Traffic Light", VisibleAnywhere)
  TArray<AWheeledVehicleAIController *> Vehicles;

  UPROPERTY(Category = "Traffic Light", VisibleAnywhere)
  int PoleIndex = 0;

  UPROPERTY(Category = "Traffic Light", VisibleAnywhere)
  TArray<ATrafficLightBase *> GroupTrafficLights;

  UPROPERTY(Category = "Traffic Light", EditAnywhere)
  UTrafficLightComponent * TrafficLightComponent = nullptr;
};
