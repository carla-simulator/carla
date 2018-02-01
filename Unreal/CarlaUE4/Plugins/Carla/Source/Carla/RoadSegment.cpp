// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "Carla.h"
#include "RoadSegment.h"

#include "Engine/StaticMesh.h"

enum RoadSegmentItems {
  ELaneLeft,
  ELaneRight,
  ESidewalkLeft,
  ESidewalkRight,
  ELaneMarkingSolid,
  ELaneMarkingBroken,
  NUMBER_OF_ITEMS
};

ARoadSegment::ARoadSegment(const FObjectInitializer& ObjectInitializer) :
  Super(ObjectInitializer)
{
  PrimaryActorTick.bCanEverTick = false;
}

void ARoadSegment::OnConstruction(const FTransform &Transform)
{
  Super::OnConstruction(Transform);
  UpdateMeshes();
}

#if WITH_EDITOR
void ARoadSegment::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
  Super::PostEditChangeProperty(PropertyChangedEvent);
  if (PropertyChangedEvent.Property) {
    GenerateRoad();
  }
}
#endif // WITH_EDITOR

void ARoadSegment::GenerateRoad()
{
  UpdateMeshes();
  UpdateRoad();
}

void ARoadSegment::UpdateMeshes()
{
  if (GetNumberOfInstantiators() != NUMBER_OF_ITEMS) {
    ClearInstantiators();
    for (auto i = 0u; i < NUMBER_OF_ITEMS; ++i) {
      PushBackInstantiator(nullptr);
    }
  }
  SetStaticMesh(ELaneLeft, LaneLeft);
  SetStaticMesh(ELaneRight, LaneRight);
  SetStaticMesh(ESidewalkLeft, SidewalkLeft);
  SetStaticMesh(ESidewalkRight, SidewalkRight);
  SetStaticMesh(ELaneMarkingSolid, LaneMarkingSolid);
  SetStaticMesh(ELaneMarkingBroken, LaneMarkingBroken);
}

void ARoadSegment::UpdateRoad()
{
  ClearInstances();
  Scale = (LaneLeft != nullptr ? LaneLeft->GetBoundingBox().GetSize().X : 1.0f);
  FVector Translation(0.0f, 0.0f, 0.0f);
  for (auto &Item : RoadDescription) {
    FTransform Transform{Translation};
    AddInstance(ELaneLeft, Transform);
    AddInstance(ELaneRight, Transform);
    if (Item.bHasRightSidewalk) {
      AddInstance(ESidewalkRight, Transform);
    }
    if (Item.bHasLeftSidewalk) {
      AddInstance(ESidewalkLeft, Transform);
    }
    if (Item.LaneMarking == ELaneMarkingType::Solid) {
      AddInstance(ELaneMarkingSolid, Transform);
    } else if (Item.LaneMarking == ELaneMarkingType::Broken) {
      AddInstance(ELaneMarkingBroken, Transform);
    }
    Translation.X += Scale;
  }
}
