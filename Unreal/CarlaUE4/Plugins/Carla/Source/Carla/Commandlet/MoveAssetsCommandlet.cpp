// Copyright (c) 2019 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "MoveAssetsCommandlet.h"

#include "Editor/ContentBrowser/Private/ContentBrowserUtils.h"
#include "GameFramework/WorldSettings.h"

#include "HAL/PlatformFile.h"
#include "HAL/PlatformFilemanager.h"

#include "UObject/MetaData.h"

UMoveAssetsCommandlet::UMoveAssetsCommandlet()
{
  IsClient = false;
  IsEditor = true;
  IsServer = false;
  LogToConsole = true;
}
#if WITH_EDITORONLY_DATA

// NOTE: Assets imported from a map FBX will be classified for semantic
// segmentation as ROAD, ROADLINES AND TERRAIN based on the asset name
// defined in RoadRunner. These tags will be used for moving the meshes
// and for specifying the path to these meshes when spawning them in a world.
namespace SSTags {
  // Carla Semantic Segmentation Folder Tags
  static const FString ROAD       = TEXT("Roads");
  static const FString ROADLINES  = TEXT("RoadLines");
  static const FString TERRAIN    = TEXT("Terrain");

  // RoadRunner Tags
  static const FString R_ROAD     = TEXT("RoadNode");
  static const FString R_TERRAIN  = TEXT("Terrain");
  static const FString R_MARKING  = TEXT("MarkingNode");
}

FMovePackageParams UMoveAssetsCommandlet::ParseParams(const FString &InParams) const
{
  TArray<FString> Tokens;
  TArray<FString> Params;

  ParseCommandLine(*InParams, Tokens, Params);

  FMovePackageParams PackageParams;
  FParse::Value(*InParams, TEXT("PackageName="), PackageParams.Name);

  FString Maps;
  FParse::Value(*InParams, TEXT("Maps="), Maps);

  TArray<FString> MapNames;
  Maps.ParseIntoArray(MapNames, TEXT(" "), true);

  PackageParams.MapNames = MapNames;
  return PackageParams;
}

void UMoveAssetsCommandlet::MoveMeshes(const FMovePackageParams &PackageParams)
{

  AssetsObjectLibrary = UObjectLibrary::CreateLibrary(UStaticMesh::StaticClass(), false, GIsEditor);
  AssetsObjectLibrary->AddToRoot();

  for (const auto &Map : PackageParams.MapNames)
  {
    MoveMeshesForSemanticSegmentation(PackageParams.Name, Map);
  }
}

void UMoveAssetsCommandlet::MoveMeshesForSemanticSegmentation(
    const FString &PackageName,
    const FString &MapName)
{
  // Prepare a UObjectLibrary for moving assets
  const FString SrcPath = TEXT("/Game/") + PackageName + TEXT("/Maps/") + MapName;
  AssetsObjectLibrary->LoadAssetDataFromPath(*SrcPath);
  AssetsObjectLibrary->LoadAssetsFromAssetData();

  // Load Assets to move
  MapContents.Empty();
  AssetsObjectLibrary->GetAssetDataList(MapContents);
  AssetsObjectLibrary->ClearLoaded();

  TArray<FString> DestinationPaths = {SSTags::ROAD, SSTags::ROADLINES, SSTags::TERRAIN};

  // Init Map with keys
  TMap<FString, TArray<UObject *>> AssetDataMap;
  for (const auto &Paths : DestinationPaths)
  {
    AssetDataMap.Add(Paths, {});
  }

  for (const auto &MapAsset : MapContents)
  {
    // Get AssetName
    UStaticMesh *MeshAsset = CastChecked<UStaticMesh>(MapAsset.GetAsset());
    FString ObjectName = MeshAsset->GetName();

    FString AssetName;
    MapAsset.AssetName.ToString(AssetName);

    if (SrcPath.Len())
    {

      const FString CurrentPackageName = MeshAsset->GetOutermost()->GetName();

      if (!ensure(CurrentPackageName.StartsWith(SrcPath)))
      {
        continue;
      }

      // Bind between tags and classify tags in different folders according to
      // semantic segmentation
      if (AssetName.Contains(SSTags::R_ROAD))
      {
        AssetDataMap[SSTags::ROAD].Add(MeshAsset);
      }
      else if (AssetName.Contains(SSTags::R_MARKING))
      {
        AssetDataMap[SSTags::ROADLINES].Add(MeshAsset);
      }
      else if (AssetName.Contains(SSTags::R_TERRAIN))
      {
        AssetDataMap[SSTags::TERRAIN].Add(MeshAsset);
      }
    }
  }

  for (const auto &Elem : AssetDataMap)
  {
    FString DestPath = TEXT("/Game/") + PackageName + TEXT("/Static/") + Elem.Key + "/" + MapName;
    ContentBrowserUtils::MoveAssets(Elem.Value, DestPath);
  }
}

int32 UMoveAssetsCommandlet::Main(const FString &Params)
{
  FMovePackageParams PackageParams = ParseParams(Params);

  MoveMeshes(PackageParams);

  return 0;
}
#endif
