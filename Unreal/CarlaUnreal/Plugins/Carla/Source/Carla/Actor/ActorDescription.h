// Copyright (c) 2024 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "Carla/Actor/ActorAttribute.h"

#include <util/ue-header-guard-begin.h>
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include <util/ue-header-guard-end.h>

#include "ActorDescription.generated.h"

/// A description of a Carla Actor with all its variation.
USTRUCT(BlueprintType)
struct FActorDescription
{
  GENERATED_BODY()

  /// UId of the definition in which this description was based.
  uint32 UId = 0u;

  /// Display ID that identifies the actor.
  UPROPERTY(EditAnywhere, BlueprintReadWrite)
  FString Id;

  /// Class of the actor to be spawned.
  UPROPERTY(EditAnywhere, BlueprintReadWrite)
  TSubclassOf<AActor> Class;

  /// User selected variations of the actor. Note that at this point are
  /// represented by non-modifiable attributes.
  ///
  ///   Key: Id of the attribute.
  ///   Value: The attribute.
  UPROPERTY(EditAnywhere, BlueprintReadWrite)
  TMap<FString, FActorAttribute> Variations;
};
