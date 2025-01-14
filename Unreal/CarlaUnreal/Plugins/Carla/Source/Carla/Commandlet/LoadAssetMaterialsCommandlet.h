// Copyright (c) 2024 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "PrepareAssetsForCookingCommandlet.h"
#include "Util/RoadPainterWrapper.h"
#include "Carla/OpenDrive/OpenDrive.h"

#include <util/disable-ue4-macros.h>
#include <carla/opendrive/OpenDriveParser.h>
#include <carla/geom/Math.h>
#include <carla/rpc/String.h>
#include <util/enable-ue4-macros.h>

#include <util/ue-header-guard-begin.h>
#include "Engine/ObjectLibrary.h"
#include <util/ue-header-guard-end.h>

#include "LoadAssetMaterialsCommandlet.generated.h"

USTRUCT()
struct CARLA_API FLargeMapTileData
{
  GENERATED_USTRUCT_BODY()

  UPROPERTY()
  float FirstTileCenterX = 0.0F;

  UPROPERTY()
  float FirstTileCenterY = 0.0F;

  UPROPERTY()
  float Size = 0.0F;
};

struct FDecalsProperties;

UCLASS()
class CARLA_API ULoadAssetMaterialsCommandlet
  : public UCommandlet
{
  GENERATED_BODY()

public:

  /// Default constructor.
  ULoadAssetMaterialsCommandlet();

#if WITH_EDITORONLY_DATA

  /// Gets the Path of all the Assets contained in the package to cook with name
  /// @a PackageName
  FAssetsPaths GetAssetsPathFromPackage(const FString &PackageName) const;

  /// Parses the command line parameters provided through @a InParams
  FPackageParams ParseParams(const FString& InParams) const;

  void LoadAssetsMaterials(const FString &PackageName, const TArray<FMapData> &MapsPaths);

  void ApplyRoadPainterMaterials(const FString &LoadedMapName, const FString &PackageName, bool IsInTiles = false);

  FDecalsProperties ReadDecalsConfigurationFile(const FString &PackageName);

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

  UPROPERTY()
  UWorld *NewWorldToLoad;

  /// Used for loading maps in object library. Loaded Data is stored in
  /// AssetDatas.
  UPROPERTY()
  UObjectLibrary *MapObjectLibrary;

  /// Subclass for acquiring the RoadPainter blueprint
  UPROPERTY()
  TSubclassOf<ARoadPainterWrapper> RoadPainterSubclass;

  /// Dictionary for translating the JSON file "decal_names" array
  /// to already loaded Material Instances, which are used to apply on the roads
  UPROPERTY()
  TMap<FString, FString> DecalNamesMap;

  //UPROPERTY()
  //TSet<FVector> WaypointsUsedForDecals;

  std::optional<carla::road::Map> XODRMap;

  /// Gets the first .Package.json file found in Unreal Content Directory with
  /// @a PackageName
  FString GetFirstPackagePath(const FString &PackageName) const;

  FLargeMapTileData TileData;

  FDecalsProperties DecalsProperties;

  bool FilledData;

  bool ReadConfigFile;
};
