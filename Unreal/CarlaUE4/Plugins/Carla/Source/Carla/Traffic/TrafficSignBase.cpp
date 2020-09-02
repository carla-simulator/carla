// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "Carla.h"
#include "TrafficSignBase.h"
#include "SignComponent.h"

ATrafficSignBase::ATrafficSignBase(const FObjectInitializer &ObjectInitializer)
  : Super(ObjectInitializer) {
  PrimaryActorTick.bCanEverTick = false;

  RootComponent =
      ObjectInitializer.CreateDefaultSubobject<USceneComponent>(this, TEXT("SceneRootComponent"));
  RootComponent->SetMobility(EComponentMobility::Static);
}

TArray<UBoxComponent*> ATrafficSignBase::GetTriggerVolumes() const
{
  TArray<USignComponent*> Components;
  GetComponents<USignComponent>(Components, false);
  if (Components.Num())
  {
    USignComponent* SignComponent = Components.Top();
    return SignComponent->GetEffectTriggerVolume();
  }
  else
  {
    TArray<UBoxComponent*> TriggerVolumes;
    TriggerVolumes.Add(GetTriggerVolume());
    return TriggerVolumes;
  }
}
