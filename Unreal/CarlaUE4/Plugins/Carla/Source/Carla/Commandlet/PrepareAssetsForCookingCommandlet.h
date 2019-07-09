// Copyright (c) 2019 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "Carla/OpenDrive/OpenDriveActor.h"
#include "Commandlets/Commandlet.h"
#include "Containers/Map.h"
#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Misc/PackageName.h"
#include "Runtime/Engine/Classes/Engine/ObjectLibrary.h"
#include "UObject/Package.h"

#if WITH_EDITORONLY_DATA
#include "AssetRegistry/Public/AssetRegistryModule.h"
#include "Developer/AssetTools/Public/AssetToolsModule.h"
#include "Developer/AssetTools/Public/IAssetTools.h"
#endif // WITH_EDITORONLY_DATA
#include "Runtime/Engine/Classes/Engine/StaticMeshActor.h"
#include "PrepareAssetsForCookingCommandlet.generated.h"

/// Struct containing Package Params
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
class UPrepareAssetsForCookingCommandlet
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

  /// Spawns all the static meshes located in @a AssetsPaths inside the World.
  /// There is an option to use Carla materials by setting @a bUseCarlaMaterials
  /// to true, otherwise it will use RoadRunner materials.
  /// If meshes are been added to a PropsMap, set @a bIsPropMap to true.
  ///
  /// @pre World is expected to be previously loaded
  TArray<AStaticMeshActor *> SpawnMeshesToWorld(
      const TArray<FString> &AssetsPaths,
      bool bUseCarlaMaterials,
      bool bIsPropsMap = false);

  /// Saves the current World, contained in @a AssetData, into @a DestPath
  /// composed of @a PackageName and with @a WorldName.
  bool SaveWorld(FAssetData &AssetData, FString &PackageName, FString &DestPath, FString &WorldName);

  /// Destroys all the previously spawned actors stored in @a SpawnedActors
  void DestroySpawnedActorsInWorld(TArray<AStaticMeshActor *> &SpawnedActors);

  /// Gets the Path of all the Assets contained in the package to cook with name
  /// @a PackageName
  FAssetsPaths GetAssetsPathFromPackage(const FString &PackageName) const;

public:

  /// Main method and entry of the commandlet, taking as input parameters @a
  /// Params.
  virtual int32 Main(const FString &Params) override;

#endif // WITH_EDITORONLY_DATA

private:

  /// Loaded assets from any object library
  UPROPERTY()
  TArray<FAssetData> AssetDatas;

  /// Loaded maps from any object library
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

  /// Workaround material for MarkingNodes mesh
  UPROPERTY()
  UMaterial *MarkingNodeMaterial;

  /// Workaround material for the RoadNode mesh
  UPROPERTY()
  UMaterial *RoadNodeMaterial;

  /// Workaround material for the second material for the MarkingNodes
  UPROPERTY()
  UMaterial *MarkingNodeMaterialAux;

  /// Workaround material for the TerrainNodes
  UPROPERTY()
  UMaterial *TerrainNodeMaterial;

  /// Saves @a Package in .umap format in path @a PackagePath inside Unreal
  /// Content folder
  bool SavePackage(const FString &PackagePath, UPackage *Package) const;

  /// Gets the first .Package.json file found in Unreal Content Directory with
  /// @a PackageName
  FString GetFirstPackagePath(const FString &PackageName) const;

};
