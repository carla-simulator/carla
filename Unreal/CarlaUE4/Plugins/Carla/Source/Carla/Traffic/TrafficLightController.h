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

class ATrafficLightGroup;

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

  UFUNCTION(Category = "Traffic Controller", BlueprintCallable)
  void SetStates(TArray<FTrafficLightStage> States);

  UFUNCTION(Category = "Traffic Controller", BlueprintPure)
  const FTrafficLightStage &GetCurrentState() const;

  // Updates traffic light components to the next state
  UFUNCTION(Category = "Traffic Controller", BlueprintCallable)
  float NextState();

  // Advances the counter of the controller and returns true if The cicle is finished
  UFUNCTION(Category = "Traffic Controller", BlueprintCallable)
  bool AdvanceTimeAndCycleFinished(float DeltaTime);

  UFUNCTION(Category = "Traffic Controller", BlueprintCallable)
  void StartCycle();

  UFUNCTION(Category = "Traffic Controller", BlueprintPure)
  const TArray<UTrafficLightComponent *> &GetTrafficLights();

  UFUNCTION(Category = "Traffic Controller", BlueprintCallable)
  void EmptyTrafficLights();

  UFUNCTION(Category = "Traffic Controller", BlueprintPure)
  const FString &GetControllerId() const;

  UFUNCTION(Category = "Traffic Controller", BlueprintCallable)
  void SetControllerId(const FString &Id);

  UFUNCTION(Category = "Traffic Controller", BlueprintCallable)
  void AddTrafficLight(UTrafficLightComponent * TrafficLight);

  UFUNCTION(Category = "Traffic Controller", BlueprintCallable)
  bool IsCycleFinished() const;

  UFUNCTION(Category = "Traffic Controller", BlueprintCallable)
  void SetTrafficLightsState(ETrafficLightState NewState);

  UFUNCTION(Category = "Traffic Controller", BlueprintCallable)
  int GetSequence() const;

  UFUNCTION(Category = "Traffic Controller", BlueprintCallable)
  void SetSequence(int InSequence);

  UFUNCTION(Category = "Traffic Controller", BlueprintCallable)
  void ResetState();

  UFUNCTION(Category = "Traffic Controller", BlueprintCallable)
  void SetYellowTime(float NewTime);

  UFUNCTION(Category = "Traffic Controller", BlueprintCallable)
  void SetRedTime(float NewTime);

  UFUNCTION(Category = "Traffic Controller", BlueprintCallable)
  void SetGreenTime(float NewTime);

  UFUNCTION(Category = "Traffic Controller", BlueprintCallable)
  float GetGreenTime() const;

  UFUNCTION(Category = "Traffic Controller", BlueprintCallable)
  float GetYellowTime() const;

  UFUNCTION(Category = "Traffic Controller", BlueprintCallable)
  float GetRedTime() const;

  UFUNCTION(Category = "Traffic Controller", BlueprintCallable)
  float GetElapsedTime() const;

  UFUNCTION(Category = "Traffic Controller", BlueprintCallable)
  void SetElapsedTime(float InElapsedTime);

  void SetGroup(ATrafficLightGroup* Group);

  ATrafficLightGroup* GetGroup();

  const ATrafficLightGroup* GetGroup() const;

private:

  void SetStateTime(const ETrafficLightState State, float NewTime);

  float GetStateTime(const ETrafficLightState State) const;

  UPROPERTY(Category = "Traffic Controller", EditAnywhere)
  FString ControllerId = "";

  UPROPERTY(Category = "Traffic Controller", EditAnywhere)
  int CurrentState = 0;

  // Pairs with the state of the semaphors (time - state) e.g. 10s in green
  UPROPERTY(Category = "Traffic Controller", EditAnywhere)
  TArray<FTrafficLightStage> LightStates = {
    {10, ETrafficLightState::Green},
    { 3, ETrafficLightState::Yellow},
    { 2, ETrafficLightState::Red}
  };

  UPROPERTY(Category = "Traffic Controller", EditAnywhere)
  TArray<UTrafficLightComponent *> TrafficLights;

  UPROPERTY(Category = "Traffic Controller", VisibleAnywhere)
  ATrafficLightGroup* TrafficLightGroup;

  // Sequence within junction (unused for now)
  UPROPERTY(Category = "Traffic Controller", EditAnywhere)
  int Sequence = 0;

  UPROPERTY()
  float ElapsedTime = 0;

};
