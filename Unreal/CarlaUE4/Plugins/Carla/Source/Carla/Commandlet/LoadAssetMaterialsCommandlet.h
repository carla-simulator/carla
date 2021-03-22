// Copyright (c) 2019 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once
#include "Commandlets/Commandlet.h"
#include "Util/RoadPainterWrapper.h"
#include "Runtime/Engine/Classes/Engine/ObjectLibrary.h"
#include "LoadAssetMaterialsCommandlet.generated.h"

UCLASS()
class CARLA_API ULoadAssetMaterialsCommandlet
	: public UCommandlet
{
	GENERATED_BODY()

public:

	/// Default constructor.
	ULoadAssetMaterialsCommandlet();
	
#if WITH_EDITORONLY_DATA

	/// Parses the command line parameters provided through @a InParams
	//FPackageParams ParseParams(const FString &InParams) const;

	/// Loads a UWorld object contained in Carla BaseMap into @a AssetData data
	/// structure.
	//void LoadWorld(FAssetData &AssetData);

	/// Spawns all the static meshes located in @a AssetsPaths inside the World.
	/// There is an option to use Carla materials by setting @a bUseCarlaMaterials
	/// to true, otherwise it will use RoadRunner materials.
	/// If meshes are been added to a PropsMap, set @a bIsPropMap to true.
	///
	/// @pre World is expected to be previously loaded
	//TArray<AStaticMeshActor *> SpawnMeshesToWorld(
	//	const TArray<FString> &AssetsPaths,
	//	bool bUseCarlaMaterials);

	/// Saves the current World, contained in @a AssetData, into @a DestPath
	/// composed of @a PackageName and with @a WorldName.
	//bool SaveWorld(
	//	FAssetData &AssetData,
	//	const FString &PackageName,
	//	const FString &DestPath,
	//	const FString &WorldName);

	/// Destroys all the previously spawned actors stored in @a SpawnedActors
	//void DestroySpawnedActorsInWorld(TArray<AStaticMeshActor *> &SpawnedActors);

	/// Gets the Path of all the Assets contained in the package to cook with name
	/// @a PackageName
	//FAssetsPaths GetAssetsPathFromPackage(const FString &PackageName) const;

	/// Generates the MapPaths file provided @a AssetsPaths and @a PropsMapPath
	//void GenerateMapPathsFile(const FAssetsPaths &AssetsPaths, const FString &PropsMapPath);

	/// Generates the PackagePat file that contains the path of a package with @a
	/// PackageName
	//void GeneratePackagePathFile(const FString &PackageName);

	/// For each Map data contained in @MapsPaths, it creates a World, spawn its
	/// actors inside the world and saves it in .umap format
	/// in a destination path built from @a PackageName.
	//void PrepareMapsForCooking(const FString &PackageName, const TArray<FMapData> &MapsPaths);

	/// For all the props inside @a PropsPaths, it creates a single World, spawn
	/// all the props inside the world and saves it in .umap format
	/// in a destination path built from @a PackageName and @a MapDestPath.
	//void PreparePropsForCooking(FString &PackageName, const TArray<FString> &PropsPaths, FString &MapDestPath);

	void ApplyRoadPainterMaterials();
	
	void LoadImportedMapWorld(FAssetData &AssetData);

public:
	
	/// Main method and entry of the commandlet, taking as input parameters @a
	/// Params.
	virtual int32 Main(const FString &Params) override;

#endif // WITH_EDITORONLY_DATA
	
private:

	/// Loaded assets from any object library
	UPROPERTY()
		TArray<FAssetData> AssetDatas;

	UPROPERTY()
		UWorld *World;
	
	/// Used for loading maps in object library. Loaded Data is stored in
	/// AssetDatas.
	UPROPERTY()
		UObjectLibrary *MapObjectLibrary;

	/// Texture used for painting roads with road painter
	UPROPERTY()
		UTextureRenderTarget2D *RoadPainterTexture;

	/// Subclass for acquiring the RoadPainter blueprint
	UPROPERTY()
		TSubclassOf<ARoadPainterWrapper> RoadPainterSubclass;

};