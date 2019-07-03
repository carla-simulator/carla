// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.
#pragma once

#include "Commandlets/Commandlet.h"
#include <Engine/World.h>
#include <UObject/Package.h>
#include <Misc/PackageName.h>
#include "CoreMinimal.h"
#include <Runtime/Engine/Classes/Engine/ObjectLibrary.h>
#include "Carla/OpenDrive/OpenDriveActor.h"
#include "Containers/Map.h"

#if WITH_EDITORONLY_DATA
#include <Developer/AssetTools/Public/IAssetTools.h>
#include <Developer/AssetTools/Public/AssetToolsModule.h>
#include <AssetRegistry/Public/AssetRegistryModule.h>
#endif // WITH_EDITORONLY_DATA
#include <Runtime/Engine/Classes/Engine/StaticMeshActor.h>
#include "CookAssetsCommandlet.generated.h"

USTRUCT()
struct CARLA_API FPackageParams
{
  GENERATED_USTRUCT_BODY()

  FString Name;

  bool bUseCarlaMapMaterials;
};

USTRUCT()
struct CARLA_API FMapData
{
  GENERATED_USTRUCT_BODY()

  FString Name;

  FString Path;

  bool bUseCarlaMapMaterials;
};

USTRUCT()
struct CARLA_API FAssetsPaths
{
  GENERATED_USTRUCT_BODY()

  TArray<FMapData> MapsPaths;

  TArray<FString> PropsPaths;
};

UCLASS()
class UCookAssetsCommandlet
  : public UCommandlet
{
  GENERATED_BODY()

public:

  /** Default constructor. */
  UCookAssetsCommandlet();
#if WITH_EDITORONLY_DATA

  /**
   * Parses the command line parameters
   * @param InParams - The parameters to parse
   */
  FPackageParams ParseParams(const FString &InParams) const;

  /**
   * Loads a UWorld object from a given path into a asset data structure.
   * @param AssetData - Structure in which the loaded UWorld will be saved.
   */
  void LoadWorld(FAssetData &AssetData);

  /**
   * Add StaticMeshes from a folder into the World loaded as UPROPERTY.
   * @param SrcPath - Array containing the folders from which the Assets will be
   * loaded
   * @param bMaterialWorkaround - Flag that will trigger a change in the
   * materials to fix a known bug
   * in RoadRunner.
   */
  TArray<AStaticMeshActor *> AddMeshesToWorld(const TArray<FString> &AssetsPaths, bool bUseCarlaMaterials);

  /**
   * Save a given Asset containing a World into a given path with a given name.
   * @param AssetData - Contains all the info about the World to be saved
   * @param DestPath - Path in which the asset will be saved.
   * @param WorldName - Name for the saved world.
   */
  bool SaveWorld(FAssetData &AssetData, FString &PackageName, FString &DestPath, FString &WorldName);

  void DestroyWorldSpawnedActors(TArray<AStaticMeshActor *> &SpawnedActors);

  /**
   * Get Path of all the Assets contained in the package to cook
   * @param PackageName - The name of the package to cook
   */
  FAssetsPaths GetAssetsPathFromPackage(const FString &PackageName) const;

public:

  /**
   * Main method and entry of the commandlet
   * @param Params - Parameters of the commandlet.
   */
  virtual int32 Main(const FString &Params) override;

#endif // WITH_EDITORONLY_DATA

private:

  UPROPERTY()
  UObjectLibrary *MapObjectLibrary;

  UPROPERTY()
  UObjectLibrary *AssetsObjectLibrary;

  UPROPERTY()
  TArray<FAssetData> AssetDatas;

  UPROPERTY()
  UWorld *World;

  UPROPERTY()
  TArray<FAssetData> MapContents;

  /** Materials for the workaround */
  /**
   * Workaround material for MarkingNodes mesh
   */
  UMaterial *MarkingNodeMaterial;

  /**
   * Workaround material for the RoadNode mesh
   */
  UMaterial *RoadNodeMaterial;

  /**
   * Workaround material for the second material for the MarkingNodes
   */
  UMaterial *MarkingNodeMaterialAux;

  /**
   * Workaround material for the TerrainNodes
   */
  UMaterial *TerrainNodeMaterial;

  /**
   * Saves the package
   */
  bool SavePackage(const FString &PackagePath, UPackage *Package) const;

};
