// Copyright (c) 2023 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB). This work is licensed under the terms of the MIT license. For a copy, see <https://opensource.org/licenses/MIT>.

#include "USDImporterWidget.h"
#include "USDCARLAInterface.h"
#include "Kismet/GameplayStatics.h"
#include "Modules/ModuleManager.h"
#include "IMeshMergeUtilities.h"
#include "MeshMergeModule.h"
#include "Components/PrimitiveComponent.h"


void UUSDImporterWidget::ImportUSDProp(
    const FString& USDPath, const FString& DestinationAssetPath, bool bAsBlueprint)
{
  FUSDCARLAInterface::ImportUSD(USDPath, DestinationAssetPath, false, bAsBlueprint);
}

void UUSDImporterWidget::ImportUSDVehicle(
    const FString& USDPath, const FString& DestinationAssetPath, bool bAsBlueprint)
{

}

AActor* UUSDImporterWidget::GetGeneratedBlueprint(UWorld* World, const FString& USDPath)
{
  TArray<AActor*> Actors;
  UGameplayStatics::GetAllActorsOfClass(World, AActor::StaticClass(), Actors);
  FString USDFileName = FPaths::GetBaseFilename(USDPath, true);
  UE_LOG(LogCarlaTools, Log, TEXT("Searching for name %s"), *USDFileName);
  for (AActor* Actor : Actors)
  {
    if(Actor->GetName().Contains(USDFileName))
    {
      return Actor;
    }
  }
  return nullptr;
}

bool UUSDImporterWidget::MergeStaticMeshComponents(
    TArray<AActor*> Actors, const FString& DestMesh)
{
  if (Actors.Num() == 0)
  {
    UE_LOG(LogCarlaTools, Error, TEXT("No actors for merge"));
    return false;
  }
  UWorld* World = Actors[0]->GetWorld();
  const IMeshMergeUtilities& MeshUtilities = FModuleManager::Get().LoadModuleChecked<IMeshMergeModule>("MeshMergeUtilities").GetUtilities();
  TArray<UPrimitiveComponent*> ComponentsToMerge;
  for(AActor* Actor : Actors)
  {
    TArray<UPrimitiveComponent*> ActorComponents;
    Actor->GetComponents(ActorComponents, false);
    ComponentsToMerge.Append(ActorComponents);
  }
  FMeshMergingSettings MeshMergeSettings;
  TArray<UObject*> AssetsToSync;
  const float ScreenAreaSize = TNumericLimits<float>::Max();
  FVector NewLocation;
  MeshUtilities.MergeComponentsToStaticMesh(ComponentsToMerge, World, MeshMergeSettings, nullptr, nullptr, DestMesh, AssetsToSync, NewLocation, ScreenAreaSize, true);
  return true;
}
