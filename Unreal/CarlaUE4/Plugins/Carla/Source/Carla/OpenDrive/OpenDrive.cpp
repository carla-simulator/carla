// Copyright (c) 2019 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "Carla.h"
#include "Carla/OpenDrive/OpenDrive.h"

#include "Runtime/Core/Public/HAL/FileManagerGeneric.h"

FString UOpenDrive::FindPathToXODRFile(const FString &InMapName)
{
  FString MapName = InMapName;
#if WITH_EDITOR
    {
      // When playing in editor the map name gets an extra prefix, here we
      // remove it.
      FString CorrectedMapName = MapName;
      constexpr auto PIEPrefix = TEXT("UEDPIE_0_");
      CorrectedMapName.RemoveFromStart(PIEPrefix);
      UE_LOG(LogCarla, Log, TEXT("UOpenDrive: Corrected map name from %s to %s"), *MapName, *CorrectedMapName);
      MapName = CorrectedMapName;
    }
#endif // WITH_EDITOR

  MapName += TEXT(".xodr");

  const FString DefaultFilePath =
      FPaths::ProjectContentDir() +
      TEXT("Carla/Maps/OpenDrive/") +
      MapName;

  auto &FileManager = IFileManager::Get();

  if (FileManager.FileExists(*DefaultFilePath))
  {
    return DefaultFilePath;
  }

  TArray<FString> FilesFound;
  FileManager.FindFilesRecursive(
      FilesFound,
      *FPaths::ProjectContentDir(),
      *MapName,
      true,
      false,
      false);

  return FilesFound.Num() > 0 ? FilesFound[0u] : FString{};
}

FString UOpenDrive::LoadXODR(const FString &MapName)
{
  const auto FilePath = FindPathToXODRFile(MapName);

  FString Content;

  if (FilePath.IsEmpty())
  {
    UE_LOG(LogTemp, Error, TEXT("Failed to find OpenDrive file for map '%s'"), *MapName);
  }
  else if (FFileHelper::LoadFileToString(Content, *FilePath))
  {
    UE_LOG(LogTemp, Log, TEXT("Loaded OpenDrive file '%s'"), *FilePath);
  }
  else
  {
    UE_LOG(LogTemp, Error, TEXT("Failed to load OpenDrive file '%s'"), *FilePath);
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
  UWorld *World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull);
  return World != nullptr ?
      LoadOpenDriveMap(World->GetMapName()) :
      nullptr;
}
