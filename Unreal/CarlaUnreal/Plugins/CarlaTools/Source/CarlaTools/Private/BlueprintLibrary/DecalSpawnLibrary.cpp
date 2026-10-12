// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "BlueprintLibrary/DecalSpawnLibrary.h"
#include "Carla/Traffic/RoutePlanner.h"

#include <util/ue-header-guard-begin.h>
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Engine/DecalActor.h"
#include "Components/DecalComponent.h"
#include <util/ue-header-guard-end.h>

static const FDecalSpawnParams *PickWeightedDecal(const TArray<FDecalSpawnParams> &Library, FRandomStream &Stream)
{
  float TotalWeight = 0.0f;
  for (const FDecalSpawnParams &Entry : Library)
  {
    TotalWeight += FMath::Max(Entry.Probability, 0.0f);
  }
  if (TotalWeight <= 0.0f)
  {
    return nullptr;
  }

  const float Roll = Stream.FRandRange(0.0f, TotalWeight);
  float Accumulated = 0.0f;
  for (const FDecalSpawnParams &Entry : Library)
  {
    Accumulated += FMath::Max(Entry.Probability, 0.0f);
    if (Roll <= Accumulated)
    {
      return &Entry;
    }
  }
  return &Library.Last();
}

int32 UDecalSpawnLibrary::ReplaceActorsWithRandomDecals(
    UObject *WorldContextObject,
    const TArray<AActor *> &ActorsToReplace,
    const TArray<FDecalSpawnParams> &DecalLibrary,
    FRandomStream RandomStream,
    float RoadFlowSearchRadius,
    FName SpawnedDecalTag)
{
  UWorld *World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::ReturnNull);
  if ((World == nullptr) || (DecalLibrary.Num() == 0))
  {
    return 0;
  }

  // Fixed snapshot taken up front: we only ever iterate the actors the
  // caller handed us, we never re-query the world mid-loop, so a decal
  // this call spawns can never be re-processed by this same call.
  const TArray<AActor *> Snapshot = ActorsToReplace;

  int32 SpawnedCount = 0;

  for (AActor *OldActor : Snapshot)
  {
    if (!IsValid(OldActor))
    {
      continue;
    }

    const FDecalSpawnParams *Chosen = PickWeightedDecal(DecalLibrary, RandomStream);
    if (Chosen == nullptr)
    {
      continue;
    }

    const FVector Location = OldActor->GetActorLocation();

    float YawDegrees = OldActor->GetActorRotation().Yaw;
    if (Chosen->bFollowRoadFlow)
    {
      FRotator RoadRotation;
      if (ARoutePlanner::FindRoadFlowRotationAtLocation(World, Location, RoadFlowSearchRadius, RoadRotation))
      {
        YawDegrees = RoadRotation.Yaw + Chosen->RoadFlowYawOffset;
      }
    }
    YawDegrees += RandomStream.FRandRange(-Chosen->RotationOffset, Chosen->RotationOffset);

    const FRotator Rotation(-90.0f, YawDegrees, 0.0f);

    // If OldActor is already a DecalActor, edit it in place instead of
    // destroying it and spawning a new one: same actor, same World
    // Partition external-actor package on disk, just resaved -- versus
    // destroy+spawn, which deletes one package and creates another. At
    // the scale of a full-map pass (thousands of decals) that difference
    // is the gap between a normal resave and an avalanche of paired
    // file deletions/creations that can bring the editor's save down.
    ADecalActor *TargetDecal = Cast<ADecalActor>(OldActor);
    if (TargetDecal == nullptr)
    {
      FActorSpawnParameters SpawnParams;
      SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
      TargetDecal = World->SpawnActor<ADecalActor>(ADecalActor::StaticClass(), Location, Rotation, SpawnParams);
    }
    else
    {
      // A freshly spawned actor is marked dirty by the editor automatically;
      // an existing one being edited in place is not -- plain C++ setters
      // don't dirty the package on their own, unlike going through the
      // editor's property panel. Without this, the edit applies in memory
      // (visible in the viewport) but "Save" has nothing to save, and the
      // change is lost on reopening the level.
      TargetDecal->Modify();
      TargetDecal->SetActorLocationAndRotation(Location, Rotation);
    }

    if (TargetDecal == nullptr)
    {
      continue;
    }

    UDecalComponent *DecalComponent = TargetDecal->GetDecal();
    if ((DecalComponent != nullptr) && (Chosen->Decal != nullptr))
    {
      DecalComponent->Modify();
      DecalComponent->SetDecalMaterial(Chosen->Decal);

      // Scale is always applied to the component's default size, never
      // multiplied onto whatever size it already had -- an in-place edit
      // can run on the same decal more than once, and multiplying onto
      // the current size each time would compound across runs.
      const float Scale = RandomStream.FRandRange(Chosen->MinScale, Chosen->MaxScale);
      DecalComponent->DecalSize = GetDefault<UDecalComponent>()->DecalSize * Scale;
    }

    TargetDecal->Tags.Reset();
    TargetDecal->Tags.Add(SpawnedDecalTag);
    TargetDecal->Tags.Add(FName(*StaticEnum<EDecalType>()->GetNameStringByValue(static_cast<int64>(Chosen->DecalType))));

    if (TargetDecal != OldActor)
    {
      OldActor->Destroy();
    }
    ++SpawnedCount;
  }

  return SpawnedCount;
}
