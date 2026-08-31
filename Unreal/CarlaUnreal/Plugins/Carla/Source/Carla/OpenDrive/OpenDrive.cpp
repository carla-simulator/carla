// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "Carla/OpenDrive/OpenDrive.h"
#include "Carla.h"
#include "Carla/ContentPacks/ContentPackManager.h"
#include "Carla/Game/CarlaGameModeBase.h"
#include "Carla/Game/CarlaStatics.h"

#include <util/ue-header-guard-begin.h>
#include "Engine/Engine.h"
#include "GenericPlatform/GenericPlatformProcess.h"
#include "Misc/FileHelper.h"
#include "HAL/FileManagerGeneric.h"
#include <util/ue-header-guard-end.h>

FString UOpenDrive::GetSavedXODRPath(const FString &MapName)
{
  return FPaths::ConvertRelativePathToFull(
      FPaths::ProjectSavedDir() / TEXT("OpenDrive") / (MapName + TEXT(".xodr")));
}

FString UOpenDrive::FindPathToXODRFile(const FString &InMapName){

  FString MapName = InMapName;

#if WITH_EDITOR
    {
      FString CorrectedMapName = MapName;
      constexpr auto PIEPrefix = TEXT("UEDPIE_0_");
      CorrectedMapName.RemoveFromStart(PIEPrefix);
      MapName = CorrectedMapName;
    }
#endif // WITH_EDITOR

  FString XODRFileName = MapName + TEXT(".xodr");

  auto &FileManager = IFileManager::Get();

  // Generated OpenDRIVE worlds write their xodr to Saved/ (the content dir
  // is pak-backed in a packaged build); that copy is the current one.
  const FString SavedFilePath = GetSavedXODRPath(MapName);
  if (FileManager.FileExists(*SavedFilePath))
  {
    return SavedFilePath;
  }

  const FString DefaultFilePath =
      FPaths::ProjectContentDir() +
      TEXT("Carla/Maps/OpenDrive/") +
      XODRFileName;

  if (FileManager.FileExists(*DefaultFilePath))
  {
    return DefaultFilePath;
  }

  // Mounted content packs ship <Content>/Maps/OpenDrive/<Map>.xodr.
  if (const UCarlaContentPackManager *ContentPacks = UCarlaContentPackManager::Get())
  {
    for (const FString &PackContentDir : ContentPacks->GetPackContentDirs())
    {
      const FString PackFilePath = PackContentDir / TEXT("Maps/OpenDrive") / XODRFileName;
      if (FileManager.FileExists(*PackFilePath))
      {
        return PackFilePath;
      }
    }
  }

  TArray<FString> FilesFound;
  FileManager.FindFilesRecursive(
      FilesFound,
      *FPaths::ProjectContentDir(),
      *XODRFileName,
      true,
      false,
      false);

  if (FilesFound.Num() > 0)
  {
    return FilesFound[0u];
  }

  FString PluginPath = FPaths::ProjectDir() + TEXT("Plugins/") + MapName + TEXT("/Content/Maps/OpenDrive/") + XODRFileName;

  if (FileManager.FileExists(*PluginPath))
  {
    return PluginPath;
  }

  FString PluginsDir = FPaths::ProjectDir() + TEXT("Plugins/");

  TArray<FString> PluginFilesFound;
  FileManager.FindFilesRecursive(
      PluginFilesFound,
      *PluginsDir,
      *XODRFileName,
      true,
      false,
      false);

  if (PluginFilesFound.Num() > 0)
  {
    return PluginFilesFound[0u];
  }

  return FString{};
}

FString UOpenDrive::GetXODR(const UWorld *World)
{
  auto MapName = World->GetMapName();

  #if WITH_EDITOR
  {
    FString CorrectedMapName = MapName;
    constexpr auto PIEPrefix = TEXT("UEDPIE_0_");
    CorrectedMapName.RemoveFromStart(PIEPrefix);
    MapName = CorrectedMapName;
  }
  #endif // WITH_EDITOR

  ACarlaGameModeBase* GameMode = UCarlaStatics::GetGameMode(World);
  auto MapDir = GameMode->GetFullMapPath();
  const auto FolderDir = MapDir + "/OpenDrive/";
  const auto FileName = MapDir.EndsWith(MapName) ? "*" : MapName;

  TArray<FString> Files;

  // A generated OpenDRIVE world keeps its current xodr in Saved/; a stale
  // OpenDriveMap.xodr may still be cooked into the content dir.
  const FString SavedFilePath = GetSavedXODRPath(MapName);
  if (IFileManager::Get().FileExists(*SavedFilePath))
  {
    Files.Add(SavedFilePath);
  }

  if (!Files.Num())
  {
    IFileManager::Get().FindFilesRecursive(Files, *FolderDir, *FString(FileName + ".xodr"), true, false, false);
  }

  if (!Files.Num())
  {
    FString PluginFolder = FPaths::ProjectDir() + TEXT("Plugins/") + MapName + TEXT("/Content/Maps/OpenDrive/");
    IFileManager::Get().FindFilesRecursive(Files, *PluginFolder, *FString(FileName + ".xodr"), true, false, false);
  }

  if (!Files.Num())
  {
    // Content packs and any other root FindPathToXODRFile knows about.
    const FString FoundPath = FindPathToXODRFile(MapName);
    if (!FoundPath.IsEmpty())
    {
      Files.Add(FoundPath);
    }
  }

  FString Content;

  if (!Files.Num())
  {
    UE_LOG(LogCarla, Error, TEXT("No OpenDrive file found for map '%s'"), *MapName);
  }
  else if (FFileHelper::LoadFileToString(Content, *Files[0]))
  {
    UE_LOG(LogCarla, Log, TEXT("Loaded OpenDrive file '%s'"), *Files[0]);
  }
  else
  {
    UE_LOG(LogCarla, Error, TEXT("Failed to load OpenDrive file '%s'"), *Files[0]);
  }

  return Content;
}

FString UOpenDrive::LoadXODR(const FString &MapName)
{
  const auto FilePath = FindPathToXODRFile(MapName);

  FString Content;

  if (FilePath.IsEmpty())
  {
    UE_LOG(LogCarla, Error, TEXT("Failed to find OpenDrive file for map '%s'"), *MapName);
  }
  else if (FFileHelper::LoadFileToString(Content, *FilePath))
  {
    UE_LOG(LogCarla, Log, TEXT("Loaded OpenDrive file '%s'"), *FilePath);
  }
  else
  {
    UE_LOG(LogCarla, Error, TEXT("Failed to load OpenDrive file '%s'"), *FilePath);
  }

  return Content;
}

FString UOpenDrive::LoadXODRFullPath(const FString &FullPath)
{
  FString Content;

  if (FullPath.IsEmpty())
  {
    UE_LOG(LogCarla, Error, TEXT("Failed to find OpenDrive file for map '%s'"), *FullPath);
  }
  else if (FFileHelper::LoadFileToString(Content, *FullPath))
  {
    UE_LOG(LogCarla, Log, TEXT("Loaded OpenDrive file '%s'"), *FullPath);
  }
  else
  {
    UE_LOG(LogCarla, Error, TEXT("Failed to load OpenDrive file '%s'"), *FullPath);
  }

  return Content;
}

FString UOpenDrive::GetXODRByPath(FString XODRPath, FString MapName){

  // When playing in editor the map name gets an extra prefix, here we
  // remove it.
  #if WITH_EDITOR
  {
    FString CorrectedMapName = MapName;
    constexpr auto PIEPrefix = TEXT("UEDPIE_0_");
    CorrectedMapName.RemoveFromStart(PIEPrefix);
    MapName = CorrectedMapName;
  }
  #endif // WITH_EDITOR

  FString FileName = XODRPath.EndsWith(MapName) ? "*" : MapName;
  FString FolderDir = XODRPath;
  FolderDir.RemoveFromEnd(MapName + ".xodr");

  // Find all the xodr and bin files from the map
  TArray<FString> Files;
  IFileManager::Get().FindFilesRecursive(Files, *FolderDir, *FString(FileName + ".xodr"), true, false, false);

  FString Content;

  if (!Files.Num())
  {
    UE_LOG(LogTemp, Error, TEXT("Failed to find OpenDrive file for map '%s'"), *MapName);
  }
  else if (FFileHelper::LoadFileToString(Content, *Files[0]))
  {
    UE_LOG(LogTemp, Log, TEXT("Loaded OpenDrive file '%s'"), *Files[0]);
  }

  return Content;
}

UOpenDriveMap *UOpenDrive::LoadOpenDriveMap(const FString &MapName)
{
  UOpenDriveMap *Map = nullptr;
  auto XODRContent = LoadXODR(MapName);
  if (!XODRContent.IsEmpty())
  {
    Map = NewObject<UOpenDriveMap>();
    Map->Load(XODRContent);
  }
  return Map;
}

UOpenDriveMap *UOpenDrive::LoadCurrentOpenDriveMap(const UObject *WorldContextObject)
{
#if WITH_EDITOR

  if (WorldContextObject == nullptr)
    return nullptr;

  auto World = GEngine->GetWorldFromContextObject(
    WorldContextObject,
    EGetWorldErrorMode::LogAndReturnNull);

  if (World == nullptr)
    return nullptr;

  return LoadOpenDriveMap(World->GetMapName());

#else

  return nullptr;

#endif
}
