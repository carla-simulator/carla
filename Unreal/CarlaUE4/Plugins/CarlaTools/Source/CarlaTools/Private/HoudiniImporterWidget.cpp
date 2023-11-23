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
#include "EditorLevelLibrary.h"
#include "Components/PrimitiveComponent.h"

void UHoudiniImporterWidget::CreateSubLevels(ALargeMapManager* LargeMapManager)
{

}

void UHoudiniImporterWidget::MoveActorsToSubLevelWithLargeMap(TArray<AActor*> Actors, ALargeMapManager* LargeMapManager)
{
  TMap<FCarlaMapTile*, TArray<AActor*>> ActorsToMove;
  for (AActor* Actor : Actors)
  {
    if (Actor == nullptr) {
      continue;
    }
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
      UE_LOG(LogCarlaTools, Error, TEXT("Error: actor %s in location %s is outside the map"),
          *Actor->GetName(),*ActorLocation.ToString());
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
    if(!ActorList.Num())
    {
      continue;
    }

    UWorld* World = UEditorLevelLibrary::GetEditorWorld();
    ULevelStreamingDynamic* StreamingLevel = Tile->StreamingLevel;
    StreamingLevel->bShouldBlockOnLoad = true;
    StreamingLevel->SetShouldBeVisible(true);
    StreamingLevel->SetShouldBeLoaded(true);
    ULevelStreaming* Level =
        UEditorLevelUtils::AddLevelToWorld(
        World, *Tile->Name, ULevelStreamingDynamic::StaticClass(), FTransform());
    int MovedActors = UEditorLevelUtils::MoveActorsToLevel(ActorList, Level, false, false);
    // StreamingLevel->SetShouldBeLoaded(false);
    UE_LOG(LogCarlaTools, Log, TEXT("Moved %d actors"), MovedActors);
    FEditorFileUtils::SaveDirtyPackages(false, true, true, false, false, false, nullptr);
    UEditorLevelUtils::RemoveLevelFromWorld(Level->GetLoadedLevel());
  }

  GEngine->PerformGarbageCollectionAndCleanupActors();
  FText TransResetText(FText::FromString("Clean up after Move actors to sublevels"));
  if ( GEditor->Trans )
  {
    GEditor->Trans->Reset(TransResetText);
    GEditor->Cleanse(true, true, TransResetText);
  }
}

void UHoudiniImporterWidget::ForceStreamingLevelsToUnload( ALargeMapManager* LargeMapManager )
{
  UWorld* World = UEditorLevelLibrary::GetGameWorld();

  FIntVector NumTilesInXY  = LargeMapManager->GetNumTilesInXY();

  for(int x = 0; x < NumTilesInXY.X; ++x)
  {
    for(int y = 0; y < NumTilesInXY.Y; ++y)
    {
      FIntVector CurrentTileVector(x, y, 0);
      FCarlaMapTile CarlaTile = LargeMapManager->GetCarlaMapTile(CurrentTileVector);
      ULevelStreamingDynamic* StreamingLevel = CarlaTile.StreamingLevel;
      ULevelStreaming* Level =
        UEditorLevelUtils::AddLevelToWorld(
        World, *CarlaTile.Name, ULevelStreamingDynamic::StaticClass(), FTransform());
      FEditorFileUtils::SaveDirtyPackages(false, true, true, false, false, false, nullptr);
      UEditorLevelUtils::RemoveLevelFromWorld(Level->GetLoadedLevel());
    }
  }

}

void UHoudiniImporterWidget::MoveActorsToSubLevel(TArray<AActor*> Actors, ULevelStreaming* Level)
{
  int MovedActors = UEditorLevelUtils::MoveActorsToLevel(Actors, Level, false, false);

  UE_LOG(LogCarlaTools, Log, TEXT("Moved %d actors"), MovedActors);
  FEditorFileUtils::SaveDirtyPackages(false, true, true, false, false, false, nullptr);
  UEditorLevelUtils::RemoveLevelFromWorld(Level->GetLoadedLevel());
  FText TransResetText(FText::FromString("Clean up after Move actors to sublevels"));
  if ( GEditor->Trans )
  {
    GEditor->Trans->Reset(TransResetText);
  }
}

void UHoudiniImporterWidget::UpdateGenericActorCoordinates(
    AActor* Actor, FVector TileOrigin)
{
  FVector LocalLocation = Actor->GetActorLocation() - TileOrigin;
  Actor->SetActorLocation(LocalLocation);
  UE_LOG(LogCarlaTools, Log, TEXT("%s New location %s"),
    *Actor->GetName(), *LocalLocation.ToString());
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

void UHoudiniImporterWidget::UseCOMasActorLocation(TArray<AActor*> Actors)
{
  for (AActor* Actor : Actors)
  {
    UPrimitiveComponent* Primitive = Cast<UPrimitiveComponent>(
        Actor->GetComponentByClass(UPrimitiveComponent::StaticClass()));
    if(Primitive)
    {
      FBodyInstance* BodyInstance = Primitive->GetBodyInstance();
      FVector CenterOfMass = BodyInstance->COMNudge;
      Actor->SetActorLocation(CenterOfMass);
    }
  }
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
