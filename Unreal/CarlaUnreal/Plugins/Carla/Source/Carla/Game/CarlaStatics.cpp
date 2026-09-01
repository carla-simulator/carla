// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "Carla/Game/CarlaStatics.h"
#include "Carla.h"

#include <util/ue-header-guard-begin.h>
#include "Interfaces/IPluginManager.h"
#include "Misc/Paths.h"
#include "Misc/PackageName.h"
#include "Modules/ModuleManager.h"
#include "HAL/FileManagerGeneric.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "AssetRegistry/IAssetRegistry.h"
#include "Engine/World.h"
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


// World assets known to the asset registry, which unlike IFileManager also
// sees packages that exist only inside a Pak/IoStore container.
static void GetMapAssetsFromRegistry(TArray<FAssetData>& OutAssets)
{
  IAssetRegistry& Registry =
      FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry").Get();
  Registry.SearchAllAssets(/*bSynchronousSearch=*/true);

  FARFilter Filter;
  Filter.bRecursivePaths = true;
  Filter.bRecursiveClasses = true;
  Filter.ClassPaths.Add(UWorld::StaticClass()->GetClassPathName());
  Filter.PackagePaths.Add(TEXT("/Game"));
  Registry.GetAssets(Filter, OutAssets);
}

// A map package the client should never be offered: engine/editor scratch
// levels, and the World Partition cells a large map cooks in their thousands.
static bool IsInternalMapPackage(const FString& PackageName)
{
  return PackageName.Contains(TEXT("TestMaps"))
      || PackageName.Contains(TEXT("OpenDriveMap"))
      || PackageName.Contains(TEXT("Sublevels"))
      || PackageName.Contains(TEXT("_Generated_"))
      || PackageName.Contains(TEXT("/Engine/"));
}

TArray<FString> UCarlaStatics::GetAllMapNames()
{
  TArray<FAssetData> MapAssets;
  GetMapAssetsFromRegistry(MapAssets);

  TArray<FString> MapNameList;
  for (const FAssetData& Asset : MapAssets)
  {
    const FString PackageName = Asset.PackageName.ToString();
    if (IsInternalMapPackage(PackageName))
    {
      continue;
    }
    MapNameList.AddUnique(Asset.AssetName.ToString());
  }

  if (MapNameList.Num() == 0)
  {
    // Uncooked trees with a cold registry: fall back to scanning Content.
    TArray<FString> PathList;
    PathList.Add(FPaths::ProjectContentDir());
    PathList.Append(GetAllPluginContentPaths());
    TArray<FString> Files;
    for (const FString& Path : PathList)
    {
      if (FPaths::DirectoryExists(Path))
      {
        IFileManager::Get().FindFilesRecursive(Files, *Path, TEXT("*.umap"), true, false, false);
      }
    }
    for (const FString& File : Files)
    {
      if (!IsInternalMapPackage(File))
      {
        MapNameList.AddUnique(FPaths::GetBaseFilename(File));
      }
    }
  }

  return MapNameList;
}

FString UCarlaStatics::FindMapPath(const FString &MapName)
{
  TArray<FString> ContentPaths;

  ContentPaths.Add(FPaths::ProjectContentDir());
  ContentPaths.Append(GetAllPluginContentPaths());

  // Look for matching map files
  for (const FString& Path : ContentPaths)
  {
      TArray<FString> FoundFiles;
      IFileManager::Get().FindFilesRecursive(FoundFiles, *Path, TEXT("*.umap"), true, false);

      for (const FString& FilePath : FoundFiles)
      {
          FString FileName = FPaths::GetBaseFilename(FilePath); // just "MyMap", no path, no extension
          if (FileName.Equals(MapName, ESearchCase::IgnoreCase))
          {
              return FilePath; // Return the full path of the first matching map. Only one map is expected.
          }
      }
  }

  // Cooked packages can live exclusively in Pak/IoStore containers, where
  // IFileManager cannot discover their .umap files. Ask the asset registry,
  // which resolves any layout -- including a large map's nested
  // /Game/Carla/Maps/<Town>/<Town>.
  TArray<FAssetData> MapAssets;
  GetMapAssetsFromRegistry(MapAssets);
  for (const FAssetData& Asset : MapAssets)
  {
      const FString PackageName = Asset.PackageName.ToString();
      if (IsInternalMapPackage(PackageName))
      {
          continue;
      }
      if (Asset.AssetName.ToString().Equals(MapName, ESearchCase::IgnoreCase))
      {
          return PackageName;
      }
  }

  const FString ProjectMapPackage = FString::Printf(
      TEXT("/Game/Carla/Maps/%s"), *MapName);
  if (FPackageName::DoesPackageExist(ProjectMapPackage))
  {
      return ProjectMapPackage;
  }

  return FString();
}
