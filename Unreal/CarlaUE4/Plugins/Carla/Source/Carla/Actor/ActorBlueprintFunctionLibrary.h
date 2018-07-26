// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "Carla/Actor/ActorDefinition.h"
#include "Carla/Vehicle/CarlaWheeledVehicle.h"

#include "Kismet/BlueprintFunctionLibrary.h"

#include "ActorBlueprintFunctionLibrary.generated.h"

USTRUCT(BlueprintType)
struct CARLA_API FVehicleParameters
{
  GENERATED_BODY()

  UPROPERTY(EditAnywhere, BlueprintReadWrite)
  FString Make;

  UPROPERTY(EditAnywhere, BlueprintReadWrite)
  FString Model;

  UPROPERTY(EditAnywhere, BlueprintReadWrite)
  TSubclassOf<ACarlaWheeledVehicle> Class;

  UPROPERTY(EditAnywhere, BlueprintReadWrite)
  int32 NumberOfWheels = 4;

  UPROPERTY(EditAnywhere, BlueprintReadWrite)
  TArray<FColor> RecommendedColors;
};

UCLASS()
class UActorBlueprintFunctionLibrary : public UBlueprintFunctionLibrary
{
  GENERATED_BODY()

public:

  /// Return whether the actor definition is valid. Prints all the errors found.
  UFUNCTION(Category = "Carla Actor", BlueprintCallable)
  static bool CheckActorDefinition(const FActorDefinition &ActorDefinitions);

  /// Return whether the list of actor definitions is valid. Prints all the
  /// errors found.
  UFUNCTION(Category = "Carla Actor", BlueprintCallable)
  static bool CheckActorDefinitions(const TArray<FActorDefinition> &ActorDefinitions);

  UFUNCTION(Category = "Carla Actor", BlueprintCallable)
  static void MakeVehicleDefinition(
      const FVehicleParameters &Parameters,
      bool &Success,
      FActorDefinition &Definition);

  UFUNCTION(Category = "Carla Actor", BlueprintCallable)
  static void MakeVehicleDefinitions(
      const TArray<FVehicleParameters> &ParameterArray,
      TArray<FActorDefinition> &Definitions);
};

