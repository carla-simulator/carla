// Copyright (c) 2019 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "Carla.h"
#include "Carla/Util/NavigationMesh.h"

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

  FString FilePath =
      FPaths::ProjectContentDir() +
      TEXT("Carla/Maps/Nav/") +
      MapName + TEXT(".bin");

  auto &FileManager = IFileManager::Get();
  if (!FileManager.FileExists(*FilePath))
  {
    TArray<FString> FilesFound;
    FileManager.FindFilesRecursive(
      FilesFound,
      *FPaths::ProjectContentDir(),
      *(MapName + TEXT(".bin")),
      true,
      false,
      false);
    if (FilesFound.Num() > 0) 
      FilePath = FilesFound[0u];
  }

  TArray<uint8> Content;
  UE_LOG(LogCarla, Log, TEXT("Loading Navigation Mesh file '%s'"), *FilePath);

  if (!FFileHelper::LoadFileToArray(Content, *FilePath, 0))
  {
    UE_LOG(LogTemp, Error, TEXT("Failed to load Navigation Mesh file '%s'"), *FilePath);
  }

  return Content;
}
