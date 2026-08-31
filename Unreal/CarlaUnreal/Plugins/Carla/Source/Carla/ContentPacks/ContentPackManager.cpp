// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "Carla/ContentPacks/ContentPackManager.h"
#include "Carla.h"
#include "Carla/Actor/ActorDispatcher.h"
#include "Carla/Actor/CarlaActorFactory.h"
#include "Carla/Game/CarlaStatics.h"

#include <util/ue-header-guard-begin.h>
#include "AssetRegistry/AssetRegistryState.h"
#include "AssetRegistry/IAssetRegistry.h"
#include "Engine/Engine.h"
#include "Engine/LevelStreaming.h"
#include "Engine/World.h"
#include "GenericPlatform/GenericPlatformProperties.h"
#include "HAL/FileManager.h"
#include "HAL/PlatformMisc.h"
#include "HAL/PlatformProperties.h"
#include "Interfaces/IPluginManager.h"
#include "Misc/CommandLine.h"
#include "Misc/CoreDelegates.h"
#include "Misc/EngineVersion.h"
#include "Misc/FileHelper.h"
#include "Misc/PackageName.h"
#include "Misc/Parse.h"
#include "Misc/Paths.h"
#include "UObject/Package.h"
#include "UObject/UObjectGlobals.h"
#include "UObject/UObjectIterator.h"
#include <util/ue-header-guard-end.h>

static const TCHAR *CARLA_PACK_MANIFEST_NAME = TEXT("carla-pack.json");

UCarlaContentPackManager *UCarlaContentPackManager::Get()
{
  if (GEngine == nullptr)
  {
    return nullptr;
  }
  return GEngine->GetEngineSubsystem<UCarlaContentPackManager>();
}

void UCarlaContentPackManager::Initialize(FSubsystemCollectionBase &Collection)
{
  Super::Initialize(Collection);

  // Auto-discovery is for game processes only: the cooker must not pick up
  // packs lying next to the project, and the full editor keeps its plugin
  // set as configured (packs can still be mounted explicitly there).
  if (IsRunningCommandlet() || GIsEditor)
  {
    UE_LOG(LogCarla, Log, TEXT("Content packs: auto-discovery skipped (editor/commandlet process)"));
    return;
  }
  Discover();
}

void UCarlaContentPackManager::Deinitialize()
{
  Super::Deinitialize();
}

// =============================================================================
// -- Discovery ----------------------------------------------------------------
// =============================================================================

TArray<FString> UCarlaContentPackManager::GetDiscoveryDirs() const
{
  TArray<FString> Dirs;
  Dirs.Add(FPaths::ConvertRelativePathToFull(FPaths::ProjectDir() / TEXT("Packs")));

  auto AppendList = [&Dirs](const FString &List)
  {
    TArray<FString> Parts;
    List.ParseIntoArray(Parts, TEXT(";"), true);
    for (FString &Part : Parts)
    {
      Part.TrimStartAndEndInline();
      Part = Part.TrimQuotes();
      if (!Part.IsEmpty())
      {
        Dirs.AddUnique(FPaths::ConvertRelativePathToFull(Part));
      }
    }
  };

  // -carla-packs=<dir>[;<dir>]; quote the value ("-carla-packs=\"/a b;/c\"") when
  // a directory contains spaces, the token otherwise ends at the first space.
  FString FromCommandLine;
  if (FParse::Value(FCommandLine::Get(), TEXT("-carla-packs="), FromCommandLine))
  {
    AppendList(FromCommandLine);
  }

  const FString FromEnv = FPlatformMisc::GetEnvironmentVariable(TEXT("CARLA_PACKS"));
  if (!FromEnv.IsEmpty())
  {
    AppendList(FromEnv);
  }

  return Dirs;
}

void UCarlaContentPackManager::Discover()
{
  if (bDiscovered)
  {
    return;
  }
  bDiscovered = true;

  const FString Stamp = GetBaseReleaseStamp();
  UE_LOG(LogCarla, Log, TEXT("Content packs: base release stamp '%s'"),
      Stamp.IsEmpty() ? TEXT("<none: editor/dev build>") : *Stamp);

  for (const FString &Dir : GetDiscoveryDirs())
  {
    if (!FPaths::DirectoryExists(Dir))
    {
      UE_LOG(LogCarla, Log, TEXT("Content packs: directory '%s' does not exist, skipping"), *Dir);
      continue;
    }

    TArray<FString> Children;
    IFileManager::Get().FindFiles(Children, *(Dir / TEXT("*")), false, true);
    Children.Sort();
    for (const FString &Child : Children)
    {
      const FString Manifest = Dir / Child / CARLA_PACK_MANIFEST_NAME;
      if (!FPaths::FileExists(Manifest))
      {
        continue;
      }
      FString Error;
      if (!Mount(Manifest, Error))
      {
        UE_LOG(LogCarla, Warning, TEXT("Content packs: '%s' not mounted: %s"), *Manifest, *Error);
      }
    }
  }

  // Packs the engine mounted itself (Plugins/<Pack>.upluginmanifest in a
  // packaged build, or an enabled plugin in the editor): never mount them a
  // second time, just register their manifest so maps and catalogs show up.
  for (const TSharedRef<IPlugin> &Plugin : IPluginManager::Get().GetEnabledPluginsWithContent())
  {
    const FString Manifest = FPaths::ConvertRelativePathToFull(Plugin->GetBaseDir()) / CARLA_PACK_MANIFEST_NAME;
    if (!Plugin->IsMounted() || !FPaths::FileExists(Manifest) || FindPack(Plugin->GetName()) != nullptr)
    {
      continue;
    }
    FString Error;
    FCarlaContentPackManifest Parsed;
    if (!FCarlaContentPackManifest::LoadFromFile(Manifest, Parsed, Error))
    {
      UE_LOG(LogCarla, Warning, TEXT("Content packs: engine-mounted plugin '%s' not adopted: %s"),
          *Plugin->GetName(), *Error);
      continue;
    }
    if (!Parsed.Name.Equals(Plugin->GetName(), ESearchCase::IgnoreCase))
    {
      UE_LOG(LogCarla, Warning,
          TEXT("Content packs: engine-mounted plugin '%s' not adopted: manifest name '%s' does not match the plugin name"),
          *Plugin->GetName(), *Parsed.Name);
      continue;
    }
    if (!Mount(Manifest, Error))
    {
      UE_LOG(LogCarla, Warning, TEXT("Content packs: engine-mounted plugin '%s' not adopted: %s"),
          *Plugin->GetName(), *Error);
    }
  }
}

// =============================================================================
// -- Queries ------------------------------------------------------------------
// =============================================================================

const FCarlaContentPack *UCarlaContentPackManager::FindPack(const FString &Name) const
{
  for (const FCarlaContentPack &Pack : Packs)
  {
    if (Pack.GetName().Equals(Name, ESearchCase::IgnoreCase))
    {
      return &Pack;
    }
  }
  return nullptr;
}

FString UCarlaContentPackManager::FindMapPackage(const FString &MapName) const
{
  for (const FCarlaContentPack &Pack : Packs)
  {
    if (!Pack.bMounted)
    {
      continue;
    }
    for (const FCarlaContentPackMap &Map : Pack.Manifest.Maps)
    {
      if (Map.Name.Equals(MapName, ESearchCase::IgnoreCase))
      {
        return Map.Package;
      }
    }
  }
  return FString();
}

TArray<FString> UCarlaContentPackManager::GetPackMapNames() const
{
  TArray<FString> Names;
  for (const FCarlaContentPack &Pack : Packs)
  {
    if (Pack.bMounted)
    {
      for (const FCarlaContentPackMap &Map : Pack.Manifest.Maps)
      {
        Names.AddUnique(Map.Name);
      }
    }
  }
  return Names;
}

TArray<FString> UCarlaContentPackManager::GetPackContentDirs() const
{
  TArray<FString> Dirs;
  for (const FCarlaContentPack &Pack : Packs)
  {
    if (Pack.bMounted)
    {
      Dirs.Add(Pack.ContentDir);
    }
  }
  return Dirs;
}

TArray<FString> UCarlaContentPackManager::FindPackCatalogFiles(const FString &RelativeName)
{
  TArray<FString> Files;
  const UCarlaContentPackManager *Manager = Get();
  if (Manager == nullptr)
  {
    return Files;
  }
  for (const FString &ContentDir : Manager->GetPackContentDirs())
  {
    const FString File = ContentDir / TEXT("Config") / RelativeName;
    if (IFileManager::Get().FileExists(*File))
    {
      Files.Add(File);
    }
  }
  return Files;
}

FString UCarlaContentPackManager::GetBaseReleaseStamp()
{
  FString Stamp;
  const FString StampFile = FPaths::ProjectDir() / TEXT("BaseRelease");
  if (FPaths::FileExists(StampFile) && FFileHelper::LoadFileToString(Stamp, *StampFile))
  {
    Stamp.TrimStartAndEndInline();
  }
  return Stamp;
}

// =============================================================================
// -- Mount --------------------------------------------------------------------
// =============================================================================

bool UCarlaContentPackManager::CheckCompatibility(
    const FCarlaContentPackManifest &Manifest,
    FString &OutError) const
{
  const FString Stamp = GetBaseReleaseStamp();
  if (Stamp.IsEmpty())
  {
    UE_LOG(LogCarla, Warning,
        TEXT("Content pack '%s': this build has no BaseRelease stamp (editor/dev build); "
             "skipping the base release check (pack was built for '%s')"),
        *Manifest.Name, *Manifest.BaseRelease);
  }
  else if (Manifest.BaseRelease.IsEmpty())
  {
    UE_LOG(LogCarla, Warning,
        TEXT("Content pack '%s': manifest has no base_release; this build is '%s'"),
        *Manifest.Name, *Stamp);
  }
  else if (!Manifest.BaseRelease.Equals(Stamp, ESearchCase::IgnoreCase))
  {
    OutError = FString::Printf(
        TEXT("pack '%s' was built against base release '%s' but this server is '%s'; "
             "rebuild the pack against this release"),
        *Manifest.Name, *Manifest.BaseRelease, *Stamp);
    return false;
  }

  const FString ThisPlatform = FPlatformProperties::IniPlatformName();
  if (Manifest.Platform.IsEmpty())
  {
    UE_LOG(LogCarla, Warning,
        TEXT("Content pack '%s': manifest has no platform; this server runs on '%s'"),
        *Manifest.Name, *ThisPlatform);
  }
  else if (!Manifest.Platform.Equals(ThisPlatform, ESearchCase::IgnoreCase))
  {
    OutError = FString::Printf(
        TEXT("pack '%s' was built for platform '%s' but this server runs on '%s'"),
        *Manifest.Name, *Manifest.Platform, *ThisPlatform);
    return false;
  }

  if (!Manifest.EngineVersion.IsEmpty())
  {
    const FEngineVersion &Current = FEngineVersion::Current();
    const FString ThisEngine = FString::Printf(TEXT("%u.%u.%u"),
        Current.GetMajor(), Current.GetMinor(), Current.GetPatch());
    if (!Manifest.EngineVersion.Equals(ThisEngine))
    {
      UE_LOG(LogCarla, Warning,
          TEXT("Content pack '%s': built with engine %s, this server runs %s"),
          *Manifest.Name, *Manifest.EngineVersion, *ThisEngine);
    }
  }

  return true;
}

bool UCarlaContentPackManager::MountPaks(FCarlaContentPack &Pack, FString &OutError)
{
  Pack.MountedPaks.Reset();

  // MountExplicitlyLoadedPlugin only registers the mount point: the pak loop
  // in FPluginManager::ProcessEnabledPlugins runs for startup plugins only,
  // so a pack mounted at runtime has to mount its own containers (as Game
  // Feature Plugins do). The .utoc/.ucas siblings are picked up by the pak
  // platform file from the .pak path.
  if (!FPlatformProperties::RequiresCookedData())
  {
    // Editor process: loose assets under Content/ are served by the mount
    // point itself, cooked containers cannot be used here.
    return true;
  }

  // Containers are named by the stager (<DLCName><ProjectName>-<Platform>):
  // glob, never assume a name. The .pak is the handle FPakPlatformFile mounts
  // and it picks up the sibling .utoc/.ucas itself.
  const FString PaksDir = Pack.ContentDir / TEXT("Paks") / FPlatformProperties::PlatformName();
  TArray<FString> PakFiles;
  IFileManager::Get().FindFiles(PakFiles, *(PaksDir / TEXT("*.pak")), true, false);
  PakFiles.Sort();

  if (PakFiles.Num() == 0)
  {
    TArray<FString> TocFiles;
    IFileManager::Get().FindFiles(TocFiles, *(PaksDir / TEXT("*.utoc")), true, false);
    if (TocFiles.Num() > 0)
    {
      OutError = FString::Printf(
          TEXT("'%s' has %d .utoc container(s) but no .pak next to them; the stager always writes the "
               ".pak stub, ship all three of .pak/.utoc/.ucas"), *PaksDir, TocFiles.Num());
      return false;
    }
    UE_LOG(LogCarla, Warning,
        TEXT("Content pack '%s': no .pak under '%s'; only loose files (if any) will be visible"),
        *Pack.GetName(), *PaksDir);
    return true;
  }

  if (!FCoreDelegates::MountPak.IsBound())
  {
    OutError = TEXT("pak mounting is unavailable in this process (FCoreDelegates::MountPak not bound)");
    return false;
  }

  for (const FString &PakName : PakFiles)
  {
    const FString PakPath = PaksDir / PakName;
    if (FCoreDelegates::MountPak.Execute(PakPath, 0) == nullptr)
    {
      OutError = FString::Printf(TEXT("failed to mount pak '%s'"), *PakPath);
      UnmountPaks(Pack);
      return false;
    }
    UE_LOG(LogCarla, Log, TEXT("Content pack '%s': mounted pak '%s'"), *Pack.GetName(), *PakPath);
    Pack.MountedPaks.Add(PakPath);
  }
  return true;
}

void UCarlaContentPackManager::UnmountPaks(FCarlaContentPack &Pack)
{
  for (int32 i = Pack.MountedPaks.Num() - 1; i >= 0; --i)
  {
    const FString &PakPath = Pack.MountedPaks[i];
    if (FCoreDelegates::OnUnmountPak.IsBound() && FCoreDelegates::OnUnmountPak.Execute(PakPath))
    {
      UE_LOG(LogCarla, Log, TEXT("Content pack '%s': unmounted pak '%s'"), *Pack.GetName(), *PakPath);
    }
    else
    {
      UE_LOG(LogCarla, Warning, TEXT("Content pack '%s': failed to unmount pak '%s'"), *Pack.GetName(), *PakPath);
    }
  }
  Pack.MountedPaks.Reset();
}

bool UCarlaContentPackManager::MountPlugin(FCarlaContentPack &Pack, FString &OutError)
{
  IPluginManager &PluginManager = IPluginManager::Get();
  const FString &Name = Pack.GetName();

  TSharedPtr<IPlugin> Existing = PluginManager.FindPlugin(Name);
  if (Existing.IsValid())
  {
    const FString ExistingFile = FPaths::ConvertRelativePathToFull(Existing->GetDescriptorFileName());
    if (!FPaths::IsSamePath(ExistingFile, Pack.PluginFile))
    {
      OutError = FString::Printf(
          TEXT("pack name '%s' collides with the plugin at '%s'"), *Name, *ExistingFile);
      return false;
    }
    if (Existing->IsEnabled() && FPackageName::MountPointExists(FString::Printf(TEXT("/%s/"), *Name)))
    {
      OutError = FString::Printf(TEXT("plugin '%s' is already mounted"), *Name);
      return false;
    }
  }
  bool bAddedNow = false;
  if (!Existing.IsValid())
  {
    FText FailReason;
    if (!PluginManager.AddToPluginsList(Pack.PluginFile, &FailReason))
    {
      OutError = FString::Printf(TEXT("cannot register plugin '%s': %s"),
          *Pack.PluginFile, *FailReason.ToString());
      return false;
    }
    bAddedNow = true;
  }

  if (!PluginManager.MountExplicitlyLoadedPlugin(Name))
  {
    OutError = FString::Printf(
        TEXT("MountExplicitlyLoadedPlugin('%s') failed; the .uplugin must set "
             "\"ExplicitlyLoaded\": true and \"CanContainContent\": true"), *Name);
    if (bAddedNow)
    {
      // Otherwise the bad descriptor stays registered and a retry after
      // fixing the .uplugin keeps failing until restart.
      PluginManager.RemoveFromPluginsList(Pack.PluginFile);
    }
    return false;
  }

  const FString MountPoint = FString::Printf(TEXT("/%s/"), *Name);
  if (!FPackageName::MountPointExists(MountPoint))
  {
    OutError = FString::Printf(TEXT("mount point '%s' was not registered"), *MountPoint);
    return false;
  }

  TSharedPtr<IPlugin> Plugin = PluginManager.FindPlugin(Name);
  if (Plugin.IsValid())
  {
    Pack.ContentDir = FPaths::ConvertRelativePathToFull(Plugin->GetContentDir());
  }
  UE_LOG(LogCarla, Log, TEXT("Content pack '%s': mounted '%s' -> '%s'"),
      *Name, *MountPoint, *Pack.ContentDir);
  return true;
}

void UCarlaContentPackManager::RegisterAssetRegistry(FCarlaContentPack &Pack)
{
  IAssetRegistry *AssetRegistry = IAssetRegistry::Get();
  if (AssetRegistry == nullptr)
  {
    UE_LOG(LogCarla, Warning, TEXT("Content pack '%s': asset registry unavailable"), *Pack.GetName());
    return;
  }

  const FString MountPoint = FString::Printf(TEXT("/%s/"), *Pack.GetName());
  const FString StateFile = Pack.PackDir / TEXT("AssetRegistry.bin");

  if (FPaths::FileExists(StateFile))
  {
    if (AssetRegistry->ActiveMountsRegisterAndLoadFilePathSynchronous(StateFile))
    {
      UE_LOG(LogCarla, Log, TEXT("Content pack '%s': loaded asset registry '%s'"),
          *Pack.GetName(), *StateFile);
      return;
    }
    FAssetRegistryState State;
    if (FAssetRegistryState::LoadFromDisk(*StateFile, FAssetRegistryLoadOptions(), State))
    {
      AssetRegistry->AppendState(State);
      UE_LOG(LogCarla, Log, TEXT("Content pack '%s': appended asset registry '%s'"),
          *Pack.GetName(), *StateFile);
      return;
    }
    UE_LOG(LogCarla, Warning,
        TEXT("Content pack '%s': cannot load '%s', falling back to a synchronous scan of %s"),
        *Pack.GetName(), *StateFile, *MountPoint);
  }
  else
  {
    UE_LOG(LogCarla, Warning,
        TEXT("Content pack '%s': no AssetRegistry.bin, scanning %s synchronously"),
        *Pack.GetName(), *MountPoint);
  }

  AssetRegistry->ScanPathsSynchronous({MountPoint}, true);
}

bool UCarlaContentPackManager::Mount(
    const FString &Path,
    FString &OutError,
    const FCarlaContentPack **OutPack)
{
  if (OutPack != nullptr)
  {
    *OutPack = nullptr;
  }
  if (Path.IsEmpty())
  {
    OutError = TEXT("empty pack path");
    return false;
  }

  FString ManifestPath = FPaths::ConvertRelativePathToFull(Path);
  if (FPaths::DirectoryExists(ManifestPath))
  {
    ManifestPath = ManifestPath / CARLA_PACK_MANIFEST_NAME;
  }
  if (!FPaths::FileExists(ManifestPath))
  {
    OutError = FString::Printf(
        TEXT("'%s' is not a content pack: no directory or manifest at '%s'"), *Path, *ManifestPath);
    return false;
  }

  FCarlaContentPackManifest Manifest;
  if (!FCarlaContentPackManifest::LoadFromFile(ManifestPath, Manifest, OutError))
  {
    return false;
  }

  // Reuse the slot of an unmounted pack of the same name (re-mount).
  int32 Index = INDEX_NONE;
  for (int32 i = 0; i < Packs.Num(); ++i)
  {
    if (Packs[i].GetName().Equals(Manifest.Name, ESearchCase::IgnoreCase))
    {
      if (Packs[i].bMounted)
      {
        OutError = FString::Printf(TEXT("pack '%s' is already mounted from '%s'"),
            *Manifest.Name, *Packs[i].PackDir);
        if (OutPack != nullptr)
        {
          *OutPack = &Packs[i];
        }
        return false;
      }
      Index = i;
      break;
    }
  }

  if (!CheckCompatibility(Manifest, OutError))
  {
    return false;
  }

  FCarlaContentPack Pack;
  Pack.Manifest = Manifest;
  Pack.PackDir = Manifest.PackDir;
  Pack.PluginFile = Pack.PackDir / (Manifest.Name + TEXT(".uplugin"));
  Pack.ContentDir = Pack.PackDir / TEXT("Content");

  if (!FPaths::FileExists(Pack.PluginFile))
  {
    OutError = FString::Printf(TEXT("pack '%s' has no plugin descriptor '%s'"),
        *Manifest.Name, *Pack.PluginFile);
    return false;
  }
  if (!FPaths::DirectoryExists(Pack.ContentDir))
  {
    UE_LOG(LogCarla, Warning, TEXT("Content pack '%s': no Content directory at '%s'"),
        *Manifest.Name, *Pack.ContentDir);
  }

  // A plugin the engine already mounted (Plugins/<Pack>.upluginmanifest at
  // startup) is adopted, not mounted twice: its paks and mount point are
  // live, only the manifest and the asset registry state are registered.
  TSharedPtr<IPlugin> EnginePlugin = IPluginManager::Get().FindPlugin(Manifest.Name);
  if (EnginePlugin.IsValid())
  {
    // Name collision is decided here, before any container is mounted.
    const FString ExistingFile = FPaths::ConvertRelativePathToFull(EnginePlugin->GetDescriptorFileName());
    if (!FPaths::IsSamePath(ExistingFile, Pack.PluginFile))
    {
      OutError = FString::Printf(
          TEXT("pack name '%s' collides with the plugin at '%s'"), *Manifest.Name, *ExistingFile);
      return false;
    }
  }
  if (EnginePlugin.IsValid() && EnginePlugin->IsMounted())
  {
    Pack.bEngineMounted = true;
    Pack.ContentDir = FPaths::ConvertRelativePathToFull(EnginePlugin->GetContentDir());
    UE_LOG(LogCarla, Log, TEXT("Content pack '%s': plugin already mounted by the engine, adopting it"),
        *Manifest.Name);
  }
  else
  {
    if (!MountPaks(Pack, OutError))
    {
      return false;
    }
    if (!MountPlugin(Pack, OutError))
    {
      UnmountPaks(Pack);
      return false;
    }
  }
  RegisterAssetRegistry(Pack);

  Pack.bMounted = true;
  if (Index == INDEX_NONE)
  {
    Index = Packs.Add(MoveTemp(Pack));
  }
  else
  {
    Packs[Index] = MoveTemp(Pack);
  }

  const FCarlaContentPack &Mounted = Packs[Index];
  UCarlaStatics::InvalidateMapCache();
  UE_LOG(LogCarla, Log, TEXT("Content pack '%s' %s mounted from '%s' (%d maps: %s)"),
      *Mounted.GetName(), *Mounted.Manifest.Version, *Mounted.PackDir,
      Mounted.Manifest.Maps.Num(), *FString::Join(Mounted.Manifest.GetMapNames(), TEXT(", ")));
  if (OutPack != nullptr)
  {
    *OutPack = &Mounted;
  }
  // The pack's catalogs are merged now, but the actor definitions of the
  // running episode were built when its factories were registered. Bind what
  // the pack added, so its blueprints are usable without a world load (the
  // unmount path drops them again through ReleaseContentPack).
  for (TObjectIterator<UActorDispatcher> DispatcherIt; DispatcherIt; ++DispatcherIt)
  {
    if (!IsValid(*DispatcherIt))
    {
      continue;
    }
    for (TObjectIterator<ACarlaActorFactory> FactoryIt; FactoryIt; ++FactoryIt)
    {
      if (IsValid(*FactoryIt))
      {
        DispatcherIt->BindNewDefinitions(**FactoryIt);
      }
    }
  }

  OnPackMounted.Broadcast(Mounted);
  return true;
}

// =============================================================================
// -- Unmount ------------------------------------------------------------------
// =============================================================================

bool UCarlaContentPackManager::IsPackInUse(const FCarlaContentPack &Pack, FString &OutReason) const
{
  const FString MountPoint = FString::Printf(TEXT("/%s/"), *Pack.GetName());

  if (GEngine != nullptr)
  {
    for (const FWorldContext &Context : GEngine->GetWorldContexts())
    {
      UWorld *World = Context.World();
      if (World == nullptr)
      {
        continue;
      }
      const FString WorldPackage = World->GetOutermost()->GetName();
      if (WorldPackage.StartsWith(MountPoint))
      {
        OutReason = FString::Printf(TEXT("its map '%s' is the current world"), *WorldPackage);
        return true;
      }
      for (const ULevelStreaming *Streaming : World->GetStreamingLevels())
      {
        if (Streaming != nullptr &&
            Streaming->GetWorldAssetPackageName().StartsWith(MountPoint))
        {
          OutReason = FString::Printf(TEXT("its level '%s' is streamed into the current world"),
              *Streaming->GetWorldAssetPackageName());
          return true;
        }
      }
    }
  }

  return false;
}

static TArray<FString> UCarlaContentPackManager_LoadedPackages(const FString &MountPoint)
{
  TArray<FString> Alive;
  for (TObjectIterator<UPackage> It; It; ++It)
  {
    const FString PackageName = It->GetName();
    if (PackageName.StartsWith(MountPoint))
    {
      Alive.Add(PackageName);
      if (Alive.Num() >= 5)
      {
        break;
      }
    }
  }
  return Alive;
}

bool UCarlaContentPackManager::ArePackObjectsAlive(const FCarlaContentPack &Pack, FString &OutReason)
{
  const FString MountPoint = FString::Printf(TEXT("/%s/"), *Pack.GetName());

  // Nothing from the pack was ever loaded: no need for a full GC.
  if (UCarlaContentPackManager_LoadedPackages(MountPoint).Num() == 0)
  {
    return false;
  }

  // Anything else still referencing pack content shows up as a live package
  // under the mount point once unreferenced objects are gone.
  CollectGarbage(GARBAGE_COLLECTION_KEEPFLAGS, true);

  const TArray<FString> Alive = UCarlaContentPackManager_LoadedPackages(MountPoint);
  if (Alive.Num() > 0)
  {
    OutReason = FString::Printf(TEXT("objects from the pack are still loaded (%s%s)"),
        *FString::Join(Alive, TEXT(", ")), Alive.Num() >= 5 ? TEXT(", ...") : TEXT(""));
    return true;
  }
  return false;
}

bool UCarlaContentPackManager::Unmount(const FString &Name, FString &OutError)
{
  int32 Index = INDEX_NONE;
  for (int32 i = 0; i < Packs.Num(); ++i)
  {
    if (Packs[i].GetName().Equals(Name, ESearchCase::IgnoreCase))
    {
      Index = i;
      break;
    }
  }
  if (Index == INDEX_NONE)
  {
    OutError = FString::Printf(TEXT("unknown content pack '%s'"), *Name);
    return false;
  }
  FCarlaContentPack &Pack = Packs[Index];
  if (!Pack.bMounted)
  {
    OutError = FString::Printf(TEXT("content pack '%s' is not mounted"), *Name);
    return false;
  }

  if (Pack.bEngineMounted)
  {
    OutError = FString::Printf(
        TEXT("content pack '%s' was mounted by the engine at startup (Plugins/%s.upluginmanifest); "
             "remove it from the Plugins directory and restart the server to unload it"),
        *Pack.GetName(), *Pack.GetName());
    return false;
  }

  // Cheap refusal first (state must not change on a refused unmount): the
  // pack map or one of its levels is loaded.
  FString Reason;
  if (IsPackInUse(Pack, Reason))
  {
    OutError = FString::Printf(TEXT("cannot unmount content pack '%s': %s; load another map first"),
        *Pack.GetName(), *Reason);
    return false;
  }

  // Definition caches hold the pack's blueprint classes and meshes; drop
  // them first, otherwise the garbage collection below keeps them alive.
  {
    const FString MountPoint = FString::Printf(TEXT("/%s/"), *Pack.GetName());
    for (TObjectIterator<ACarlaActorFactory> It; It; ++It)
    {
      if (IsValid(*It))
      {
        It->ReleaseContentPack(MountPoint);
      }
    }
    for (TObjectIterator<UActorDispatcher> It; It; ++It)
    {
      if (IsValid(*It))
      {
        It->ReleaseContentPack(MountPoint);
      }
    }
  }

  if (ArePackObjectsAlive(Pack, Reason))
  {
    // Rare: something outside the current world still holds pack objects.
    // The catalog entries were already dropped; a reload_world rebuilds them.
    UE_LOG(LogCarla, Warning,
        TEXT("Content pack '%s': unmount refused after its blueprint definitions were dropped; "
             "reload_world restores them"), *Pack.GetName());
    OutError = FString::Printf(TEXT("cannot unmount content pack '%s': %s; reload the world and retry"),
        *Pack.GetName(), *Reason);
    return false;
  }

  FText FailReason;
  if (!IPluginManager::Get().UnmountExplicitlyLoadedPlugin(Pack.GetName(), &FailReason))
  {
    OutError = FString::Printf(TEXT("cannot unmount plugin '%s': %s"),
        *Pack.GetName(), *FailReason.ToString());
    return false;
  }
  UnmountPaks(Pack);
  Pack.bMounted = false;
  UCarlaStatics::InvalidateMapCache();

  UE_LOG(LogCarla, Log, TEXT("Content pack '%s' unmounted"), *Pack.GetName());
  OnPackUnmounted.Broadcast(Pack);
  return true;
}
