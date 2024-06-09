#pragma once

#include "Carla/Actor/ActorAttribute.h"

#include "GameFramework/Actor.h"

#include "WaypointParameters.generated.h"

USTRUCT(BlueprintType)
struct CARLA_API FWaypointParameters
{
  GENERATED_BODY()

  /// Uniquely identifies the definition (no need to fill it).
  uint32 UId = 0u;

  /// Display ID that identifies the actor.
  UPROPERTY(EditAnywhere, BlueprintReadWrite)
  FString Id;

  /// Class of the actor to be spawned (Optional).
  ///
  /// Note that this parameter is not exposed on the client-side, only used by
  /// the spawner itself.
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
