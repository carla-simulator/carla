// CARLA, Copyright (C) 2017 Computer Vision Center (CVC)

#include "Carla.h"
#include "CityMapMeshHolder.h"

#include "Components/InstancedStaticMeshComponent.h"
#include "Engine/StaticMesh.h"

#include <vector>

using tag_size_t = std::underlying_type<ECityMapMeshTag>::type;

constexpr static tag_size_t NUMBER_OF_TAGS = CityMapMeshTag::GetNumberOfTags();

// =============================================================================
// -- Constructor --------------------------------------------------------------
// =============================================================================

ACityMapMeshHolder::ACityMapMeshHolder(const FObjectInitializer& ObjectInitializer)
  : Super(ObjectInitializer)
{
  PrimaryActorTick.bCanEverTick = false;

  SceneRootComponent =
      ObjectInitializer.CreateDefaultSubobject<USceneComponent>(this, TEXT("SceneComponent"));
  SceneRootComponent->SetMobility(EComponentMobility::Static);
  RootComponent = SceneRootComponent;

  for (tag_size_t i = 0u; i < NUMBER_OF_TAGS; ++i) {
    // Add static mesh holder.
    StaticMeshes.Add(CityMapMeshTag::FromUInt(i));
    // Create an instantiator for each mesh.
    const FString name = CityMapMeshTag::ToString(i) + "Instantiator";
    auto instantiator = CreateDefaultSubobject<UInstancedStaticMeshComponent>(*name);
    instantiator->SetMobility(EComponentMobility::Static);
    instantiator->SetupAttachment(SceneRootComponent);
    MeshInstatiators.Add(instantiator);
    instantiator->RegisterComponent();
  }
}

// =============================================================================
// -- Public methods -----------------------------------------------------------
// =============================================================================

FVector ACityMapMeshHolder::GetTileLocation(uint32 X, uint32 Y) const
{
  return {X * MapScale, Y * MapScale, 0.0f};
}

// =============================================================================
// -- Protected methods --------------------------------------------------------
// =============================================================================

#if WITH_EDITOR
void ACityMapMeshHolder::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
  Super::PostEditChangeProperty(PropertyChangedEvent);
  if (PropertyChangedEvent.Property) {
    ResetInstantiators();
    UpdateMapScale();
  }
}
#endif

void ACityMapMeshHolder::SetStaticMesh(ECityMapMeshTag Tag, UStaticMesh *Mesh)
{
  StaticMeshes[Tag] = Mesh;
}

const UStaticMesh *ACityMapMeshHolder::GetStaticMesh(ECityMapMeshTag Tag) const
{
  return StaticMeshes[Tag];
}

void ACityMapMeshHolder::AddInstance(ECityMapMeshTag Tag, uint32 X, uint32 Y)
{
  AddInstance(Tag, FTransform(GetTileLocation(X, Y)));
}

void ACityMapMeshHolder::AddInstance(ECityMapMeshTag Tag, uint32 X, uint32 Y, float Angle)
{
  const FQuat rotation(FVector(0.0f, 0.0f, 1.0f), Angle);
  const FVector location = GetTileLocation(X, Y);
  AddInstance(Tag, FTransform(rotation, location));

}

void ACityMapMeshHolder::AddInstance(ECityMapMeshTag Tag, FTransform Transform)
{
  auto instantiator = MeshInstatiators[CityMapMeshTag::ToUInt(Tag)];
  check(instantiator != nullptr);
  instantiator->AddInstance(Transform);
}

// =============================================================================
// -- Private methods ----------------------------------------------------------
// =============================================================================

void ACityMapMeshHolder::ResetInstantiators()
{
  for (tag_size_t i = 0u; i < NUMBER_OF_TAGS; ++i) {
    UInstancedStaticMeshComponent *instantiator = MeshInstatiators[i];
    check(instantiator != nullptr);
    instantiator->ClearInstances();
    instantiator->SetStaticMesh(StaticMeshes[CityMapMeshTag::FromUInt(i)]);
  }
}

void ACityMapMeshHolder::UpdateMapScale()
{
  auto Tag = CityMapMeshTag::GetBaseMeshTag();
  auto *mesh = GetStaticMesh(Tag);
  if (mesh == nullptr) {
    UE_LOG(
        LogCarla,
        Error,
        TEXT("Cannot find mesh \"%s\" for computing tile size"),
        *CityMapMeshTag::ToString(Tag));
    MapScale = 1.0f;
  } else {
    FVector size = mesh->GetBoundingBox().GetSize();
    if (size.X != size.Y) {
      UE_LOG(
          LogCarla,
          Warning,
          TEXT("Base mesh \"%s\" is not squared"),
          *CityMapMeshTag::ToString(Tag));
    }
    MapScale = size.X;
  }
}
