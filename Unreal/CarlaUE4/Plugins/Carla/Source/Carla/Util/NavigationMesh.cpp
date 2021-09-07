// Copyright (c) 2019 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "Carla.h"
#include "Carla/Util/NavigationMesh.h"
#include "Misc/FileHelper.h"

TArray<uint8> FNavigationMesh::Load(FString MapName)
{
#if WITH_EDITOR
    {
      // When playing in editor the map name gets an extra prefix, here we
      // remove it.
      FString CorrectedMapName = MapName;
      constexpr auto PIEPrefix = TEXT("UEDPIE_0_");
      CorrectedMapName.RemoveFromStart(PIEPrefix);
      UE_LOG(LogCarla, Log, TEXT("FNavigationMesh: Corrected map name from %s to %s"), *MapName, *CorrectedMapName);
      MapName = CorrectedMapName;
    }
#endif // WITH_EDITOR

  const auto FileName = MapName + ".bin";

  TArray<FString> Files;
  IFileManager::Get().FindFilesRecursive(Files, *FPaths::ProjectContentDir(), *FileName, true, false, false);

  TArray<uint8> Content;

  if (!Files.Num())
  {
    UE_LOG(LogTemp, Error, TEXT("Failed to find OpenDrive file for map '%s'"), *MapName);
  }
  else if (FFileHelper::LoadFileToArray(Content, *Files[0], 0))
  {
    UE_LOG(LogCarla, Log, TEXT("Loading Navigation Mesh file '%s'"), *Files[0]);
  }
  else
  {
    UE_LOG(LogTemp, Error, TEXT("Failed to load Navigation Mesh file '%s'"), *Files[0]);
  }

  return Content;
}
