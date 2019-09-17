// Copyright (c) 2019 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "Carla/OpenDrive/OpenDriveActor.h"
#include "Commandlets/Commandlet.h"
#include "Runtime/Engine/Classes/Engine/ObjectLibrary.h"

#if WITH_EDITORONLY_DATA
#include "AssetRegistry/Public/AssetRegistryModule.h"
#include "Developer/AssetTools/Public/AssetToolsModule.h"
#endif // WITH_EDITORONLY_DATA
#include "MoveAssetsCommandlet.generated.h"

/// Struct containing Package Params, used for storing the parsed arguments when
/// invoking this commandlet
USTRUCT()
struct CARLA_API FMovePackageParams
{
  GENERATED_USTRUCT_BODY()

  FString Name;

  TArray<FString> MapNames;
};

UCLASS()
class UMoveAssetsCommandlet : public UCommandlet
{
  GENERATED_BODY()

public:

  /// Default constructor.
  UMoveAssetsCommandlet();
#if WITH_EDITORONLY_DATA

  /// Parses the command line parameters provided through @a InParams The
  /// arguments to parse are the package name and a list of map names
  /// concatenated in a string.
  FMovePackageParams ParseParams(const FString &InParams) const;

  /// Moves all the assets contained in a map from @a SrcPath to @a DestPath
  void MoveAssetsFromMapForSemanticSegmentation(const FString &PackageName, const FString &MapName);

  /// Moves the meshes of all maps listed in a @PackageParams
  void MoveAssets(const FMovePackageParams &PackageParams);

public:

  /// Main method and entry of the commandlet, taking as input parameters @a
  /// Params.
  virtual int32 Main(const FString &Params) override;

#endif // WITH_EDITORONLY_DATA

private:

  /// The following data structures are declared as class members and with
  /// UPROPERTY macro to avoid UE4 to garbage collect them.

  /// Loaded assets from any object library
  UPROPERTY()
  TArray<FAssetData> AssetDatas;

  /// Loaded maps from any object library
  UPROPERTY()
  TArray<FAssetData> MapContents;

  /// Used for loading assets in object library. Loaded Data is stored in
  /// AssetDatas.
  UPROPERTY()
  UObjectLibrary *AssetsObjectLibrary;
};
