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
  auto MapName = World->GetMapName();

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

  const auto MapDir = FPaths::GetPath(UCarlaStatics::GetGameInstance(World)->GetMapPath());
  const auto FolderDir = MapDir + "/OpenDrive/";
  const auto FileName = MapDir.EndsWith(MapName) ? "*" : MapName;

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
