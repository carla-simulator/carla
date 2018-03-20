// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "Carla.h"
#include "RoadIntersection.h"

ARoadIntersection::ARoadIntersection(const FObjectInitializer& ObjectInitializer) :
  Super(ObjectInitializer)
{
  PrimaryActorTick.bCanEverTick = false;

  RootComponent =
      ObjectInitializer.CreateDefaultSubobject<USceneComponent>(this, TEXT("RootComponent"));
  RootComponent->SetMobility(EComponentMobility::Static);

#define CARLA_CREATE_STATIC_MESH_COMPONENT(Mesh) \
    { \
      auto Component = CreateDefaultSubobject<UStaticMeshComponent>(TEXT(#Mesh) TEXT("Component")); \
      Component->SetMobility(EComponentMobility::Static); \
      Component->SetupAttachment(RootComponent); \
      StaticMeshComponents.Add(Component); \
      StaticMeshes.Add(ERoadIntersectionItem:: Mesh, nullptr); \
    }
  CARLA_CREATE_STATIC_MESH_COMPONENT(Lane0)
  CARLA_CREATE_STATIC_MESH_COMPONENT(Lane1)
  CARLA_CREATE_STATIC_MESH_COMPONENT(Lane2)
  CARLA_CREATE_STATIC_MESH_COMPONENT(Lane3)
  CARLA_CREATE_STATIC_MESH_COMPONENT(Sidewalk0)
  CARLA_CREATE_STATIC_MESH_COMPONENT(Sidewalk1)
  CARLA_CREATE_STATIC_MESH_COMPONENT(Sidewalk2)
  CARLA_CREATE_STATIC_MESH_COMPONENT(Sidewalk3)
  CARLA_CREATE_STATIC_MESH_COMPONENT(LaneMarking)
#undef CARLA_CREATE_STATIC_MESH_COMPONENT
}

#if WITH_EDITOR
void ARoadIntersection::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
  Super::PostEditChangeProperty(PropertyChangedEvent);
  if (PropertyChangedEvent.Property) {
    UpdateMeshes();
  }
}
#endif // WITH_EDITOR

void ARoadIntersection::SetStaticMesh(ERoadIntersectionItem Item, UStaticMesh *StaticMesh)
{
  if (static_cast<uint8>(Item) < StaticMeshes.Num()) {
    StaticMeshes[Item] = StaticMesh;
  }
}

void ARoadIntersection::UpdateMeshes()
{
  check(StaticMeshes.Num() == StaticMeshComponents.Num());
  int32 i = 0;
  for (auto Item : StaticMeshes) {
    check(StaticMeshComponents[i] != nullptr);
    StaticMeshComponents[i]->SetStaticMesh(Item.Value);
    ++i;
  }
}
