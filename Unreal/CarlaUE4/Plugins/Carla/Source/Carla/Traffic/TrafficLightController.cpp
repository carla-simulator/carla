// Copyright (c) 2020 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "TrafficLightController.h"

UTrafficLightController::UTrafficLightController()
{

}

void UTrafficLightController::SetStates(TArray<FTrafficLightStage> States)
{
  LightStates = States;
  ResetState();
}

const FTrafficLightStage &UTrafficLightController::GetCurrentState() const
{
  return LightStates[CurrentState];
}

float UTrafficLightController::NextState()
{
  CurrentState = (CurrentState + 1) % LightStates.Num();
  SetTrafficLightsState(GetCurrentState().State);
  return GetCurrentState().Time;
}

const TArray<UTrafficLightComponent *> &UTrafficLightController::GetTrafficLights()
{
  return TrafficLights;
}

void UTrafficLightController::AddTrafficLight(UTrafficLightComponent * TrafficLight)
{
  TrafficLights.Add(TrafficLight);
}

const FString &UTrafficLightController::GetControllerId() const
{
  return ControllerId;
}

void UTrafficLightController::SetControllerId(const FString &Id)
{
  ControllerId = Id;
}

bool UTrafficLightController::IsCycleFinished() const
{
  return CurrentState == (LightStates.Num() - 1);
}

void UTrafficLightController::SetTrafficLightsState(ETrafficLightState NewState)
{
  for(auto *Light : TrafficLights)
  {
    Light->SetLightState(NewState);
  }
}

int UTrafficLightController::GetSequence() const
{
  return Sequence;
}

void UTrafficLightController::SetSequence(int InSequence)
{
  Sequence = InSequence;
}

void UTrafficLightController::ResetState()
{
  CurrentState = (LightStates.Num() - 1);
  SetTrafficLightsState(GetCurrentState().State);
}
