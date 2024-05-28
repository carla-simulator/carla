// Copyright (c) 2024 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "MoveAssetsCommandlet.h"

#include "SSTags.h"

UMoveAssetsCommandlet::UMoveAssetsCommandlet()
{
  IsClient = false;
  IsEditor = true;
  IsServer = false;
  LogToConsole = true;
}
#if WITH_EDITORONLY_DATA

FMovePackageParams UMoveAssetsCommandlet::ParseParams(const FString &InParams) const
{
  TArray<FString> Tokens;
  TArray<FString> Params;

  ParseCommandLine(*InParams, Tokens, Params);

  // Parse and store package name
  FMovePackageParams PackageParams;
  FParse::Value(*InParams, TEXT("PackageName="), PackageParams.Name);

  // Parse and store maps name in an array
  FString Maps;
  FParse::Value(*InParams, TEXT("Maps="), Maps);

  TArray<FString> MapNames;
  Maps.ParseIntoArray(MapNames, TEXT(" "), true);

  PackageParams.MapNames = MapNames;

  return PackageParams;
}

void UMoveAssetsCommandlet::MoveAssets(const FMovePackageParams &PackageParams)
{
  // Create a library instance for loading all the assets
  AssetsObjectLibrary = UObjectLibrary::CreateLibrary(UStaticMesh::StaticClass(), false, GIsEditor);
  AssetsObjectLibrary->AddToRoot();

  // Start loading all the assets in the library and classify them for semantic
  // segmentation
  for (const auto &Map : PackageParams.MapNames)
  {
    MoveAssetsFromMapForSemanticSegmentation(PackageParams.Name, Map);
  }
}

void MoveFiles(const TArray<UObject *> &Assets, const FString &DestPath)
{
  check(DestPath.Len() > 0);

  FAssetToolsModule &AssetToolsModule = FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools");
  TArray<FAssetRenameData> AssetsAndNames;
  for (auto AssetIt = Assets.CreateConstIterator(); AssetIt; ++AssetIt)
  {
    UObject *Asset = *AssetIt;

    if (!ensure(Asset))
    {
      continue;
    }
    AssetsAndNames.Emplace(Asset, DestPath, Asset->GetName());
  }

  if (AssetsAndNames.Num() > 0)
  {
    AssetToolsModule.Get().RenameAssets(AssetsAndNames);
  }
}

void UMoveAssetsCommandlet::MoveAssetsFromMapForSemanticSegmentation(
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

  TArray<FString> DestinationPaths = {SSTags::ROAD, SSTags::ROADLINE, SSTags::TERRAIN, SSTags::GRASS, SSTags::SIDEWALK, SSTags::CURB, SSTags::GUTTER};

  // Init Map with keys
  TMap<FString, TArray<UObject *>> AssetDataMap;
  for (const auto &Paths : DestinationPaths)
  {
    AssetDataMap.Add(Paths, {});
  }

  for (const auto &MapAsset : MapContents)
  {
    // Get AssetName
    FString AssetName;
    UStaticMesh *MeshAsset = CastChecked<UStaticMesh>(MapAsset.GetAsset());
    MapAsset.AssetName.ToString(AssetName);

    if (SrcPath.Len())
    {

      const FString CurrentPackageName = MeshAsset->GetOutermost()->GetName();

      if (!ensure(CurrentPackageName.StartsWith(SrcPath)))
      {
        continue;
      }

      // Bind between tags and classify assets according to semantic
      // segmentation
      if (AssetName.Contains(SSTags::R_ROAD1) || AssetName.Contains(SSTags::R_ROAD2))
      {
        AssetDataMap[SSTags::ROAD].Add(MeshAsset);
      }
      else if (AssetName.Contains(SSTags::R_MARKING1) || AssetName.Contains(SSTags::R_MARKING2))
      {
        AssetDataMap[SSTags::ROADLINE].Add(MeshAsset);
      }
      else if (AssetName.Contains(SSTags::R_TERRAIN))
      {
        AssetDataMap[SSTags::TERRAIN].Add(MeshAsset);
      }
      else if (AssetName.Contains(SSTags::R_SIDEWALK1) || AssetName.Contains(SSTags::R_SIDEWALK2))
      {
        AssetDataMap[SSTags::SIDEWALK].Add(MeshAsset);
      }
      else if (AssetName.Contains(SSTags::R_CURB1) || AssetName.Contains(SSTags::R_CURB2))
      {
        AssetDataMap[SSTags::CURB].Add(MeshAsset);
      }
      else if (AssetName.Contains(SSTags::R_GUTTER1) || AssetName.Contains(SSTags::R_GUTTER2))
      {
        AssetDataMap[SSTags::GUTTER].Add(MeshAsset);
      }
      else
      {
        AssetDataMap[SSTags::TERRAIN].Add(MeshAsset);
      }
    }
  }

  // Move assets to correspoding semantic segmentation folders
  for (const auto &Elem : AssetDataMap)
  {
    FString DestPath = TEXT("/Game/") + PackageName + TEXT("/Static/") + Elem.Key + "/" + MapName;
    
    MoveFiles(Elem.Value, DestPath);
  }
}

int32 UMoveAssetsCommandlet::Main(const FString &Params)
{
  FMovePackageParams PackageParams = ParseParams(Params);

  MoveAssets(PackageParams);

  return 0;
}
#endif
