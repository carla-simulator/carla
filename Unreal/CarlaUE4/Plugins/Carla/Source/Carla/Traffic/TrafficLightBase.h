// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "Traffic/TrafficSignBase.h"

#include "Traffic/TrafficLightState.h"

#include "TrafficLightBase.generated.h"

class ACarlaWheeledVehicle;
class AWheeledVehicleAIController;

UCLASS()
class CARLA_API ATrafficLightBase : public ATrafficSignBase
{

  GENERATED_BODY()

public:

  ATrafficLightBase(const FObjectInitializer &ObjectInitializer);

  virtual void Tick(float DeltaSeconds) override;

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

protected:

  UFUNCTION(Category = "Traffic Light", BlueprintImplementableEvent)
  void OnTrafficLightStateChanged(ETrafficLightState TrafficLightState);

private:

  UPROPERTY(Category = "Traffic Light", EditAnywhere)
  ETrafficLightState State = ETrafficLightState::Red;

  UPROPERTY(Category = "Traffic Light", VisibleAnywhere)
  TArray<AWheeledVehicleAIController *> Vehicles;

  UPROPERTY(Category = "Traffic Light", EditAnywhere)
  float GreenTime = 10.0f;

  UPROPERTY(Category = "Traffic Light", EditAnywhere)
  float YellowTime = 2.0f;

  UPROPERTY(Category = "Traffic Light", EditAnywhere)
  float RedTime = 7.0f;

  UPROPERTY(Category = "Traffic Light", VisibleAnywhere)
  float ElapsedTime = 0.0f;

  UPROPERTY(Category = "Traffic Light", EditAnywhere)
  bool TimeIsFrozen = false;

  UPROPERTY(Category = "Traffic Light", VisibleAnywhere)
  int PoleIndex = 0;

  UPROPERTY(Category = "Traffic Light", VisibleAnywhere)
  TArray<ATrafficLightBase *> GroupTrafficLights;
};
