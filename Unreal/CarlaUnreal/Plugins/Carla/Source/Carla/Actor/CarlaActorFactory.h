// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "Carla/Actor/ActorDefinition.h"
#include "Carla/Actor/ActorDescription.h"
#include "Carla/Actor/ActorSpawnResult.h"

#include <util/ue-header-guard-begin.h>
#include "Containers/Array.h"
#include "GameFramework/Actor.h"
#include <util/ue-header-guard-end.h>

#include "CarlaActorFactory.generated.h"

/// Base class for Carla actor factories.
UCLASS(Abstract)
class CARLA_API ACarlaActorFactory : public AActor
{
  GENERATED_BODY()

public:

  ACarlaActorFactory(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
  {
    PrimaryActorTick.bCanEverTick = false;
  }

  /// Retrieve the list of actor definitions that this class is able to spawn.
  virtual TArray<FActorDefinition> GetDefinitions() {
    unimplemented();
    return {};
  }

  /// Spawn an actor based on @a ActorDescription and @a Transform.
  ///
  /// @pre ActorDescription is expected to be derived from one of the
  /// definitions retrieved with GetDefinitions.
  virtual FActorSpawnResult SpawnActor(
      const FTransform &SpawnAtTransform,
      const FActorDescription &ActorDescription) {
    unimplemented();
    return {};
  }

  /// Drop every cached definition, parameter and loaded asset that refers to
  /// content under @a MountPoint ("/<Pack>/"), so a content pack can be
  /// unmounted; the dropped blueprints are gone until the next episode.
  virtual void ReleaseContentPack(const FString &MountPoint) {}

  /// True when the definition's class or any attribute/variation value
  /// points at content under @a MountPoint.
  static bool DefinitionReferencesPath(const FActorDefinition &Definition, const FString &MountPoint)
  {
    if (Definition.Class.Get() != nullptr && Definition.Class.Get()->GetPathName().StartsWith(MountPoint))
    {
      return true;
    }
    for (const FActorAttribute &Attribute : Definition.Attributes)
    {
      if (Attribute.Value.StartsWith(MountPoint))
      {
        return true;
      }
    }
    for (const FActorVariation &Variation : Definition.Variations)
    {
      for (const FString &Value : Variation.RecommendedValues)
      {
        if (Value.StartsWith(MountPoint))
        {
          return true;
        }
      }
    }
    return false;
  }
};
