// Copyright (c) 2019 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "GameFramework/Actor.h"

#include "WalkerAIController.generated.h"

/// Walker AI Controller representation. This actor does nothing, it's a handle
/// for the actual controller created on the client-side.
UCLASS()
class CARLA_API AWalkerAIController : public AActor
{
  GENERATED_BODY()

public:

  AWalkerAIController(const FObjectInitializer &ObjectInitializer)
    : Super(ObjectInitializer)
  {
    PrimaryActorTick.bCanEverTick = false;

    RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));
    RootComponent->bHiddenInGame = true;
  }
};
