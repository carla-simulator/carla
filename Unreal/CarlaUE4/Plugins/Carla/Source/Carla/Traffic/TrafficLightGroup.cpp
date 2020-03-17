// Copyright (c) 2020 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "Carla.h"
#include "TrafficLightGroup.h"


// Sets default values
ATrafficLightGroup::ATrafficLightGroup()
{
  // Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
  PrimaryActorTick.bCanEverTick = true;
  SceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));
  RootComponent = SceneComponent;
}

void ATrafficLightGroup::SetFrozenGroup(bool InFreeze)
{
  bIsFrozen = InFreeze;
}

bool ATrafficLightGroup::IsFrozen()
{
  return bIsFrozen;
}

void ATrafficLightGroup::ResetGroup()
{
  for(auto * Controller : Controllers)
  {
    Controller->ResetState();
  }
  CurrentController = 0;
  if(Controllers.Num() > 0) {
    Timer = Controllers[CurrentController]->NextState();
  } else {
    Timer = 0.0f;
  }
  CurrentStateTimer = Timer;
}

float ATrafficLightGroup::GetElapsedTime() const
{
  return (CurrentStateTimer - Timer);
}

// Called every frame
void ATrafficLightGroup::Tick(float DeltaTime)
{
  Super::Tick(DeltaTime);

  if (bIsFrozen)
  {
    return;
  }

  Timer -= DeltaTime;

  if(Timer <= 0 && Controllers.Num())
  {
    NextCycleStep();
  }
}

void ATrafficLightGroup::NextCycleStep()
{
  UTrafficLightController* controller = Controllers[CurrentController];
  if (controller->IsCycleFinished())
  {
    NextController();
  }
  else
  {
    Timer = controller->NextState();
  }
}

void ATrafficLightGroup::NextController()
{
  CurrentController = (CurrentController + 1) % Controllers.Num();
  UTrafficLightController* controller = Controllers[CurrentController];
  Timer = controller->NextState();
  CurrentStateTimer = Timer;
}

int ATrafficLightGroup::GetJunctionId() const
{
  return JunctionId;
}
