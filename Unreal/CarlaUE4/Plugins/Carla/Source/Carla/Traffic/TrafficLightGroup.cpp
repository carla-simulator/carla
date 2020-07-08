// Copyright (c) 2020 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "Carla.h"
#include "Carla/Game/CarlaStatics.h"
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

bool ATrafficLightGroup::IsFrozen() const
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
  UTrafficLightController* controller = Controllers[CurrentController];
  controller->StartCycle();
}

// Called every frame
void ATrafficLightGroup::Tick(float DeltaTime)
{
  Super::Tick(DeltaTime);

  // Do not update if the replayer is replaying
  auto* Episode = UCarlaStatics::GetCurrentEpisode(GetWorld());
  if (Episode)
  {
    auto* Replayer = Episode->GetReplayer();
    if (Replayer)
    {
      if(Replayer->IsEnabled())
      {
        return;
      }
    }
  }

  if (bIsFrozen)
  {
    return;
  }

  UTrafficLightController* controller = Controllers[CurrentController];
  if (controller->AdvanceTimeAndCycleFinished(DeltaTime))
  {
    NextController();
  }
}

void ATrafficLightGroup::NextController()
{
  CurrentController = (CurrentController + 1) % Controllers.Num();
  UTrafficLightController* controller = Controllers[CurrentController];
  controller->StartCycle();
}

int ATrafficLightGroup::GetJunctionId() const
{
  return JunctionId;
}

void ATrafficLightGroup::AddController(UTrafficLightController* Controller)
{
  Controllers.Add(Controller);
  Controller->SetGroup(this);
}
