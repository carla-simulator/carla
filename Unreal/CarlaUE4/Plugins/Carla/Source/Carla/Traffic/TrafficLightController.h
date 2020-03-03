// Copyright (c) 2020 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "CoreMinimal.h"
#include "Object.h"
#include "TrafficLightState.h"
#include "TrafficLightComponent.h"
#include "Containers/Map.h"
#include "TrafficLightController.generated.h"

/// Defines a stage of a semaphor with a State and
/// the time this state lasts
USTRUCT(BlueprintType)
struct FTrafficLightStage
{
  GENERATED_BODY()

  UPROPERTY(EditAnywhere, BlueprintReadWrite)
  float Time;

  UPROPERTY(EditAnywhere, BlueprintReadWrite)
  ETrafficLightState State;
};

/// Maps a controller from OpenDrive.
/// Controls the asociated traffic lights and contains its cycles
UCLASS(BlueprintType)
class CARLA_API UTrafficLightController : public UObject
{
  GENERATED_BODY()

public:

  UTrafficLightController();

  UFUNCTION(BlueprintCallable)
  void SetStates(TArray<FTrafficLightStage> States);

  UFUNCTION(BlueprintPure)
  const FTrafficLightStage &GetCurrentState() const;

  // Updates traffic light components to the next state
  UFUNCTION(BlueprintCallable)
  float NextState();

  UFUNCTION(BlueprintPure)
  const TArray<UTrafficLightComponent *> &GetTrafficLights();

  UFUNCTION(BlueprintPure)
  const FString &GetControllerId() const;

  UFUNCTION(BlueprintCallable)
  void SetControllerId(const FString &Id);

  UFUNCTION(BlueprintCallable)
  void AddTrafficLight(UTrafficLightComponent * TrafficLight);

  UFUNCTION(BlueprintCallable)
  bool IsCycleFinished() const;

  UFUNCTION(BlueprintCallable)
  void SetTrafficLightsState(ETrafficLightState NewState);

  UFUNCTION(BlueprintCallable)
  int GetSequence() const;

  UFUNCTION(BlueprintCallable)
  void SetSequence(int InSequence);

  UFUNCTION(BlueprintCallable)
  void ResetState();
private:

  UPROPERTY(EditAnywhere)
  FString ControllerId;

  UPROPERTY(EditAnywhere)
  int CurrentState = 0;

  // Pairs with the state of the semaphors (time - state) e.g. 10s in green
  UPROPERTY(EditAnywhere)
  TArray<FTrafficLightStage> LightStates = {
      {10, ETrafficLightState::Green}, {3, ETrafficLightState::Yellow}, {2, ETrafficLightState::Red}};

  UPROPERTY(EditAnywhere)
  TArray<UTrafficLightComponent *> TrafficLights;

  // Sequence within junction (unused for now)
  UPROPERTY(EditAnywhere)
  int Sequence = 0;
};
