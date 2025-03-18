// Copyright (c) 2025 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "Carla/Actor/ActorSpawnResult.h"
#include "Carla/Actor/CarlaActorFactory.h"
#include "Carla/Actor/BlueprintParameters.h"

#include <util/ue-header-guard-begin.h>
#include "Json.h"
#include "JsonUtilities.h"
#include <util/ue-header-guard-end.h>

#include "BlueprintActorFactory.generated.h"

UCLASS()
class CARLA_API ABlueprintActorFactory : public ACarlaActorFactory
{
	GENERATED_BODY()

	/// Retrieve the definitions of the static mesh actor
	virtual TArray<FActorDefinition> GetDefinitions() override;

	virtual FActorSpawnResult SpawnActor(
		const FTransform &SpawnAtTransform,
		const FActorDescription &ActorDescription) override;

public:
	UFUNCTION(BlueprintCallable, Category = "BlueprintActorFactory")
	static void SaveBlueprintParametersArrayToFile(const TArray<FBlueprintParameters> &BlueprintParamsArray, const FString &FileName);
	UFUNCTION(BlueprintCallable, Category = "BlueprintActorFactory")
	static void LoadBlueprintParametersArrayFromFile(const FString &FileName, TArray<FBlueprintParameters> &OutBlueprintParamsArray);

	UFUNCTION(BlueprintImplementableEvent, Category = "BlueprintActorFactory")
  	bool PostProcessBlueprint(AActor* SpawnedActor, const FActorDescription& BlueprintParams);

private:
	static TSharedPtr<FJsonObject> FBlueprintParametersToJsonObject(const FBlueprintParameters& BlueprintParams);
	static FString FBlueprintParametersArrayToJson(const TArray<FBlueprintParameters>& BlueprintParamsArray);
	static bool JsonToFBlueprintParameters(const TSharedPtr<FJsonObject> JsonObject, FBlueprintParameters& OutBlueprintParams);
	static bool JsonToFBlueprintParametersArray(const FString& JsonString, TArray<FBlueprintParameters>& OutBlueprintParamsArray);

protected:
	UPROPERTY(EditAnywhere)
	TArray<FActorDefinition> Definitions;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FBlueprintParameters> BlueprintsParams;
};