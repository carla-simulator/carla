#pragma once

#include "MultirotorPawn.h"

#include "MultirotorParameters.generated.h"

USTRUCT(BlueprintType)
struct FLYINGVEHICLES_API FMultirotorParameters
{
  GENERATED_BODY()

  UPROPERTY(EditAnywhere, BlueprintReadWrite)
  FString Make;

  UPROPERTY(EditAnywhere, BlueprintReadWrite)
  FString Model;

  UPROPERTY(EditAnywhere, BlueprintReadWrite)
  TSubclassOf<AMultirotorPawn> Class;

  UPROPERTY(EditAnywhere, BlueprintReadWrite)
  int32 NumberOfRotors = 4;

  UPROPERTY(EditAnywhere, BlueprintReadWrite)
  int32 Generation = 0;

  /// (OPTIONAL) Use for custom classification of vehicles.
  UPROPERTY(EditAnywhere, BlueprintReadWrite)
  FString ObjectType;

  UPROPERTY(EditAnywhere, BlueprintReadWrite)
  TArray<FColor> RecommendedColors;
};
