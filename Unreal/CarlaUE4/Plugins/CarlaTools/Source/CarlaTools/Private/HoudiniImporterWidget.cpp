// Copyright (c) 2023 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "HoudiniImporterWidget.h"

#include "EditorLevelUtils.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"
#include "FileHelpers.h"

void UHoudiniImporterWidget::CreateSubLevels(ALargeMapManager* LargeMapManager)
{

}

void UHoudiniImporterWidget::MoveActorsToSubLevel(TArray<AActor*> Actors, ALargeMapManager* LargeMapManager)
{
  TMap<FCarlaMapTile*, TArray<AActor*>> ActorsToMove;
  for (AActor* Actor : Actors)
  {
    UHierarchicalInstancedStaticMeshComponent* Component
        = Cast<UHierarchicalInstancedStaticMeshComponent>(
          Actor->GetComponentByClass(
          UHierarchicalInstancedStaticMeshComponent::StaticClass()));
    FVector ActorLocation = Actor->GetActorLocation();
    if (Component)
    {
      ActorLocation = FVector(0);
      for(int32 i = 0; i < Component->GetInstanceCount(); ++i)
      {
        FTransform Transform;
        Component->GetInstanceTransform(i, Transform, true);
        ActorLocation = ActorLocation + Transform.GetTranslation();
      }
      ActorLocation = ActorLocation / Component->GetInstanceCount();
    }
    UE_LOG(LogCarlaTools, Log, TEXT("Actor at location %s"),
          *ActorLocation.ToString());
    FCarlaMapTile* Tile = LargeMapManager->GetCarlaMapTile(ActorLocation);
    if(!Tile)
    {
      UE_LOG(LogCarlaTools, Error, TEXT("Error: actor in location %s is outside the map"),
          *ActorLocation.ToString());
      continue;
    }

    if(Component)
    {
      UpdateInstancedMeshCoordinates(Component, Tile->Location);
    }
    else
    {
      UpdateGenericActorCoordinates(Actor, Tile->Location);
    }
    ActorsToMove.FindOrAdd(Tile).Add(Actor);
  }

  for (auto& Element : ActorsToMove)
  {
    FCarlaMapTile* Tile = Element.Key;
    TArray<AActor*> ActorList = Element.Value;
    ULevelStreamingDynamic* StreamingLevel = Tile->StreamingLevel;
    UE_LOG(LogCarlaTools, Log, TEXT("Got Tile %s in location %s"),
        *StreamingLevel->PackageNameToLoad.ToString(), *Tile->Location.ToString());
    StreamingLevel->bShouldBlockOnLoad = true;
    StreamingLevel->SetShouldBeVisible(true);
    StreamingLevel->SetShouldBeLoaded(true);
    ULevelStreaming* Level = 
        UEditorLevelUtils::AddLevelToWorld(
        GetWorld(), *Tile->Name, ULevelStreamingDynamic::StaticClass(), FTransform());
        
    int MovedActors = UEditorLevelUtils::MoveActorsToLevel(ActorList, Level, false, false);
    // StreamingLevel->SetShouldBeLoaded(false);
    UE_LOG(LogCarlaTools, Log, TEXT("Moved %d actors"), MovedActors);
    FEditorFileUtils::SaveDirtyPackages(false, true, true, false, false, false, nullptr);
    UEditorLevelUtils::RemoveLevelFromWorld(Level->GetLoadedLevel());
  }
}

void UHoudiniImporterWidget::UpdateGenericActorCoordinates(
    AActor* Actor, FVector TileOrigin)
{
  FVector LocalLocation = Actor->GetActorLocation() - TileOrigin;
  Actor->SetActorLocation(LocalLocation);
  UE_LOG(LogCarlaTools, Log, TEXT("New location %s"),
    *LocalLocation.ToString());
}

void UHoudiniImporterWidget::UpdateInstancedMeshCoordinates(
    UHierarchicalInstancedStaticMeshComponent* Component,  FVector TileOrigin)
{
  TArray<FTransform> NewTransforms;
  for(int32 i = 0; i < Component->GetInstanceCount(); ++i)
  {
    FTransform Transform;
    Component->GetInstanceTransform(i, Transform, true);
    Transform.AddToTranslation(-TileOrigin);
    NewTransforms.Add(Transform);
    UE_LOG(LogCarlaTools, Log, TEXT("New instance location %s"),
        *Transform.GetTranslation().ToString());
  }
  Component->BatchUpdateInstancesTransforms(0, NewTransforms, true, true, true);
}

bool UHoudiniImporterWidget::GetNumberOfClusters(
    TArray<AActor*> ActorList, int& OutNumClusters)
{
  
  for (AActor* Actor : ActorList)
  {
    FString ObjectName = UKismetSystemLibrary::GetObjectName(Actor);
    UE_LOG(LogCarlaTools, Log, TEXT("Searching in string %s"), *ObjectName);
    if(ObjectName.StartsWith("b"))
    {
      int Index = ObjectName.Find("of");
      if(Index == -1)
      {
        continue;
      }
      UE_LOG(LogCarlaTools, Log, TEXT("found of substr at %d"), Index);
      FString NumClusterString = ObjectName.Mid(Index+2, ObjectName.Len());
      OutNumClusters = FCString::Atoi(*NumClusterString);
      return true;
    }
  }
  UE_LOG(LogCarlaTools, Warning, TEXT("Num clusters not found"));
  OutNumClusters = -1;
  return false;
}
