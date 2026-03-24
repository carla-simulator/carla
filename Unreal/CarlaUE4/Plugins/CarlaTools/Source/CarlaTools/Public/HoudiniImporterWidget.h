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
public:
  UFUNCTION(BlueprintCallable, Category="HoudiniImporterWidget")
  static void CreateSubLevels(ALargeMapManager* LargeMapManager);

  UFUNCTION(BlueprintCallable, Category="HoudiniImporterWidget")
  static void MoveActorsToSubLevelWithLargeMap(TArray<AActor*> Actors, ALargeMapManager* LargeMapManager);

  UFUNCTION(BlueprintCallable, Category="HoudiniImporterWidget")
  static void ForceStreamingLevelsToUnload( ALargeMapManager* LargeMapManager );

  UFUNCTION(BlueprintCallable, Category="HoudiniImporterWidget")
  static void MoveActorsToSubLevel(TArray<AActor*> Actors, ULevelStreaming* Level);

  UFUNCTION(BlueprintCallable, Category="HoudiniImporterWidget")
  static void UpdateGenericActorCoordinates(AActor* Actor, FVector TileOrigin);

  static void UpdateInstancedMeshCoordinates(
      UHierarchicalInstancedStaticMeshComponent* Component, FVector TileOrigin);

  UFUNCTION(BlueprintCallable, Category="HoudiniImporterWidget")
  static void UseCOMasActorLocation(TArray<AActor*> Actors);

  // Gets the total number of cluster from the actor name following the following scheme
  // b{builsing}c{cluster}of{clustersize}
  UFUNCTION(BlueprintCallable, Category="HoudiniImporterWidget")
  static bool GetNumberOfClusters(TArray<AActor*> ActorList, int& OutNumClusters);

};
