// Copyright (c) 2024 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "Carla/AI/AIControllerFactory.h"
#include "Carla.h"

#include "Carla/AI/WalkerAIController.h"
#include "Carla/Actor/ActorBlueprintFunctionLibrary.h"

TArray<FActorDefinition> AAIControllerFactory::GetDefinitions()
{
  using ABFL = UActorBlueprintFunctionLibrary;
  auto WalkerController = ABFL::MakeGenericDefinition(
      TEXT("controller"),
      TEXT("ai"),
      TEXT("walker"));
  WalkerController.Class = AWalkerAIController::StaticClass();
  return { WalkerController };
}

FActorSpawnResult AAIControllerFactory::SpawnActor(
    const FTransform &Transform,
    const FActorDescription &Description)
{
  auto *World = GetWorld();
  if (World == nullptr)
  {
    UE_LOG(LogCarla, Error, TEXT("AAIControllerFactory: cannot spawn controller into an empty world."));
    return {};
  }

  FActorSpawnParameters SpawnParameters;
  SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
  auto *Controller = World->SpawnActor<AActor>(Description.Class, Transform, SpawnParameters);

  if (Controller == nullptr)
  {
    UE_LOG(LogCarla, Error, TEXT("AAIControllerFactory: spawn controller failed."));
  }
  return FActorSpawnResult{Controller};
}
