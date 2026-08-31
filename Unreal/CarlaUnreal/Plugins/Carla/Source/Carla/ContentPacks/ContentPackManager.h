// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "Carla/ContentPacks/ContentPackManifest.h"

#include <util/ue-header-guard-begin.h>
#include "CoreMinimal.h"
#include "Subsystems/EngineSubsystem.h"
#include <util/ue-header-guard-end.h>

#include "ContentPackManager.generated.h"

/// A content pack known to the manager (discovered or mounted explicitly).
struct CARLA_API FCarlaContentPack
{
  FCarlaContentPackManifest Manifest;

  /// Absolute pack directory (contains carla-pack.json and <Name>.uplugin).
  FString PackDir;

  /// Absolute path of <PackDir>/<Name>.uplugin.
  FString PluginFile;

  /// Absolute path of the plugin content dir (<PackDir>/Content), the
  /// filesystem side of the "/<Name>/" mount point. Non-asset files staged
  /// in the pack (xodr, nav .bin, Config/*.json) are read from here.
  FString ContentDir;

  /// Pak files mounted for this pack (cooked builds only).
  TArray<FString> MountedPaks;

  bool bMounted = false;

  /// True when the plugin was mounted by the engine at startup (a pack
  /// dropped under Plugins/ with a .upluginmanifest): the manager only
  /// registers its manifest and catalogs and never unmounts it.
  bool bEngineMounted = false;

  const FString &GetName() const { return Manifest.Name; }
};

DECLARE_MULTICAST_DELEGATE_OneParam(FCarlaContentPackEvent, const FCarlaContentPack &);

/// Mounts CARLA content packs (content-only Unreal plugins cooked as DLC
/// against a base release, wrapped with a carla-pack.json manifest) into the
/// running process, editor or cooked game alike.
///
/// Implemented as a UEngineSubsystem: it outlives game instances and worlds,
/// so packs survive load_world, and it exists before the first map loads.
/// Auto-discovery runs from Initialize() in game processes (packaged binary
/// or editor -game); FCarlaEngine::NotifyInitGame calls Discover() again as
/// a no-op safety net so the RPC server never starts before the packs are in.
/// The full editor and commandlets (the cooker) do not auto-mount packs.
UCLASS()
class CARLA_API UCarlaContentPackManager : public UEngineSubsystem
{
  GENERATED_BODY()

public:

  static UCarlaContentPackManager *Get();

  virtual void Initialize(FSubsystemCollectionBase &Collection) override;

  virtual void Deinitialize() override;

  /// Mount every pack found in <ProjectDir>/Packs/*/carla-pack.json, the
  /// directories listed in -carla-packs=<dir>[;<dir>] and $CARLA_PACKS, and
  /// adopt every plugin the engine already mounted at startup that carries a
  /// carla-pack.json next to its .uplugin (drop-in under Plugins/).
  /// Idempotent; failures are logged, not fatal.
  void Discover();

  /// Mount a pack. Path is the pack directory or its carla-pack.json.
  /// On failure OutError names the reason and the pack list is unchanged
  /// (unless the pack was already known, in which case it stays unmounted).
  bool Mount(const FString &Path, FString &OutError, const FCarlaContentPack **OutPack = nullptr);

  /// Unmount a pack by name. Refused while a world of the pack is loaded or
  /// any object under "/<Name>/" survives a garbage collection.
  bool Unmount(const FString &Name, FString &OutError);

  const TArray<FCarlaContentPack> &GetPacks() const { return Packs; }

  const FCarlaContentPack *FindPack(const FString &Name) const;

  /// Long package name of a map declared by a mounted pack, or empty.
  FString FindMapPackage(const FString &MapName) const;

  /// Map names declared by all mounted packs.
  TArray<FString> GetPackMapNames() const;

  /// Content dirs of all mounted packs, in mount order.
  TArray<FString> GetPackContentDirs() const;

  /// <ContentDir>/Config/<RelativeName> of every mounted pack where the file
  /// exists, in mount order (later packs override earlier ones by id).
  static TArray<FString> FindPackCatalogFiles(const FString &RelativeName);

  /// Merge catalog entries: an entry whose key already exists replaces it in
  /// place (later files override by id), new keys are appended.
  template <typename T, typename KeyFn>
  static void MergeCatalog(TArray<T> &Into, const TArray<T> &From, KeyFn Key)
  {
    for (const T &Entry : From)
    {
      const FString EntryKey = Key(Entry);
      const int32 Existing = Into.IndexOfByPredicate([&](const T &Other)
      {
        return Key(Other).Equals(EntryKey, ESearchCase::IgnoreCase);
      });
      if (Existing != INDEX_NONE)
      {
        Into[Existing] = Entry;
      }
      else
      {
        Into.Add(Entry);
      }
    }
  }

  /// Base release stamp of this binary (<ProjectDir>/BaseRelease), empty in
  /// editor/dev builds.
  static FString GetBaseReleaseStamp();

  FCarlaContentPackEvent OnPackMounted;

  FCarlaContentPackEvent OnPackUnmounted;

private:

  bool CheckCompatibility(const FCarlaContentPackManifest &Manifest, FString &OutError) const;

  bool MountPaks(FCarlaContentPack &Pack, FString &OutError);

  void UnmountPaks(FCarlaContentPack &Pack);

  bool MountPlugin(FCarlaContentPack &Pack, FString &OutError);

  void RegisterAssetRegistry(FCarlaContentPack &Pack);

  /// The pack map or one of its levels is loaded in some world (no GC).
  bool IsPackInUse(const FCarlaContentPack &Pack, FString &OutReason) const;

  /// Objects under "/<Pack>/" survive a garbage collection (GC only runs
  /// when something from the pack was loaded at all).
  bool ArePackObjectsAlive(const FCarlaContentPack &Pack, FString &OutReason);

  TArray<FString> GetDiscoveryDirs() const;

  TArray<FCarlaContentPack> Packs;

  bool bDiscovered = false;
};
