// Copyright (c) 2024 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include <util/ue-header-guard-begin.h>
#include "Engine/World.h"
#include <util/ue-header-guard-end.h>

#include "TaggerDelegate.generated.h"

/// Used to tag every actor that is spawned into the world.
UCLASS()
class CARLA_API UTaggerDelegate : public UObject
{
  GENERATED_BODY()

public:

  UTaggerDelegate();

  void RegisterSpawnHandler(UWorld *World);

  void SetSemanticSegmentationEnabled(bool Enable = true)
  {
    bSemanticSegmentationEnabled = Enable;
  }

  void OnActorSpawned(AActor *Actor);

private:

  FOnActorSpawned::FDelegate ActorSpawnedDelegate;

  bool bSemanticSegmentationEnabled = false;
};
