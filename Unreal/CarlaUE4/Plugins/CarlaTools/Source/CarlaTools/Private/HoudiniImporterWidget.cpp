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

// Create sublevels for managing a large map.
// The implementation is currently empty and needs further development.
void UHoudiniImporterWidget::CreateSubLevels(ALargeMapManager* LargeMapManager)
{
}

// Move actors to specific sublevels based on their location in the large map.
void UHoudiniImporterWidget::MoveActorsToSubLevelWithLargeMap(TArray<AActor*> Actors, ALargeMapManager* LargeMapManager)
{
    TMap<FCarlaMapTile*, TArray<AActor*>> ActorsToMove;

    // Categorize actors by their respective map tiles.
    for (AActor* Actor : Actors)
    {
        if (!Actor) continue;

        // Check for hierarchical instanced mesh components and calculate location.
        UHierarchicalInstancedStaticMeshComponent* Component = Cast<UHierarchicalInstancedStaticMeshComponent>(
            Actor->GetComponentByClass(UHierarchicalInstancedStaticMeshComponent::StaticClass()));
        FVector ActorLocation = Actor->GetActorLocation();

        if (Component)
        {
            // Calculate average location for all instances in the mesh.
            ActorLocation = FVector(0);
            for (int32 i = 0; i < Component->GetInstanceCount(); ++i)
            {
                FTransform Transform;
                Component->GetInstanceTransform(i, Transform, true);
                ActorLocation += Transform.GetTranslation();
            }
            ActorLocation /= Component->GetInstanceCount();
        }

        // Log actor location and determine the corresponding map tile.
        UE_LOG(LogCarlaTools, Log, TEXT("Actor at location %s"), *ActorLocation.ToString());
        FCarlaMapTile* Tile = LargeMapManager->GetCarlaMapTile(ActorLocation);

        if (!Tile)
        {
            UE_LOG(LogCarlaTools, Error, TEXT("Error: actor %s in location %s is outside the map"), *Actor->GetName(), *ActorLocation.ToString());
            continue;
        }

        // Update actor coordinates based on tile origin.
        if (Component)
        {
            UpdateInstancedMeshCoordinates(Component, Tile->Location);
        }
        else
        {
            UpdateGenericActorCoordinates(Actor, Tile->Location);
        }

        // Add the actor to the corresponding tile's list.
        ActorsToMove.FindOrAdd(Tile).Add(Actor);
    }

    // Move categorized actors to their respective levels.
    for (auto& Element : ActorsToMove)
    {
        FCarlaMapTile* Tile = Element.Key;
        TArray<AActor*> ActorList = Element.Value;

        if (ActorList.Num() == 0) continue;

        UWorld* World = UEditorLevelLibrary::GetEditorWorld();
        ULevelStreamingDynamic* StreamingLevel = Tile->StreamingLevel;

        StreamingLevel->bShouldBlockOnLoad = true;
        StreamingLevel->SetShouldBeVisible(true);
        StreamingLevel->SetShouldBeLoaded(true);

        ULevelStreaming* Level = UEditorLevelUtils::AddLevelToWorld(
            World, *Tile->Name, ULevelStreamingDynamic::StaticClass(), FTransform());

        int MovedActors = UEditorLevelUtils::MoveActorsToLevel(ActorList, Level, false, false);

        UE_LOG(LogCarlaTools, Log, TEXT("Moved %d actors"), MovedActors);

        // Save and unload the level.
        FEditorFileUtils::SaveDirtyPackages(false, true, true, false, false, false, nullptr);
        UEditorLevelUtils::RemoveLevelFromWorld(Level->GetLoadedLevel());
    }

    // Perform garbage collection and cleanup.
    GEngine->PerformGarbageCollectionAndCleanupActors();
    FText TransResetText(FText::FromString("Clean up after Move actors to sublevels"));

    if (GEditor->Trans)
    {
        GEditor->Trans->Reset(TransResetText);
        GEditor->Cleanse(true, true, TransResetText);
    }
}

// Force the unloading of all streaming levels managed by the large map manager.
void UHoudiniImporterWidget::ForceStreamingLevelsToUnload(ALargeMapManager* LargeMapManager)
{
    UWorld* World = UEditorLevelLibrary::GetGameWorld();
    FIntVector NumTilesInXY = LargeMapManager->GetNumTilesInXY();

    for (int x = 0; x < NumTilesInXY.X; ++x)
    {
        for (int y = 0; y < NumTilesInXY.Y; ++y)
        {
            FIntVector CurrentTileVector(x, y, 0);
            FCarlaMapTile CarlaTile = LargeMapManager->GetCarlaMapTile(CurrentTileVector);

            ULevelStreamingDynamic* StreamingLevel = CarlaTile.StreamingLevel;
            ULevelStreaming* Level = UEditorLevelUtils::AddLevelToWorld(
                World, *CarlaTile.Name, ULevelStreamingDynamic::StaticClass(), FTransform());

            FEditorFileUtils::SaveDirtyPackages(false, true, true, false, false, false, nullptr);
            UEditorLevelUtils::RemoveLevelFromWorld(Level->GetLoadedLevel());
        }
    }
}

// Move a list of actors to a specified sublevel.
void UHoudiniImporterWidget::MoveActorsToSubLevel(TArray<AActor*> Actors, ULevelStreaming* Level)
{
    int MovedActors = UEditorLevelUtils::MoveActorsToLevel(Actors, Level, false, false);

    UE_LOG(LogCarlaTools, Log, TEXT("Moved %d actors"), MovedActors);

    FEditorFileUtils::SaveDirtyPackages(false, true, true, false, false, false, nullptr);
    UEditorLevelUtils::RemoveLevelFromWorld(Level->GetLoadedLevel());

    FText TransResetText(FText::FromString("Clean up after Move actors to sublevels"));
    if (GEditor->Trans)
    {
        GEditor->Trans->Reset(TransResetText);
    }
}

// Update actor coordinates to be relative to the given tile origin.
void UHoudiniImporterWidget::UpdateGenericActorCoordinates(AActor* Actor, FVector TileOrigin)
{
    FVector LocalLocation = Actor->GetActorLocation() - TileOrigin;
    Actor->SetActorLocation(LocalLocation);

    UE_LOG(LogCarlaTools, Log, TEXT("%s New location %s"), *Actor->GetName(), *LocalLocation.ToString());
}

// Update the transforms of all instances in a hierarchical instanced mesh component.
void UHoudiniImporterWidget::UpdateInstancedMeshCoordinates(UHierarchicalInstancedStaticMeshComponent* Component, FVector TileOrigin)
{
    TArray<FTransform> NewTransforms;

    for (int32 i = 0; i < Component->GetInstanceCount(); ++i)
    {
        FTransform Transform;
        Component->GetInstanceTransform(i, Transform, true);
        Transform.AddToTranslation(-TileOrigin);
        NewTransforms.Add(Transform);

        UE_LOG(LogCarlaTools, Log, TEXT("New instance location %s"), *Transform.GetTranslation().ToString());
    }

    Component->BatchUpdateInstancesTransforms(0, NewTransforms, true, true, true);
}

// Set actor location to its center of mass for a given list of actors.
void UHoudiniImporterWidget::UseCOMasActorLocation(TArray<AActor*> Actors)
{
    for (AActor* Actor : Actors)
    {
        UPrimitiveComponent* Primitive = Cast<UPrimitiveComponent>(
            Actor->GetComponentByClass(UPrimitiveComponent::StaticClass()));

        if (Primitive)
        {
            FBodyInstance* BodyInstance = Primitive->GetBodyInstance();
            FVector CenterOfMass = BodyInstance->COMNudge;
            Actor->SetActorLocation(CenterOfMass);
        }
    }
}

// Extract the number of clusters based on actor naming convention.
bool UHoudiniImporterWidget::GetNumberOfClusters(TArray<AActor*> ActorList, int& OutNumClusters)
{
    for (AActor* Actor : ActorList)
    {
        FString ObjectName = UKismetSystemLibrary::GetObjectName(Actor);
        UE_LOG(LogCarlaTools, Log, TEXT("Searching in string %s"), *ObjectName);

        if (ObjectName.StartsWith("b"))
        {
            int Index = ObjectName.Find("of");
            if (Index == -1) continue;

            UE_LOG(LogCarlaTools, Log, TEXT("found 'of' substring at %d"), Index);
            FString NumClusterString = ObjectName.Mid(Index + 2, ObjectName.Len());
            OutNumClusters = FCString::Atoi(*NumClusterString);
            return true;
        }
    }

    UE_LOG(LogCarlaTools, Warning, TEXT("Num clusters not found"));
    OutNumClusters = -1;
    return false;
}
