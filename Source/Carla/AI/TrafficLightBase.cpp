// CARLA, Copyright (C) 2017 Computer Vision Center (CVC)

#include "Carla.h"
#include "TrafficLightBase.h"

#include "AI/WheeledVehicleAIController.h"
#include "CarlaWheeledVehicle.h"

// =============================================================================
// -- Static local methods -----------------------------------------------------
// =============================================================================

static bool IsValid(const ACarlaWheeledVehicle *Vehicle)
{
  return ((Vehicle != nullptr) && !Vehicle->IsPendingKill());
}

// =============================================================================
// -- ATrafficLightBase --------------------------------------------------------
// =============================================================================

ATrafficLightBase::ATrafficLightBase() : Super()
{
	PrimaryActorTick.bCanEverTick = false;
}

void ATrafficLightBase::OnConstruction(const FTransform &Transform)
{
  Super::OnConstruction(Transform);
  SetTrafficLightState(State);
}

#if WITH_EDITOR
void ATrafficLightBase::PostEditChangeProperty(FPropertyChangedEvent &Event)
{
  Super::PostEditChangeProperty(Event);

  const FName PropertyName = (Event.Property != nullptr ? Event.Property->GetFName() : NAME_None);
  if (PropertyName == GET_MEMBER_NAME_CHECKED(ATrafficLightBase, State)) {
    SetTrafficLightState(State);
  }
}
#endif // WITH_EDITOR

void ATrafficLightBase::SetTrafficLightState(const ETrafficLightState InState)
{
  State = InState;
  for (auto Controller : Vehicles) {
    if (Controller != nullptr) {
      Controller->SetTrafficLightState(State);
    }
  }
  if (State == ETrafficLightState::Green) {
    Vehicles.Empty();
  }
  OnTrafficLightStateChanged(State);
}

void ATrafficLightBase::SwitchTrafficLightState()
{
  switch (State) {
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
  // If green, do not notify vehicle.
  if ((State != ETrafficLightState::Green) && IsValid(Vehicle)) {
    auto Controller = Cast<AWheeledVehicleAIController>(Vehicle->GetController());
    if (Controller != nullptr) {
      Controller->SetTrafficLightState(State);
      Vehicles.Add(Controller);
    }
  }
}
