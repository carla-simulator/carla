// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.
#pragma once

#include "Commandlets/Commandlet.h"
#include "CoreMinimal.h"
#include "Engine/ObjectLibrary.h"
#include "Engine/World.h"
#if WITH_EDITORONLY_DATA
#include <AssetRegistry/Public/AssetRegistryModule.h>
#include <Developer/AssetTools/Public/IAssetTools.h>
#include <Developer/AssetTools/Public/AssetToolsModule.h>
#endif // WITH_EDITORONLY_DATA
#include "FBXExporterPreparatorCommandlet.generated.h"

UCLASS()
class UFBXExporterPreparatorCommandlet
  : public UCommandlet
{
  GENERATED_BODY()

public:

  /// Default constructor.
  UFBXExporterPreparatorCommandlet();
#if WITH_EDITORONLY_DATA


  /// Parses the command line parameters
  /// @param InParams - The parameters to parse
  bool ParseParams(const FString &InParams);

  /// Move meshes from one folder to another. It only works with StaticMeshes
  /// @param SrcPath - Source folder from which the StaticMeshes will be obtained
  /// @param DestPath - Posible folder in which the Meshes will be ordered
  /// following
  /// the semantic segmentation. It follows ROAD_INDEX, MARKINGLINE_INDEX,
  /// TERRAIN_INDEX
  /// for the position in which each path will be stored.
  void MoveMeshes(const FString &SrcPath, const TArray<FString> &DestPath);

  /// Loads a UWorld object from a given path into a asset data structure.
  /// @param SrcPath - Folder in which the world is located.
  /// @param AssetData - Structure in which the loaded UWorld will be saved.
  void LoadWorld(FAssetData &AssetData);

  /// Add StaticMeshes from a folder into the World loaded as UPROPERTY.
  /// @param SrcPath - Array containing the folders from which the Assets will be
  /// loaded
  /// @param bMaterialWorkaround - Flag that will trigger a change in the
  /// materials to fix a known bug
  /// in RoadRunner.
  void AddMeshesToWorld();

  /// Save a given Asset containing a World into a given path with a given name.
  /// @param AssetData - Contains all the info about the World to be saved
  /// @param DestPath - Path in which the asset will be saved.
  /// @param WorldName - Name for the saved world.
  bool SaveWorld(FAssetData &AssetData);

public:

  /// Main method and entry of the commandlet
  /// @param Params - Parameters of the commandlet.
  virtual int32 Main(const FString &Params) override;

#endif // WITH_EDITORONLY_DATA

private:

  UPROPERTY()
  bool bOverrideMaterials;

  // UProperties are necesary or else the GC will eat everything up
  UPROPERTY()
  FString MapName;

  UPROPERTY()
  UObjectLibrary *MapObjectLibrary;

  UPROPERTY()
  TArray<FAssetData> AssetDatas;

  UPROPERTY()
  UWorld *World;

  UPROPERTY()
  UObjectLibrary *AssetsObjectLibrary;

  UPROPERTY()
  TArray<FAssetData> MapContents;

  /** All commandline tokens */
  TArray<FString> Tokens;
  /** All commandline switches */
  TArray<FString> Switches;
  /** All commandline params */
  FString Params;
};
