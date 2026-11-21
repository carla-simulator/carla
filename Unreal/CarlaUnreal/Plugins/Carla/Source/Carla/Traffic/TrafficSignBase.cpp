// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "TrafficSignBase.h"
#include "Carla.h"
#include "SignComponent.h"

#include "Components/BoxComponent.h"

ATrafficSignBase::ATrafficSignBase(const FObjectInitializer &ObjectInitializer)
  : Super(ObjectInitializer) {
  PrimaryActorTick.bCanEverTick = false;

  RootComponent =
      ObjectInitializer.CreateDefaultSubobject<USceneComponent>(this, TEXT("SceneRootComponent"));
  RootComponent->SetMobility(EComponentMobility::Static);
}

void ATrafficSignBase::BeginPlay()
{
  Super::BeginPlay();
  if (bVisualOnly)
  {
    // The sign actor underneath carries the trigger volumes; this one's would
    // fire the same stop/yield/speed events a second time.
    TArray<UBoxComponent*> Boxes;
    GetComponents<UBoxComponent>(Boxes);
    for (UBoxComponent* Box : Boxes)
    {
      Box->SetCollisionEnabled(ECollisionEnabled::NoCollision);
      Box->SetGenerateOverlapEvents(false);
    }
  }
}

TArray<UBoxComponent*> ATrafficSignBase::GetTriggerVolumes() const
{
  TArray<USignComponent*> Components;
  GetComponents<USignComponent>(Components, false);
  if (Components.Num())
  {
    USignComponent* SignComponent = Components[0];
    return SignComponent->GetEffectTriggerVolume();
  }
  else
  {
    TArray<UBoxComponent*> TriggerVolumes;
    TriggerVolumes.Add(GetTriggerVolume());
    return TriggerVolumes;
  }
}
