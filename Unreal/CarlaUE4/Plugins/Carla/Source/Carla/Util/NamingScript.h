// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"
#include "UObject/Object.h"
#include "UObject/ScriptMacros.h"
#include "Modules/ModuleManager.h"
#include "AssetRegistryModule.h"
#include "Logging/MessageLog.h"
#include "NamingScript.generated.h"


UENUM()
enum class AssetTypes : uint8{
	
	//Textures
	Texture		UMETA(DisplayName = "Texture"),
	TextureCube UMETA(DisplayName = "TextureCube"),
	//Materials
	Material	UMETA(DisplayName = "Material"),
	MaterialFunction	UMETA(DisplayName = "Material Function"),
	MaterialInstance	UMETA(DisplayName = "Material Instance"),
	MaterialParameterCollection		UMETA(DisplayName = "Material Parameter Collection"),
	//Meshes
	SkeletalMesh	UMETA(DisplayName = "Skeletal Mesh"),
	StaticMesh		UMETA(DisplayName = "Static Mesh"),
	//Animations
	AnimationBlueprint UMETA(DisplayName = "Animation Blueprint"),
	AnimationSequence	UMETA(DisplayName = "Animation Montage"),
	//Particles
	ParticleSystem	UMETA(DisplayName = "Particles System")
};

USTRUCT(BlueprintType)
struct FFilterAssetsScan {

	GENERATED_USTRUCT_BODY()

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "FFilterAssetsScan")
		bool bScanForMaterials = false;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "FFilterAssetsScan")
		bool bScanForTextures = false;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "FFilterAssetsScan")
		bool bScanForMeshes = false;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "FFilterAssetsScan")
		bool bScanForAnimations = false;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "FFilterAssetsScan")
		bool bScanForParticles = false;
};

UCLASS(Blueprintable, meta = (ShowWorldContextPin))
class CARLA_API UNamingScript : public UObject
{
	GENERATED_BODY()

public:
	
	UNamingScript(const FObjectInitializer& ObjectInitializer);

	//*Function used to check the names of the assets at a given directory, filtered by FFilterAssetsScan. This process will take longer as more filters are included.
	//*IMPORTANT : The path to the directory should be written as, for example "Game/Carla/Static/TrafficLights"
	//@param	DirectoryToScan		The string path where to scan the assets directory.
	//@param	AssetsToScan		Filter used to indicate which types of assets should be checked. Less filters will make the process faster.
	UFUNCTION(BlueprintCallable, Category = "Naming Script")
	void CheckAssetsNaming(FString DirectoryToScan, FFilterAssetsScan AssetsToScan);

private:

	bool LoadDataFromPath(const FString &Path);
	void InitializeContainers();
	void FlushData();
	void IdentifyAndCheckAssets();
	void ValidateAssetName(const FString &AssetPath, const FString &AssetName, const AssetTypes &AssetType, const FString &ValidAssetName, const FString &ValidPrefix, bool bUsesSuffix = false);

	TArray<FString> AssetFilenames;
	TArray<FAssetData> AssetData;
	TArray<TPair<FText, EMessageSeverity::Type>> MessageLog;
	TSet<FString> TextureSuffixes;
	FString TexSuffMsg;
	FFilterAssetsScan AssetFilters;
};
