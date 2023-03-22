// Copyright (c) 2019 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "Carla/OpenDrive/OpenDriveActor.h"
#include "Commandlets/Commandlet.h"
#include "Runtime/Engine/Classes/Engine/ObjectLibrary.h"
#include "Runtime/Engine/Classes/Engine/StaticMeshActor.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "PrepareAssetsForCookingCommandlet.generated.h"

// undef this API to avoid conflict with UE 4.26
// (see UE_4.26\Engine\Source\Runtime\Core\Public\Windows\HideWindowsPlatformAtomics.h)
#undef InterlockedCompareExchange
#undef _InterlockedCompareExchange

/// Struct containing Package with @a Name and @a bOnlyPrepareMaps flag used to
/// separate the cooking of maps and props across the different stages (Maps
/// will be imported during make import command and Props will be imported
/// during make package command)
USTRUCT()
struct CARLA_API FPackageParams
{
  GENERATED_USTRUCT_BODY()

  FString Name;

  bool bOnlyPrepareMaps;
};

/// Struct containing map data read from .Package.json file.
USTRUCT()
struct CARLA_API FMapData
{
  GENERATED_USTRUCT_BODY()

  FString Name;

  FString Path;

  bool bUseCarlaMapMaterials;
};

/// Struct containing all assets data read from .Package.json file.
USTRUCT()
struct CARLA_API FAssetsPaths
{
  GENERATED_USTRUCT_BODY()

  TArray<FMapData> MapsPaths;

  TArray<FString> PropsPaths;
};

UCLASS()
class CARLA_API UPrepareAssetsForCookingCommandlet
  : public UCommandlet
{
  GENERATED_BODY()

public:

  /// Default constructor.
  UPrepareAssetsForCookingCommandlet();
#if WITH_EDITORONLY_DATA

  /// Parses the command line parameters provided through @a InParams
  FPackageParams ParseParams(const FString &InParams) const;

  /// Loads a UWorld object contained in Carla BaseMap into @a AssetData data
  /// structure.
  void LoadWorld(FAssetData &AssetData);

  /// Loads a UWorld object contained in Carla BaseTile into @a AssetData data
  /// structure.
  void LoadWorldTile(FAssetData &AssetData);

  void LoadLargeMapWorld(FAssetData &AssetData);

  /// Spawns all the static meshes located in @a AssetsPaths inside the World.
  /// There is an option to use Carla materials by setting @a bUseCarlaMaterials
  /// to true, otherwise it will use RoadRunner materials.
  /// If meshes are been added to a PropsMap, set @a bIsPropMap to true.
  ///
  /// @pre World is expected to be previously loaded
  TArray<AStaticMeshActor *> SpawnMeshesToWorld(
      const TArray<FString> &AssetsPaths,
      bool bUseCarlaMaterials,
      int i = -1,
      int j = -1);

  /// Saves the current World, contained in @a AssetData, into @a DestPath
  /// composed of @a PackageName and with @a WorldName.
  bool SaveWorld(
      FAssetData &AssetData,
      const FString &PackageName,
      const FString &DestPath,
      const FString &WorldName,
      bool bGenerateSpawnPoints = true);

  /// Destroys all the previously spawned actors stored in @a SpawnedActors
  void DestroySpawnedActorsInWorld(TArray<AStaticMeshActor *> &SpawnedActors);

  /// Gets the Path of all the Assets contained in the package to cook with name
  /// @a PackageName
  FAssetsPaths GetAssetsPathFromPackage(const FString &PackageName) const;

  /// Generates the MapPaths file provided @a AssetsPaths and @a PropsMapPath
  void GenerateMapPathsFile(const FAssetsPaths &AssetsPaths, const FString &PropsMapPath);

  /// Generates the PackagePat file that contains the path of a package with @a
  /// PackageName
  void GeneratePackagePathFile(const FString &PackageName);

  /// For each Map data contained in @MapsPaths, it creates a World, spawn its
  /// actors inside the world and saves it in .umap format
  /// in a destination path built from @a PackageName.
  void PrepareMapsForCooking(const FString &PackageName, const TArray<FMapData> &MapsPaths);

  /// For all the props inside @a PropsPaths, it creates a single World, spawn
  /// all the props inside the world and saves it in .umap format
  /// in a destination path built from @a PackageName and @a MapDestPath.
  void PreparePropsForCooking(FString &PackageName, const TArray<FString> &PropsPaths, FString &MapDestPath);

  /// Return if there is any tile between the assets to cook
  bool IsMapInTiles(const TArray<FString> &AssetsPaths);

public:

  /// Main method and entry of the commandlet, taking as input parameters @a
  /// Params.
  virtual int32 Main(const FString &Params) override;

#endif // WITH_EDITORONLY_DATA

private:

  /// Loaded assets from any object library
  UPROPERTY()
  TArray<FAssetData> AssetDatas;

  /// Loaded map content from any object library
  UPROPERTY()
  TArray<FAssetData> MapContents;

  /// Used for loading maps in object library. Loaded Data is stored in
  /// AssetDatas.
  UPROPERTY()
  UObjectLibrary *MapObjectLibrary;

  /// Used for loading assets in object library. Loaded Data is stored in
  /// AssetDatas.
  UPROPERTY()
  UObjectLibrary *AssetsObjectLibrary;

  /// Base map world loaded from Carla Content
  UPROPERTY()
  UWorld *World;

  /// Workaround material for the RoadNode mesh
  UPROPERTY()
  UMaterialInstance *RoadNodeMaterial;

  /// Material to apply to curbs on the road
  UPROPERTY()
  UMaterialInstance *CurbNodeMaterialInstance;

  /// Material to apply to gutters on the road
  UPROPERTY()
  UMaterialInstance *GutterNodeMaterialInstance;

  /// Workaround material for the center lane markings
  UPROPERTY()
  UMaterialInstance *MarkingNodeYellow;

  /// Workaround material for exterior lane markings
  UPROPERTY()
  UMaterialInstance *MarkingNodeWhite;

  /// Workaround material for the TerrainNodes
  UPROPERTY()
  UMaterialInstance *TerrainNodeMaterialInstance;

  /// Workaround material for the SidewalkNodes
  UPROPERTY()
  UMaterialInstance *SidewalkNodeMaterialInstance;

  /// Saves @a Package in .umap format in path @a PackagePath inside Unreal
  /// Content folder
  bool SavePackage(const FString &PackagePath, UPackage *Package) const;

  /// Gets the first .Package.json file found in Unreal Content Directory with
  /// @a PackageName
  FString GetFirstPackagePath(const FString &PackageName) const;

};
