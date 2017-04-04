// CARLA, Copyright (C) 2017 Computer Vision Center (CVC)

#include "Carla.h"
#include "CityMapMeshHolder.h"

#include "Components/InstancedStaticMeshComponent.h"
#include "Engine/StaticMesh.h"

#include <vector>

using tag_size_t = std::underlying_type<ECityMapMeshTag>::type;

constexpr static tag_size_t NUMBER_OF_TAGS = CityMapMeshTag::GetNumberOfTags();

// =============================================================================
// -- Construction and update related methods ----------------------------------
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
  }
}

void ACityMapMeshHolder::OnConstruction(const FTransform &Transform)
{
  Super::OnConstruction(Transform);

  if (MeshInstatiators.Num() == 0) {
    ResetInstantiators();
    UpdateMapScale();
    UpdateMap();
  }
}

#if WITH_EDITOR
void ACityMapMeshHolder::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
  Super::PostEditChangeProperty(PropertyChangedEvent);
  if (PropertyChangedEvent.Property) {
    ResetInstantiators();
    UpdateMapScale();
    UpdateMap();
  }
}
#endif // WITH_EDITOR

// =============================================================================
// -- Other protected methods --------------------------------------------------
// =============================================================================

FVector ACityMapMeshHolder::GetTileLocation(uint32 X, uint32 Y) const
{
  return {X * MapScale, Y * MapScale, 0.0f};
}

void ACityMapMeshHolder::SetStaticMesh(ECityMapMeshTag Tag, UStaticMesh *Mesh)
{
  StaticMeshes[Tag] = Mesh;
}

UStaticMesh *ACityMapMeshHolder::GetStaticMesh(ECityMapMeshTag Tag)
{
  return StaticMeshes[Tag];
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
  auto &instantiator = GetInstantiator(Tag);
  instantiator.AddInstance(Transform);
}

// =============================================================================
// -- Private methods ----------------------------------------------------------
// =============================================================================

void ACityMapMeshHolder::UpdateMap() {}

void ACityMapMeshHolder::ResetInstantiators()
{
  for (auto *instantiator : MeshInstatiators) {
    if (instantiator != nullptr) {
      instantiator->ClearInstances();
    }
  }
  if (MeshInstatiators.Num() != NUMBER_OF_TAGS) {
    MeshInstatiators.Empty();
    MeshInstatiators.Init(nullptr, NUMBER_OF_TAGS);
  }
  check(MeshInstatiators.Num() == NUMBER_OF_TAGS);
  for (tag_size_t i = 0u; i < NUMBER_OF_TAGS; ++i) {
    auto &instantiator = GetInstantiator(CityMapMeshTag::FromUInt(i));
    instantiator.SetStaticMesh(GetStaticMesh(CityMapMeshTag::FromUInt(i)));
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

UInstancedStaticMeshComponent &ACityMapMeshHolder::GetInstantiator(ECityMapMeshTag Tag)
{
  UInstancedStaticMeshComponent *instantiator = MeshInstatiators[CityMapMeshTag::ToUInt(Tag)];
  if (instantiator == nullptr) {
    // Create and register an instantiator.
    instantiator = NewObject<UInstancedStaticMeshComponent>(this);
    instantiator->SetMobility(EComponentMobility::Static);
    instantiator->SetupAttachment(SceneRootComponent);
    instantiator->SetStaticMesh(GetStaticMesh(Tag));
    MeshInstatiators[CityMapMeshTag::ToUInt(Tag)] = instantiator;
    instantiator->RegisterComponent();
  }
  check(instantiator != nullptr);
  return *instantiator;
}
