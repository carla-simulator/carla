// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "Carla.h"
#include "TrafficLightBase.h"

#include "Vehicle/CarlaWheeledVehicle.h"
#include "Vehicle/WheeledVehicleAIController.h"

// =============================================================================
// -- Static local methods -----------------------------------------------------
// =============================================================================

static bool IsValid(const ACarlaWheeledVehicle *Vehicle)
{
  return ((Vehicle != nullptr) && !Vehicle->IsPendingKill());
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
  PrimaryActorTick.bCanEverTick = true;
}

void ATrafficLightBase::OnConstruction(const FTransform &Transform)
{
  Super::OnConstruction(Transform);
  SetTrafficLightState(State);
}

void ATrafficLightBase::Tick(float DeltaSeconds)
{
  Super::Tick(DeltaSeconds);

  if (TimeIsFrozen)
  {
    return;
  }

  ElapsedTime += DeltaSeconds;

  float ChangeTime;

  switch (State)
  {
    case ETrafficLightState::Red:
      ChangeTime = RedTime;
      break;
    case ETrafficLightState::Yellow:
      ChangeTime = YellowTime;
      break;
    case ETrafficLightState::Green:
      ChangeTime = GreenTime;
      break;
    default:
      UE_LOG(LogCarla, Error, TEXT("Invalid traffic light state!"));
      SetTrafficLightState(ETrafficLightState::Red);
      return;
  }

  if (ElapsedTime > ChangeTime)
  {
    // Freeze if part of a group and about to turn red
    if (GroupTrafficLights.Num() > 0 && State == ETrafficLightState::Yellow)
    {
      SetTimeIsFrozen(true);
    }
    SwitchTrafficLightState();
  }
}

#if WITH_EDITOR
void ATrafficLightBase::PostEditChangeProperty(FPropertyChangedEvent &Event)
{
  Super::PostEditChangeProperty(Event);

  const FName PropertyName = (Event.Property != nullptr ? Event.Property->GetFName() : NAME_None);
  if (PropertyName == GET_MEMBER_NAME_CHECKED(ATrafficLightBase, State))
  {
    SetTrafficLightState(State);
  }
}
#endif // WITH_EDITOR

void ATrafficLightBase::SetTrafficLightState(const ETrafficLightState InState)
{
  UTrafficLightComponent* TrafficLightComponent =
      Cast<UTrafficLightComponent>(FindComponentByClass<UTrafficLightComponent>());

  if(TrafficLightComponent) {
    TrafficLightComponent->SetLightState(InState);
  } else {
    ElapsedTime = 0.0f;
    State = InState;
    SetTrafficSignState(ToTrafficSignState(State));
    for (auto Controller : Vehicles)
    {
      if (Controller != nullptr)
      {
        Controller->SetTrafficLightState(State);
        if (State == ETrafficLightState::Green)
        {
          Controller->SetTrafficLight(nullptr);
        }
      }
    }
    if (State == ETrafficLightState::Green)
    {
      Vehicles.Empty();
    }
  }
  OnTrafficLightStateChanged(State);
}

void ATrafficLightBase::SwitchTrafficLightState()
{
  switch (State)
  {
    case ETrafficLightState::Red:
      SetTrafficLightState(ETrafficLightState::Green);
      break;
    case ETrafficLightState::Yellow:
      SetTrafficLightState(ETrafficLightState::Red);
      break;
    case ETrafficLightState::Green:
      SetTrafficLightState(ETrafficLightState::Yellow);
      break;
    default:
      UE_LOG(LogCarla, Error, TEXT("Invalid traffic light state!"));
      SetTrafficLightState(ETrafficLightState::Red);
      break;
  }
}

void ATrafficLightBase::NotifyWheeledVehicle(ACarlaWheeledVehicle *Vehicle)
{
  if (IsValid(Vehicle))
  {
    auto Controller = Cast<AWheeledVehicleAIController>(Vehicle->GetController());
    if (Controller != nullptr)
    {
      Controller->SetTrafficLightState(State);
      if (State != ETrafficLightState::Green)
      {
        Vehicles.Add(Controller);
        Controller->SetTrafficLight(this);
      }
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
  UTrafficLightComponent* TrafficLightComponent =
      Cast<UTrafficLightComponent>(FindComponentByClass<UTrafficLightComponent>());

  if(TrafficLightComponent) {
    UTrafficLightController* TrafficLightController =
      TrafficLightComponent->GetController();
    check(TrafficLightController)
    TrafficLightController->SetGreenTime(InGreenTime);

  } else {
    GreenTime = InGreenTime;
  }
}

float ATrafficLightBase::GetGreenTime() const
{
  return GreenTime;
}

void ATrafficLightBase::SetYellowTime(float InYellowTime)
{
  UTrafficLightComponent* TrafficLightComponent =
      Cast<UTrafficLightComponent>(FindComponentByClass<UTrafficLightComponent>());

  if(TrafficLightComponent) {
    UTrafficLightController* TrafficLightController =
      TrafficLightComponent->GetController();
    check(TrafficLightController)
    TrafficLightController->SetYellowTime(InYellowTime);
  } else {
    YellowTime = InYellowTime;
  }
}

float ATrafficLightBase::GetYellowTime() const
{
  return YellowTime;
}

void ATrafficLightBase::SetRedTime(float InRedTime)
{
  UTrafficLightComponent* TrafficLightComponent =
      Cast<UTrafficLightComponent>(FindComponentByClass<UTrafficLightComponent>());

  if(TrafficLightComponent) {
    UTrafficLightController* TrafficLightController =
      TrafficLightComponent->GetController();
    check(TrafficLightController)
    TrafficLightController->SetRedTime(InRedTime);
  } else {
    RedTime = InRedTime;
  }
}

float ATrafficLightBase::GetRedTime() const
{
  return RedTime;
}

float ATrafficLightBase::GetElapsedTime() const
{
  return ElapsedTime;
}

void ATrafficLightBase::SetElapsedTime(float InElapsedTime)
{
  ElapsedTime = InElapsedTime;
}

void ATrafficLightBase::SetTimeIsFrozen(bool InTimeIsFrozen)
{
  UTrafficLightComponent* TrafficLightComponent =
      Cast<UTrafficLightComponent>(FindComponentByClass<UTrafficLightComponent>());

  if(TrafficLightComponent) {
    TrafficLightComponent->SetFrozenGroup(InTimeIsFrozen);
  } else {
    TimeIsFrozen = InTimeIsFrozen;
    if (!TimeIsFrozen)
    {
      ElapsedTime = 0.0f;
    }
  }
}

bool ATrafficLightBase::GetTimeIsFrozen() const
{
  return TimeIsFrozen;
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
  UTrafficLightComponent* TrafficLightComponent =
      Cast<UTrafficLightComponent>(FindComponentByClass<UTrafficLightComponent>());

  if(TrafficLightComponent) {
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
