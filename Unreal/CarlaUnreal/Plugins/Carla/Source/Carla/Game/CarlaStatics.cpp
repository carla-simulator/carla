// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "Carla/Game/CarlaStatics.h"
#include "Carla.h"

#include <util/ue-header-guard-begin.h>
#include "AssetRegistry/AssetRegistryModule.h"
#include "AssetRegistry/IAssetRegistry.h"
#include "Engine/World.h"
#include "Interfaces/IPluginManager.h"
#include "Misc/Paths.h"
#include "Modules/ModuleManager.h"
#include "HAL/FileManagerGeneric.h"
#include <util/ue-header-guard-end.h>


TArray<FString> UCarlaStatics::GetAllPluginContentPaths()
{
  TArray<FString> OutContentDirs;
  const TArray<TSharedRef<IPlugin>> Plugins = IPluginManager::Get().GetDiscoveredPlugins();
  for (const TSharedRef<IPlugin>& Plugin : Plugins)
  {
      if (Plugin->GetLoadedFrom() == EPluginLoadedFrom::Engine)
      {
          continue;
      }

      FString ContentDir = Plugin->GetContentDir();
      if (FPaths::DirectoryExists(ContentDir))
      {
        OutContentDirs.Add(ContentDir);
      }
  }
  return OutContentDirs;
}


namespace {

TArray<FString> GetMapPackageNamesFromAssetRegistry()
{
  TArray<FString> Result;

  FAssetRegistryModule &AssetRegistryModule =
      FModuleManager::LoadModuleChecked<FAssetRegistryModule>(TEXT("AssetRegistry"));
  IAssetRegistry &AssetRegistry = AssetRegistryModule.Get();

#if WITH_EDITOR
  AssetRegistry.WaitForCompletion();
#endif

  TArray<FAssetData> MapAssets;
  AssetRegistry.GetAssetsByClass(
      FTopLevelAssetPath(TEXT("/Script/Engine"), TEXT("World")), MapAssets, true);

  for (const FAssetData &Asset : MapAssets)
  {
    const FString PackageName = Asset.PackageName.ToString();
    if (!PackageName.StartsWith(TEXT("/Engine/")))
    {
      Result.Add(PackageName);
    }
  }

  return Result;
}

bool IsNotASelectableMap(const FString &Name)
{
  return Name.Contains(TEXT("TestMaps")) ||
         Name.Contains(TEXT("OpenDriveMap")) ||
         Name.Contains(TEXT("Sublevels")) ||
         Name.Contains(TEXT("_Generated_"));
}

} // namespace

TArray<FString> UCarlaStatics::GetAllMapNames()
{
  TArray<FString> MapNameList = GetMapPackageNamesFromAssetRegistry();

  // Retain filesystem discovery for uncooked trees whose registry is empty.
  if (MapNameList.Num() == 0)
  {
    TArray<FString> PathList;
    PathList.Add(FPaths::ProjectContentDir());
    PathList.Append(GetAllPluginContentPaths());

    for (const FString &Path : PathList)
    {
      if (FPaths::DirectoryExists(Path))
      {
        UE_LOG(LogCarla, Log, TEXT("Path: %s"), *Path);
        IFileManager::Get().FindFilesRecursive(
            MapNameList, *Path, TEXT("*.umap"), true, false, false);
      }
    }
  }

  MapNameList.RemoveAll([](const FString &Name) { return IsNotASelectableMap(Name); });

  for (int i = 0; i < MapNameList.Num(); i++)
  {
    MapNameList[i] = FPaths::GetBaseFilename(*MapNameList[i]);
  }
  return MapNameList;
}

FString UCarlaStatics::FindMapPath(const FString &MapName)
{
  for (const FString &PackageName : GetMapPackageNamesFromAssetRegistry())
  {
    if (!IsNotASelectableMap(PackageName) &&
        FPaths::GetBaseFilename(PackageName).Equals(MapName, ESearchCase::IgnoreCase))
    {
      return PackageName;
    }
  }

  // Retain filesystem discovery for uncooked trees whose registry is empty.
  TArray<FString> ContentPaths;
  ContentPaths.Add(FPaths::ProjectContentDir());
  ContentPaths.Append(GetAllPluginContentPaths());

  for (const FString &Path : ContentPaths)
  {
    TArray<FString> FoundFiles;
    IFileManager::Get().FindFilesRecursive(FoundFiles, *Path, TEXT("*.umap"), true, false);

    for (const FString &FilePath : FoundFiles)
    {
      if (FPaths::GetBaseFilename(FilePath).Equals(MapName, ESearchCase::IgnoreCase))
      {
        return FilePath;
      }
    }
  }

  return FString();
}
