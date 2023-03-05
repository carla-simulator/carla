// Copyright (c) 2023 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "CoreMinimal.h"
#include "EditorUtilityWidget.h"
#include "Carla/MapGen/LargeMapManager.h"
#include "Components/HierarchicalInstancedStaticMeshComponent.h"
#include "Engine/LevelStreaming.h"

#include "HoudiniImporterWidget.generated.h"

UCLASS(BlueprintType)
class CARLATOOLS_API UHoudiniImporterWidget : public UEditorUtilityWidget
{
  GENERATED_BODY()

  UFUNCTION(BlueprintCallable, Category="HoudiniImporterWidget")
  void CreateSubLevels(ALargeMapManager* LargeMapManager);

  UFUNCTION(BlueprintCallable, Category="HoudiniImporterWidget")
  void MoveActorsToSubLevel(TArray<AActor*> Actors, ALargeMapManager* LargeMapManager);

  void UpdateGenericActorCoordinates(AActor* Actor, FVector TileOrigin);

  void UpdateInstancedMeshCoordinates(
      UHierarchicalInstancedStaticMeshComponent* Component, FVector TileOrigin);
  
  // Gets the total number of cluster from the actor name following the following scheme
  // b{builsing}c{cluster}of{clustersize}
  UFUNCTION(BlueprintCallable, Category="HoudiniImporterWidget")
  bool GetNumberOfClusters(TArray<AActor*> ActorList, int& OutNumClusters);

};
