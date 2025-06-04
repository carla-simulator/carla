// Copyright (c) 2025 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "Carla/Actor/ActorSpawnResult.h"
#include "Carla/Actor/CarlaActorFactory.h"
#include "Carla/Actor/PedestrianParameters.h"

#include <util/ue-header-guard-begin.h>
#include "Json.h"
#include "JsonUtilities.h"
#include <util/ue-header-guard-end.h>

#include "WalkerActorFactory.generated.h"

UCLASS()
class CARLA_API AWalkerActorFactory : public ACarlaActorFactory
{
	GENERATED_BODY()

	/// Retrieve the definitions of the static mesh actor
	virtual TArray<FActorDefinition> GetDefinitions() override;

	virtual FActorSpawnResult SpawnActor(
		const FTransform &SpawnAtTransform,
		const FActorDescription &ActorDescription) override;

public:
	UFUNCTION(BlueprintCallable, Category = "WalkerActorFactory")
	static void SaveWalkerParametersArrayToFile(const TArray<FPedestrianParameters> &WalkerParamsArray, const FString &FileName);
	UFUNCTION(BlueprintCallable, Category = "WalkerActorFactory")
	static void LoadWalkerParametersArrayFromFile(const FString &FileName, TArray<FPedestrianParameters> &OutWalkerParamsArray);

	UFUNCTION(BlueprintImplementableEvent, Category = "WalkerActorFactory")
  	bool PostProcessWalker(AActor* SpawnedActor, const FActorDescription& WalkerParams);

private:
	static TSharedPtr<FJsonObject> FWalkerParametersToJsonObject(const FPedestrianParameters& WalkerParams);
	static FString FWalkerParametersArrayToJson(const TArray<FPedestrianParameters>& WalkerParamsArray);
	static bool JsonToFWalkerParameters(const TSharedPtr<FJsonObject> JsonObject, FPedestrianParameters& OutWalkerParams);
	static bool JsonToFWalkerParametersArray(const FString& JsonString, TArray<FPedestrianParameters>& OutWalkerParamsArray);

protected:
	UPROPERTY(EditAnywhere)
	TArray<FActorDefinition> Definitions;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FPedestrianParameters> WalkersParams;
};