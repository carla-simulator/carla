// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "StreetMap.h"
#include "SignDataAsset.h"
#include "PoleDataAsset.h"
#include <carla/road/Map.h>
#include "SignGenerationController.generated.h"

class AGeoTrafficSign;

/**
 * Editor tool: stands regional traffic signs (USignDataAsset catalog under SignPackagePath /
 * <style>) at the OSM sign nodes of a UStreetMap (highway=stop / give_way / crossing,
 * traffic_sign=*, maxspeed=*), pushed out of the carriageway with the level's OpenDRIVE and
 * turned to face the oncoming traffic of the nearest lane.
 *
 * Every sign is an AGeoTrafficSign (Carla runtime module, an ATrafficSignBase) carrying the
 * catalog entry's OpenDRIVE type / subtype and the matching ETrafficSignState, so
 * ATrafficLightManager::SpawnSignals adopts it for the signal it stands at instead of
 * spawning the stock blueprint on top.
 *
 * Inputs are resolved for the current level when not set explicitly: the UStreetMap is
 * searched next to the level (/Game/Carla/Maps/Twins/<Level>, the level's own folder) and in
 * the legacy generated-plugin layout (/<Level>/...); the .xodr under
 * Content/Carla/Maps/Twins/<Level>/OpenDrive, Content/Carla/Maps/OpenDrive, then the legacy
 * Plugins/<Level>/Content/Maps/OpenDrive path.
 */
UCLASS()
class CARLATOOLS_API ASignGenerationController : public AActor
{
	GENERATED_BODY()

public:
	ASignGenerationController(const FObjectInitializer& ObjectInitializer);

protected:
	virtual void BeginPlay() override;

public:
	virtual void Tick(float DeltaTime) override;

	UFUNCTION(BlueprintCallable, Category = "EditorUtilityWidget")
	void SignGenerationByPath(FName sign_package_path, FName pole_package_path, ESignStyle sign_style);

	UFUNCTION(Category= "SignGeneration", BlueprintCallable, CallInEditor)
	void SignGenerationForCurrentMap();

	/// OSM data with sign nodes (UStreetMap::GetSigns). Looked up for the current level when null.
	UPROPERTY(Category = "SignGeneration", EditAnywhere, BlueprintReadWrite)
	UStreetMap* StreetMapData;

	/// Absolute path of the level's OpenDRIVE file. Resolved for the current level when empty.
	UPROPERTY(Category = "SignGeneration", EditAnywhere, BlueprintReadWrite)
	FString XodrPath;

	UPROPERTY(Category = "SignGeneration", EditAnywhere, BlueprintReadWrite)
	FName SignPackagePath;

	UPROPERTY(Category = "SignGeneration", EditAnywhere, BlueprintReadWrite)
	FName PolePackagePath;

	UPROPERTY(Category = "SignGeneration", EditAnywhere, BlueprintReadWrite)
	ESignStyle SignStyle;

	UPROPERTY(Category = "SignGeneration", EditAnywhere, BlueprintReadWrite)
	bool bDisplaceSignsToEdge;

	UPROPERTY(Category = "SignGeneration", EditAnywhere, BlueprintReadWrite)
	int MaxDisplacementIterations;

	UPROPERTY(Category = "SignGeneration", EditAnywhere, BlueprintReadWrite)
	float RoadBorderPadding;

	UPROPERTY(Category = "SignGeneration", EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "2.0"))
	float StepPercentOfLaneWidth;

	/// Plate offset from the pole axis in actor space (cm). The catalog plates are thin along
	/// Y and their print faces +Y; with the runtime yaw convention (lane heading + 90) +Y is
	/// the oncoming traffic, so the plate sits on that side of the pole.
	UPROPERTY(Category = "SignGeneration", EditAnywhere, BlueprintReadWrite)
	FVector2D PlateOffsetCm;

	/// Height of the plate centre above the ground (cm) for poles without a "Sign1" socket.
	UPROPERTY(Category = "SignGeneration", EditAnywhere, BlueprintReadWrite)
	float PlateHeightCm;

	/// Plate scale per shape mesh name (the SignShapes meshes are not all drawn at road scale).
	UPROPERTY(Category = "SignGeneration", EditAnywhere, BlueprintReadWrite)
	TMap<FString, float> PlateScaleByMesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<AActor*> GeneratedSigns;

	/// Summary of the last run.
	UPROPERTY(Category = "SignGeneration", VisibleAnywhere, BlueprintReadOnly)
	int32 LastSignNodeCount;

	UPROPERTY(Category = "SignGeneration", VisibleAnywhere, BlueprintReadOnly)
	int32 LastMatchedCount;

	UPROPERTY(Category = "SignGeneration", VisibleAnywhere, BlueprintReadOnly)
	int32 LastSpawnedCount;

	/// Resolve StreetMapData / XodrPath for the current level (no-op for fields already set).
	/// Returns false when either is missing.
	UFUNCTION(BlueprintCallable, Category = "SignGeneration")
	bool ResolveInputs();

private:

	UFUNCTION()
	void GetSteetMapFile();

	FString ResolveXodrPath() const;

	UFUNCTION()
	void GetSignPropertyValue(FStreetMapMisc Sign, FString KeyToFind, FString& Out_KeyName, FString& Out_Value);

	AGeoTrafficSign* SpawnSign(const FVector& Location, USignDataAsset* SignAsset, UPoleDataAsset* PoleAsset,
	                           const FString& Label, const FString& StyleName);

	USignDataAsset* current_data_asset;

	bool has_spawned_sign;
};
