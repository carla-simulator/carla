// Copyright (c) 2019 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "Carla.h"
#include "Carla/Trigger/TriggerFactory.h"

#include "Carla/Game/CarlaGameInstance.h"
#include "Carla/Game/CarlaStatics.h"
#include "Carla/Trigger/FrictionTrigger.h"

// =============================================================================
// -- ATriggerFactory -----------------------------------------------------------
// =============================================================================

TArray<FActorDefinition> ATriggerFactory::GetDefinitions()
{
  FActorDefinition TriggerDefinition = FActorDefinition();
  TriggerDefinition.Class = AFrictionTrigger::StaticClass();

  TArray<FActorDefinition> TriggerDefinitions;

  bool Success;
  UActorBlueprintFunctionLibrary::MakeTriggerDefinition(TEXT("friction"), Success, TriggerDefinition);
  check(Success);
  TriggerDefinitions.Add(TriggerDefinition);

  return TriggerDefinitions;
}

FActorSpawnResult ATriggerFactory::SpawnActor(
    const FTransform &Transform,
    const FActorDescription &Description)
{
  auto *World = GetWorld();
  if (World == nullptr)
  {
    UE_LOG(LogCarla, Error, TEXT("ATriggerFactory: cannot spawn trigger into an empty world."));
    return {};
  }

  UCarlaGameInstance *GameInstance = UCarlaStatics::GetGameInstance(World);
  if (GameInstance == nullptr)
  {
    UE_LOG(LogCarla, Error, TEXT("ATriggerFactory:: cannot spawn trigger, incompatible game instance."));
    return {};
  }

  auto *Trigger = World->SpawnActorDeferred<AFrictionTrigger>(
      Description.Class,
      Transform,
      this,
      nullptr,
      ESpawnActorCollisionHandlingMethod::AlwaysSpawn);

  if (Trigger == nullptr)
  {
    UE_LOG(LogCarla, Error, TEXT("ATriggerFactory:: spawn trigger failed."));
  }
  else
  {
    // Retrieve Episode
    auto *Episode = GameInstance->GetCarlaEpisode();
    check(Episode != nullptr);
    Trigger->SetEpisode(*Episode);

    // Retrieve Friction
    float Friction = UActorBlueprintFunctionLibrary::RetrieveActorAttributeToFloat("friction",
        Description.Variations,
        3.5f);
    Trigger->SetFriction(Friction);

    // Retrieve Extent
    FVector Extent {100.0f, 100.0f, 100.0f};

    Extent.X = UActorBlueprintFunctionLibrary::RetrieveActorAttributeToFloat("extent_x",
        Description.Variations,
        Extent.X);
    Extent.Y = UActorBlueprintFunctionLibrary::RetrieveActorAttributeToFloat("extent_y",
        Description.Variations,
        Extent.Y);
    Extent.Z = UActorBlueprintFunctionLibrary::RetrieveActorAttributeToFloat("extent_z",
        Description.Variations,
        Extent.Z);

    Trigger->SetBoxExtent(Extent);
  }
  UGameplayStatics::FinishSpawningActor(Trigger, Transform);
  return FActorSpawnResult{Trigger};
}
