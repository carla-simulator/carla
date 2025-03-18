// Copyright (c) 2024 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "Carla/Actor/ActorSpawnResult.h"
#include "Carla/Actor/CarlaActorFactory.h"
#include "Carla/Actor/VehicleParameters.h"

#include <util/ue-header-guard-begin.h>
#include "Json.h"
#include "JsonUtilities.h"
#include <util/ue-header-guard-end.h>

#include "VehicleActorFactory.generated.h"

/// Factory in charge of spawning static meshes. This factory is able to spawn
/// any mesh in content.
UCLASS()
class CARLA_API AVehicleActorFactory : public ACarlaActorFactory
{
  GENERATED_BODY()

  /// Retrieve the definitions of the static mesh actor
  virtual TArray<FActorDefinition> GetDefinitions() override;

  virtual FActorSpawnResult SpawnActor(
      const FTransform &SpawnAtTransform,
      const FActorDescription &ActorDescription) override;

public:
  
  UFUNCTION(BlueprintCallable, Category = "VehicleActorFactory")
  static void SaveVehicleParametersArrayToFile(const TArray<FVehicleParameters>& VehicleParamsArray, const FString& FileName);
  UFUNCTION(BlueprintCallable, Category = "VehicleActorFactory")
  static void LoadVehicleParametersArrayFromFile(const FString& FileName, TArray<FVehicleParameters>& OutVehicleParamsArray);

  UFUNCTION(BlueprintImplementableEvent, Category = "VehicleActorFactory")
  bool PostProcessVehicle(AActor* SpawnedActor, const FActorDescription& VehicleParams);
private: 
  static TSharedPtr<FJsonObject> FVehicleParametersToJsonObject(const FVehicleParameters& VehicleParams);
  static FString FVehicleParametersArrayToJson(const TArray<FVehicleParameters>& VehicleParamsArray);
  static bool JsonToFVehicleParameters(const TSharedPtr<FJsonObject> JsonObject, FVehicleParameters& OutVehicleParams);
  static bool JsonToFVehicleParametersArray(const FString& JsonString, TArray<FVehicleParameters>& OutVehicleParamsArray);

protected:

  UPROPERTY(EditAnywhere)
  TArray<FActorDefinition> Definitions;
  UPROPERTY(EditAnywhere, BlueprintReadWrite)
  TArray<FVehicleParameters> VehiclesParams;
  UPROPERTY(EditAnywhere, BlueprintReadWrite)
  TArray<FVehicleParameters> MineVehiclesParams;
};
