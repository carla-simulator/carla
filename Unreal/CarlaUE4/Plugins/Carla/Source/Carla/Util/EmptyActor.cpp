// Copyright (c) 2024 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "EmptyActor.h"

AEmptyActor::AEmptyActor(const FObjectInitializer &ObjectInitializer)
  : Super(ObjectInitializer)
{
  PrimaryActorTick.bCanEverTick = false;
  RootSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRootComponent"));
  RootComponent = RootSceneComponent;
  RootComponent->SetMobility(EComponentMobility::Movable);
}
