// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "Carla.h"
#include "Carla/Util/OpenDrive.h"

FString FOpenDrive::Load(FString MapName)
{
#if WITH_EDITOR
    {
      // When playing in editor the map name gets an extra prefix, here we
      // remove it.
      FString CorrectedMapName = MapName;
      constexpr auto PIEPrefix = TEXT("UEDPIE_0_");
      CorrectedMapName.RemoveFromStart(PIEPrefix);
      UE_LOG(LogCarla, Log, TEXT("FOpenDrive: Corrected map name from %s to %s"), *MapName, *CorrectedMapName);
      MapName = CorrectedMapName;
    }
#endif // WITH_EDITOR

  const FString FilePath =
      FPaths::ProjectContentDir() +
      TEXT("Carla/Maps/OpenDrive/") +
      MapName +
      TEXT(".xodr");

  FString Content;
  UE_LOG(LogCarla, Log, TEXT("Loading OpenDrive file '%s'"), *FilePath);

  if (!FFileHelper::LoadFileToString(Content, *FilePath))
  {
    UE_LOG(LogTemp, Error, TEXT("Failed to load OpenDrive file '%s'"), *FilePath);
  }

  return Content;
}
