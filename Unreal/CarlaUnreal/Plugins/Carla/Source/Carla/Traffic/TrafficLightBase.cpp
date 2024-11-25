// Copyright (c) 2024 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "TrafficLightBase.h"
#include "Carla.h"
#include "Carla/Game/CarlaStatics.h"
#include "Vehicle/CarlaWheeledVehicle.h"
#include "Carla/Vehicle/WheeledVehicleAIController.h"

// =============================================================================
// -- Static local methods -----------------------------------------------------
// =============================================================================

static bool IsValid(const ACarlaWheeledVehicle *Vehicle)
{
  return ((Vehicle != nullptr) && IsValidChecked(Vehicle));
}

static ETrafficSignState ToTrafficSignState(ETrafficLightState State)
{
  switch (State)
  {
    case ETrafficLightState::Green:
      return ETrafficSignState::TrafficLightGreen;
    case ETrafficLightState::Yellow:
      return ETrafficSignState::TrafficLightYellow;
    default:
    case ETrafficLightState::Red:
      return ETrafficSignState::TrafficLightRed;
  }
}

// =============================================================================
// -- ATrafficLightBase --------------------------------------------------------
// =============================================================================

ATrafficLightBase::ATrafficLightBase(const FObjectInitializer &ObjectInitializer)
  : Super(ObjectInitializer)
{
  PrimaryActorTick.bCanEverTick = false;
  TrafficLightComponent = CreateDefaultSubobject<UTrafficLightComponent>(TEXT("TrafficLightComponent"));
  if(TrafficLightComponent && RootComponent)
  {
    TrafficLightComponent->SetupAttachment(RootComponent);
  }
  SetTrafficSignState(ETrafficSignState::TrafficLightGreen);
}

ETrafficLightState ATrafficLightBase::GetTrafficLightState() const
{
  if (TrafficLightComponent)
  {
    return TrafficLightComponent->GetLightState();
  }
  else
  {
    return ETrafficLightState::Red;
  }
}

void ATrafficLightBase::SetTrafficLightState(const ETrafficLightState InState)
{
  if(TrafficLightComponent)
  {
    TrafficLightComponent->SetLightState(InState);
  }
  // OnTrafficLightStateChanged(State);
}

void ATrafficLightBase::NotifyWheeledVehicle(ACarlaWheeledVehicle *Vehicle)
{
  if (IsValid(Vehicle))
  {
    auto Controller = Cast<AWheeledVehicleAIController>(Vehicle->GetController());
    if (Controller != nullptr)
    {
      Controller->SetTrafficLightState(GetTrafficLightState());
      Vehicles.Add(Controller);
      Controller->SetTrafficLight(this);
    }
  }
}

void ATrafficLightBase::UnNotifyWheeledVehicle(ACarlaWheeledVehicle *Vehicle)
{
  if (IsValid(Vehicle))
  {
    auto Controller = Cast<AWheeledVehicleAIController>(Vehicle->GetController());
    if (Controller != nullptr)
    {
      Controller->SetTrafficLight(nullptr);
      Controller->SetTrafficLightState(ETrafficLightState::Green);
    }
  }
}

void ATrafficLightBase::SetGreenTime(float InGreenTime)
{
  if(TrafficLightComponent)
  {
    UTrafficLightController* TrafficLightController =
        TrafficLightComponent->GetController();
    check(TrafficLightController)
    TrafficLightController->SetGreenTime(InGreenTime);
    AddTimeToRecorder();
  }
}

float ATrafficLightBase::GetGreenTime() const
{
  if (TrafficLightComponent)
  {
    auto* Controller = TrafficLightComponent->GetController();
    check(Controller);
    return Controller->GetGreenTime();
  }
  else
  {
    return 0;
  }
}

void ATrafficLightBase::SetYellowTime(float InYellowTime)
{
  if(TrafficLightComponent)
  {
    UTrafficLightController* TrafficLightController =
      TrafficLightComponent->GetController();
    check(TrafficLightController)
    TrafficLightController->SetYellowTime(InYellowTime);
    AddTimeToRecorder();
  }
}

float ATrafficLightBase::GetYellowTime() const
{
  if (TrafficLightComponent)
  {
    auto* Controller = TrafficLightComponent->GetController();
    check(Controller);
    return Controller->GetYellowTime();
  }
  else
  {
    return 0;
  }
}

void ATrafficLightBase::SetRedTime(float InRedTime)
{
  if(TrafficLightComponent)
  {
    UTrafficLightController* TrafficLightController =
      TrafficLightComponent->GetController();
    check(TrafficLightController)
    TrafficLightController->SetRedTime(InRedTime);
    AddTimeToRecorder();
  }
}

float ATrafficLightBase::GetRedTime() const
{
  if (TrafficLightComponent)
  {
    auto* Controller = TrafficLightComponent->GetController();
    check(Controller);
    return Controller->GetRedTime();
  }
  else
  {
    return 0;
  }
}

float ATrafficLightBase::GetElapsedTime() const
{
  if (TrafficLightComponent)
  {
    auto* Controller = TrafficLightComponent->GetController();
    check(Controller);
    return Controller->GetElapsedTime();
  }
  else
  {
    return 0;
  }
}

void ATrafficLightBase::SetElapsedTime(float InElapsedTime)
{
  if (TrafficLightComponent)
  {
    auto* Controller = TrafficLightComponent->GetController();
    check(Controller);
    return Controller->SetElapsedTime(InElapsedTime);
  }
}

void ATrafficLightBase::SetTimeIsFrozen(bool InTimeIsFrozen)
{
  if(TrafficLightComponent)
  {
    TrafficLightComponent->SetFrozenGroup(InTimeIsFrozen);
  }
}

bool ATrafficLightBase::GetTimeIsFrozen() const
{
  if(TrafficLightComponent)
  {
    auto* Group = TrafficLightComponent->GetGroup();
    check(Group);
    return Group->IsFrozen();
  }
  return false;
}

void ATrafficLightBase::SetPoleIndex(int InPoleIndex)
{
  PoleIndex = InPoleIndex;
}

int ATrafficLightBase::GetPoleIndex() const
{
  return PoleIndex;
}

TArray<ATrafficLightBase *> ATrafficLightBase::GetGroupTrafficLights() const
{
  if(TrafficLightComponent)
  {
    TArray<ATrafficLightBase *> result;

    ATrafficLightGroup* Group = TrafficLightComponent->GetGroup();
    check(Group)

    for(auto& Controller : Group->GetControllers())
    {
      for(auto& TLComp : Controller->GetTrafficLights())
      {
        result.Add(Cast<ATrafficLightBase>(TLComp->GetOwner()));
      }
    }

    return result;
  }
  return GroupTrafficLights;
}

void ATrafficLightBase::SetGroupTrafficLights(TArray<ATrafficLightBase *> InGroupTrafficLights)
{
  GroupTrafficLights = InGroupTrafficLights;
}

UTrafficLightComponent* ATrafficLightBase::GetTrafficLightComponent()
{
  return TrafficLightComponent;
}
const UTrafficLightComponent* ATrafficLightBase::GetTrafficLightComponent() const
{
  return TrafficLightComponent;
}

void ATrafficLightBase::AddTimeToRecorder()
{
  auto * Recorder = UCarlaStatics::GetRecorder(GetWorld());
  if (Recorder && Recorder->IsEnabled())
  {
    Recorder->AddTrafficLightTime(*this);
  }
}

void ATrafficLightBase::LightChangedCompatibility(ETrafficLightState NewLightState)
{
  OnTrafficLightStateChanged(NewLightState);
}
