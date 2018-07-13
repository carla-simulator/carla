// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "Carla/Actor/ActorAttribute.h"

#include "GameFramework/Actor.h"

#include "ActorDefinition.generated.h"

/// A definition of a Carla Actor with all the variation and attributes.
USTRUCT(BlueprintType)
struct FActorDefinition
{
  GENERATED_BODY()

  /// Display ID that identifies the actor.
  UPROPERTY(EditAnywhere, BlueprintReadWrite)
  FString Id;

  /// AActor class of the actor to be spawned.
  UPROPERTY(EditAnywhere, BlueprintReadWrite)
  TSubclassOf<AActor> Class;

  /// A list of comma-separated tags.
  UPROPERTY(EditAnywhere, BlueprintReadWrite)
  FString Tags;

  /// Variations represent variables the user can modify to generate variations
  /// of the actor.
  UPROPERTY(EditAnywhere, BlueprintReadWrite)
  TArray<FActorVariation> Variations;

  /// Attributes represent non-modifiable properties of the actor that might
  /// help the user identifying and filtering actors.
  UPROPERTY(EditAnywhere, BlueprintReadWrite)
  TArray<FActorAttribute> Attributes;
};
