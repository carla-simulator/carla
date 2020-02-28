// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "Carla.h"
#include "Carla/Game/CarlaStatics.h"

#include "Runtime/Core/Public/HAL/FileManagerGeneric.h"

TArray<FString> UCarlaStatics::GetAllMapNames()
{
  TArray<FString> TmpStrList, MapNameList;
  IFileManager::Get().FindFilesRecursive(
      MapNameList, *FPaths::ProjectContentDir(), TEXT("*.umap"), true, false, false);
  MapNameList.RemoveAll( [](const FString &Name) { return Name.Contains("TestMaps");});
  MapNameList.RemoveAll( [](const FString &Name) { return Name.Contains("OpenDriveMap");});
  for (int i = 0; i < MapNameList.Num(); i++) {
      MapNameList[i].ParseIntoArray(TmpStrList, TEXT("Content/"), true);
      MapNameList[i] = TmpStrList[1];
      MapNameList[i] = MapNameList[i].Replace(TEXT(".umap"), TEXT(""));
      MapNameList[i] = "/Game/" + MapNameList[i];
  }
  return MapNameList;
}
