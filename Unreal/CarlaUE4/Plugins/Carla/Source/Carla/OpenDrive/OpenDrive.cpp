// Copyright (c) 2019 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "Carla.h"
#include "Carla/OpenDrive/OpenDrive.h"

#include "Runtime/Core/Public/HAL/FileManagerGeneric.h"


FString UOpenDrive::GetXODR(const UWorld *World)
{
  const auto mapName = World->GetMapName();
  const auto mapDir = FPaths::GetPath(UCarlaStatics::GetGameInstance(World)->GetMapPath());
  const auto folderDir = mapDir + "/OpenDrive/";
  const auto fileName = mapDir.EndsWith(mapName) ? "*" : mapName;

  // Find all the xodr and bin files from the map
  TArray<FString> Files;
  IFileManager::Get().FindFilesRecursive(Files, *folderDir, *FString(fileName + ".xodr"), true, false, false);

  FString Content;

  if (!Files.Num())
  {
    UE_LOG(LogTemp, Error, TEXT("Failed to find OpenDrive file for map '%s'"), *mapName);
  }
  else if (FFileHelper::LoadFileToString(Content, *Files[0]))
  {
    UE_LOG(LogTemp, Log, TEXT("Loaded OpenDrive file '%s'"), *Files[0]);
  }
  else
  {
    UE_LOG(LogTemp, Error, TEXT("Failed to load OpenDrive file '%s'"), *Files[0]);
  }

  return Content;
}

UOpenDriveMap *UOpenDrive::LoadOpenDriveMap(const UWorld *World)
{
  UOpenDriveMap *Map = nullptr;
  auto XODRContent = GetXODR(World);
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
  return World != nullptr ? LoadOpenDriveMap(World) : nullptr;
}
