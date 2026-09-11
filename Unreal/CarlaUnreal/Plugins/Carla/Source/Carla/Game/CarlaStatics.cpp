// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "Carla/Game/CarlaStatics.h"
#include "Carla.h"
#include "Carla/ContentPacks/ContentPackManager.h"

#include <util/ue-header-guard-begin.h>
#include "AssetRegistry/ARFilter.h"
#include "AssetRegistry/AssetData.h"
#include "AssetRegistry/IAssetRegistry.h"
#include "Engine/World.h"
#include "Interfaces/IPluginManager.h"
#include "Misc/Paths.h"
#include "Misc/PackageName.h"
#include "Modules/ModuleManager.h"
#include "HAL/FileManagerGeneric.h"
#include <util/ue-header-guard-end.h>


/// UWorld assets the asset registry knows outside /Engine (project content,
/// mounted plugins, content packs). Cooked base maps live inside Pak/IoStore
/// containers where the .umap walk finds nothing; the premade
/// AssetRegistry.bin of a packaged build still lists them. The registry
/// asserts on non-game threads, so off-thread callers get an empty list.
static TArray<FAssetData> UCarlaStatics_GetRegistryWorlds()
{
  TArray<FAssetData> Worlds;
  if (!IsInGameThread())
  {
    return Worlds;
  }
  IAssetRegistry *AssetRegistry = IAssetRegistry::Get();
  if (AssetRegistry == nullptr)
  {
    return Worlds;
  }
  FARFilter Filter;
  Filter.ClassPaths.Add(UWorld::StaticClass()->GetClassPathName());
  AssetRegistry->GetAssets(Filter, Worlds);
  // Only project content and project plugins (incl. mounted content packs) hold
  // CARLA maps; engine content and engine plugins (PCG/Water/Volumetrics sample
  // levels, ...) never do.
  TSet<FString> Roots;
  Roots.Add(TEXT("/Game/"));
  for (const TSharedRef<IPlugin> &Plugin : IPluginManager::Get().GetEnabledPluginsWithContent())
  {
    if (Plugin->GetLoadedFrom() != EPluginLoadedFrom::Engine)
    {
      Roots.Add(FString::Printf(TEXT("/%s/"), *Plugin->GetName()));
    }
  }
  // A cooked World Partition town also has every generated streaming cell
  // (<Town>/_Generated_/<Cell>) registered as a UWorld named after the town;
  // those are not loadable maps. Registry rows whose package no longer exists
  // (stale cache entries, unmounted packs) are dropped too.
  Worlds.RemoveAll([&Roots](const FAssetData &World)
  {
    const FString PackageName = World.PackageName.ToString();
    int32 SecondSlash = INDEX_NONE;
    const bool bHasRoot = PackageName.FindChar(TEXT('/'), SecondSlash) &&
        PackageName.Len() > 1 && PackageName.RightChop(1).FindChar(TEXT('/'), SecondSlash);
    const FString Root = bHasRoot ? PackageName.Left(SecondSlash + 2) : FString();
    return !Roots.Contains(Root) ||
        (World.PackageFlags & PKG_CookGenerated) != 0 ||
        PackageName.Contains(TEXT("/_Generated_/")) ||
        !FPackageName::DoesPackageExist(PackageName);
  });
  return Worlds;
}

/// The recursive .umap walk over the project and plugin content dirs is
/// expensive (tens of thousands of files in an editor tree) and runs on the
/// game thread for get_available_maps / load_world, so its result is cached
/// keyed on the set of content dirs; content pack mount/unmount invalidates it.
static TArray<FString> UCarlaStatics_CachedMapFiles;
static FString UCarlaStatics_CachedMapFilesKey;
static bool UCarlaStatics_bMapFilesDirty = true;

static const TArray<FString> &UCarlaStatics_GetMapFiles()
{
  TArray<FString> PathList;
  PathList.Add(FPaths::ProjectContentDir());
  PathList.Append(UCarlaStatics::GetAllPluginContentPaths());
  const FString Key = FString::Join(PathList, TEXT(";"));
  if (!UCarlaStatics_bMapFilesDirty && Key == UCarlaStatics_CachedMapFilesKey)
  {
    return UCarlaStatics_CachedMapFiles;
  }
  UCarlaStatics_CachedMapFiles.Reset();
  for (const FString &Path : PathList)
  {
    if (FPaths::DirectoryExists(Path))
    {
      UE_LOG(LogCarla, Log, TEXT("Path: %s"), *Path);
      IFileManager::Get().FindFilesRecursive(UCarlaStatics_CachedMapFiles, *Path, TEXT("*.umap"), true, false, false);
    }
  }
  UCarlaStatics_CachedMapFilesKey = Key;
  UCarlaStatics_bMapFilesDirty = false;
  return UCarlaStatics_CachedMapFiles;
}

void UCarlaStatics::InvalidateMapCache()
{
  UCarlaStatics_bMapFilesDirty = true;
}

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
      // Content of a plugin that is not enabled (never enabled, or a content
      // pack that was unmounted) has no mount point and cannot be loaded.
      if (!Plugin->IsEnabled())
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


/// Maps that get_available_maps must not list: the OpenDRIVE-generated
/// scratch map, sub-levels and World Partition helper packages, and the test
/// maps — except EmptyMap, which is a supported empty sandbox (it ships with
/// an empty OpenDRIVE file so get_map() works).
static bool UCarlaStatics_IsHiddenMap(const FString& PathOrPackage)
{
  if (PathOrPackage.Contains("TestMaps"))
  {
    return FPaths::GetBaseFilename(PathOrPackage) != TEXT("EmptyMap");
  }
  return PathOrPackage.Contains("OpenDriveMap") || PathOrPackage.Contains("Sublevels") ||
      PathOrPackage.Contains("/BaseMap/") || PathOrPackage.Contains("/BaseLargeMap/") ||
      PathOrPackage.Contains("_Tile_");
}

TArray<FString> UCarlaStatics::GetAllMapNames()
{
  TArray<FString> MapNameList = UCarlaStatics_GetMapFiles();

  // Filter out undesired maps
  MapNameList.RemoveAll([](const FString& Name) {
      return UCarlaStatics_IsHiddenMap(Name);
  });

  for (int i = 0; i < MapNameList.Num(); i++) {
    MapNameList[i] = FPaths::GetBaseFilename(*MapNameList[i]);
  }

  // Cooked base maps: see UCarlaStatics_GetRegistryWorlds.
  {
    for (const FAssetData &World : UCarlaStatics_GetRegistryWorlds())
    {
      const FString PackageName = World.PackageName.ToString();
      if (UCarlaStatics_IsHiddenMap(PackageName))
      {
        continue;
      }
      MapNameList.AddUnique(World.AssetName.ToString());
    }
  }

  // Maps declared by mounted content packs (manifest is authoritative).
  if (const UCarlaContentPackManager *ContentPacks = UCarlaContentPackManager::Get())
  {
    for (const FString &PackMap : ContentPacks->GetPackMapNames())
    {
      MapNameList.AddUnique(PackMap);
    }
  }

  return MapNameList;
}

FString UCarlaStatics::FindMapPath(const FString &MapName)
{
  if (MapName.IsEmpty())
  {
    return FString();
  }

  // Look for matching map files
  for (const FString& FilePath : UCarlaStatics_GetMapFiles())
  {
      FString FileName = FPaths::GetBaseFilename(FilePath); // just "MyMap", no path, no extension
      if (FileName.Equals(MapName, ESearchCase::IgnoreCase))
      {
          return FilePath; // Return the full path of the first matching map. Only one map is expected.
      }
  }

  // Cooked packages can live exclusively in Pak/IoStore containers, where
  // IFileManager cannot discover their .umap files.  CARLA's generated
  // OpenDRIVE world is a project map at this canonical package path.
  const FString ProjectMapPackage = FString::Printf(
      TEXT("/Game/Carla/Maps/%s"), *MapName);
  if (FPackageName::DoesPackageExist(ProjectMapPackage))
  {
      return ProjectMapPackage;
  }

  // Cooked base maps outside the canonical folder (World Partition towns at
  // /Game/Carla/Maps/<Name>/<Name>, /Game/Carla/Maps/TestMaps/EmptyMap) are
  // only known to the asset registry. Prefer the canonical WP package over
  // any other same-named world.
  {
    const TArray<FAssetData> Worlds = UCarlaStatics_GetRegistryWorlds();
    const FString CanonicalWP = FString::Printf(TEXT("/Game/Carla/Maps/%s/%s"), *MapName, *MapName);
    FString FirstMatch;
    for (const FAssetData &World : Worlds)
    {
      if (!World.AssetName.ToString().Equals(MapName, ESearchCase::IgnoreCase))
      {
        continue;
      }
      const FString PackageName = World.PackageName.ToString();
      if (PackageName.Equals(CanonicalWP, ESearchCase::IgnoreCase))
      {
        return PackageName;
      }
      if (FirstMatch.IsEmpty())
      {
        FirstMatch = PackageName;
      }
    }
    if (!FirstMatch.IsEmpty())
    {
      return FirstMatch;
    }
  }

  // Maps of mounted content packs resolve to their manifest package path,
  // e.g. /NewPack/Maps/NewTown. Base maps win over a pack map of the same name.
  if (const UCarlaContentPackManager *ContentPacks = UCarlaContentPackManager::Get())
  {
    const FString PackMapPackage = ContentPacks->FindMapPackage(MapName);
    if (!PackMapPackage.IsEmpty())
    {
      return PackMapPackage;
    }
  }

  return FString();
}
