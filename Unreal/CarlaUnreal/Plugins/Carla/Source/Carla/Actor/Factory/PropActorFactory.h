// Copyright (c) 2025 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "Carla/Actor/ActorSpawnResult.h"
#include "Carla/Actor/CarlaActorFactory.h"
#include "Carla/Actor/PropParameters.h"

#include <util/ue-header-guard-begin.h>
#include "Json.h"
#include "JsonUtilities.h"
#include <util/ue-header-guard-end.h>

#include "PropActorFactory.generated.h"

UCLASS()
class CARLA_API APropActorFactory : public ACarlaActorFactory
{
	GENERATED_BODY()

	/// Retrieve the definitions of the static mesh actor
	virtual TArray<FActorDefinition> GetDefinitions() override;

	virtual FActorSpawnResult SpawnActor(
		const FTransform &SpawnAtTransform,
		const FActorDescription &ActorDescription) override;

public:
	UFUNCTION(BlueprintCallable, Category = "PropActorFactory")
	static void SavePropParametersArrayToFile(const TArray<FPropParameters> &PropParamsArray, const FString &FileName);
	UFUNCTION(BlueprintCallable, Category = "PropActorFactory")
	static void LoadPropParametersArrayFromFile(const FString &FileName, TArray<FPropParameters> &OutPropParamsArray);

	UFUNCTION(BlueprintImplementableEvent, Category = "PropActorFactory")
  	bool PostProcessProp(AActor* SpawnedActor, const FActorDescription& PropParams);

private:
	static TSharedPtr<FJsonObject> FPropParametersToJsonObject(const FPropParameters& PropParams);
	static FString FPropParametersArrayToJson(const TArray<FPropParameters>& PropParamsArray);
	static bool JsonToFPropParameters(const TSharedPtr<FJsonObject> JsonObject, FPropParameters& OutPropParams);
	static bool JsonToFPropParametersArray(const FString& JsonString, TArray<FPropParameters>& OutPropParamsArray);

protected:
	UPROPERTY(EditAnywhere)
	TArray<FActorDefinition> Definitions;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FPropParameters> PropsParams;
};