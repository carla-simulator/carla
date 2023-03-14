// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB). This work is licensed under the terms of the MIT license. For a copy, see <https://opensource.org/licenses/MIT>.


#include "ProceduralBuildingUtilities.h"

#include "AssetRegistryModule.h"
#include "FileHelpers.h"
#include "IMeshMergeUtilities.h"
#include "MeshMergeModule.h"
#include "UObject/Class.h"

void AProceduralBuildingUtilities::GenerateImpostor()
{
  // Not implemented yet.
}

void AProceduralBuildingUtilities::CookProceduralBuildingToMesh(const FString& DestinationPath)
{
  TArray<UPrimitiveComponent*> Components;
  this->GetComponents(Components, false);

  UWorld* World = this->GetWorld();

  FMeshMergingSettings MeshMergeSettings;
  TArray<UObject*> AssetsToSync;

  FVector NewLocation;
  const float ScreenAreaSize = TNumericLimits<float>::Max();

  const IMeshMergeUtilities& MeshUtilities = FModuleManager::Get().LoadModuleChecked<IMeshMergeModule>("MeshMergeUtilities").GetUtilities();
  MeshUtilities.MergeComponentsToStaticMesh(Components, World, MeshMergeSettings, nullptr, nullptr, DestinationPath, AssetsToSync, NewLocation, ScreenAreaSize, true);

  TArray<UClass*> ClassesToSave = { UStaticMesh::StaticClass() };
  FEditorFileUtils::SaveDirtyContentPackages(ClassesToSave, false, false, true, false);
}
