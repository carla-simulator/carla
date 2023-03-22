// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB). This work is licensed under the terms of the MIT license. For a copy, see <https://opensource.org/licenses/MIT>.


#include "ProceduralBuildingUtilities.h"

#include "AssetRegistryModule.h"
#include "Components/StaticMeshComponent.h"
#include "FileHelpers.h"
#include "GameFramework/Actor.h"
#include "IMeshMergeUtilities.h"
#include "MeshMergeModule.h"
#include "UObject/Class.h"

void AProceduralBuildingUtilities::GenerateImpostor()
{
  // Not implemented yet.
}

void AProceduralBuildingUtilities::CookProceduralBuildingToMesh(const FString& DestinationPath, const FString& FileName)
{
  TArray<UPrimitiveComponent*> Components;
  TArray<UStaticMeshComponent*> StaticMeshComponents;
  GetComponents<UStaticMeshComponent>(StaticMeshComponents, false); // Components of class

  for(UStaticMeshComponent* StaticMeshComponent : StaticMeshComponents)
  {
    if(StaticMeshComponent->GetStaticMesh())
    {
      Components.Add(StaticMeshComponent);
    }
  }

  UWorld* World = GetWorld();

  FMeshMergingSettings MeshMergeSettings;
  TArray<UObject*> AssetsToSync;

  FVector NewLocation;
  const float ScreenAreaSize = TNumericLimits<float>::Max();

  FString PackageName = DestinationPath + FileName;
  UPackage* NewPackage = CreatePackage(*PackageName);

  const IMeshMergeUtilities& MeshUtilities = FModuleManager::Get().LoadModuleChecked<IMeshMergeModule>("MeshMergeUtilities").GetUtilities();
  MeshUtilities.MergeComponentsToStaticMesh(
      Components,
      World,
      MeshMergeSettings,
      nullptr,
      NewPackage,
      FileName,
      AssetsToSync,
      NewLocation,
      ScreenAreaSize,
      true);

  UPackage::SavePackage(NewPackage,
      AssetsToSync[0],
      EObjectFlags::RF_Public | EObjectFlags::RF_Standalone,
      *FileName,
      GError,
      nullptr,
      true,
      true,
      SAVE_NoError);
}
