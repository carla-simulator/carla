// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "HoudiniImporterWidget.h"
#include "CarlaTools.h"

#include <util/ue-header-guard-begin.h>
#include "EditorLevelUtils.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"
#include "FileHelpers.h"
#include "Components/PrimitiveComponent.h"
#include <util/ue-header-guard-end.h>

void UHoudiniImporterWidget::MoveActorsToSubLevel(TArray<AActor*> Actors, ULevelStreaming* Level)
{
  int MovedActors = UEditorLevelUtils::MoveActorsToLevel(Actors, Level, false, false);

  UE_LOG(LogCarlaTools, Log, TEXT("Moved %d actors"), MovedActors);
  FEditorFileUtils::SaveDirtyPackages(false, true, true, false, false, false, nullptr);
  UEditorLevelUtils::RemoveLevelFromWorld(Level->GetLoadedLevel());
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
