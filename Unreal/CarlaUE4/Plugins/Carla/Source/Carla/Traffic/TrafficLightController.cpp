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

bool UTrafficLightController::AdvanceTimeAndCycleFinished(float DeltaTime)
{
  ElapsedTime += DeltaTime;

  if(ElapsedTime > GetCurrentState().Time)
  {
    ElapsedTime = 0;
    if (IsCycleFinished())
    {
      return true;
    }
    NextState();
  }
  return false;
}

void UTrafficLightController::StartCycle()
{
  ElapsedTime = 0;
  CurrentState = 0;
  SetTrafficLightsState(GetCurrentState().State);
}

const TArray<UTrafficLightComponent *> &UTrafficLightController::GetTrafficLights()
{
  return TrafficLights;
}

void UTrafficLightController::EmptyTrafficLights()
{
  TrafficLights.Empty();
  TrafficLightCarlaActors.Empty();
}

void UTrafficLightController::AddTrafficLight(UTrafficLightComponent * TrafficLight)
{
  TrafficLights.Add(TrafficLight);
  TrafficLight->SetController(this);
}

void UTrafficLightController::RemoveTrafficLight(UTrafficLightComponent * TrafficLight)
{
  TrafficLights.Remove(TrafficLight);
}

void UTrafficLightController::AddCarlaActorTrafficLight(FCarlaActor* CarlaActor)
{
  TrafficLightCarlaActors.Add(CarlaActor);
}

void UTrafficLightController::RemoveCarlaActorTrafficLight(FCarlaActor* CarlaActor)
{
  TrafficLightCarlaActors.Remove(CarlaActor);
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
  SetCurrentLightState(NewState);
  for(auto *Light : TrafficLights)
  {
    Light->SetLightState(NewState);
  }
  for(FCarlaActor* Light : TrafficLightCarlaActors)
  {
    Light->SetTrafficLightState(NewState);
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
  ElapsedTime = 0;
}

void UTrafficLightController::SetYellowTime(float NewTime)
{
  SetStateTime(ETrafficLightState::Yellow, NewTime);
}

void UTrafficLightController::SetRedTime(float NewTime)
{
  SetStateTime(ETrafficLightState::Red, NewTime);
}

void UTrafficLightController::SetGreenTime(float NewTime)
{
  SetStateTime(ETrafficLightState::Green, NewTime);
}

float UTrafficLightController::GetGreenTime() const
{
  return GetStateTime(ETrafficLightState::Green);
}

float UTrafficLightController::GetYellowTime() const
{
  return GetStateTime(ETrafficLightState::Yellow);
}

float UTrafficLightController::GetRedTime() const
{
  return GetStateTime(ETrafficLightState::Red);
}

void UTrafficLightController::SetStateTime(const ETrafficLightState State, float NewTime)
{
  for(auto& LightState : LightStates)
  {
    if(LightState.State == State)
    {
      LightState.Time = NewTime;
    }
  }
}

float UTrafficLightController::GetStateTime(const ETrafficLightState State) const
{
  for(auto& LightState : LightStates)
  {
    if(LightState.State == State)
    {
      return LightState.Time;
    }
  }
  return 0.0f;
}

float UTrafficLightController::GetElapsedTime() const
{
  return ElapsedTime;
}

void UTrafficLightController::SetElapsedTime(float InElapsedTime)
{
  ElapsedTime = InElapsedTime;
}

void UTrafficLightController::SetGroup(ATrafficLightGroup* Group)
{
  TrafficLightGroup = Group;
}

ATrafficLightGroup* UTrafficLightController::GetGroup()
{
  return TrafficLightGroup;
}

const ATrafficLightGroup* UTrafficLightController::GetGroup() const
{
  return TrafficLightGroup;
}
