// CARLA, Copyright (C) 2017 Computer Vision Center (CVC)

#include "Carla.h"
#include "StaticMeshCollection.h"

#include "Components/InstancedStaticMeshComponent.h"
#include "Engine/StaticMesh.h"

AStaticMeshCollection::AStaticMeshCollection(
  const FObjectInitializer& ObjectInitializer) :
  Super(ObjectInitializer)
{
  PrimaryActorTick.bCanEverTick = false;
  RootComponent =
      ObjectInitializer.CreateDefaultSubobject<USceneComponent>(this, TEXT("SceneComponent"));
  RootComponent->SetMobility(EComponentMobility::Static);
}

void AStaticMeshCollection::PushBackInstantiator(UStaticMesh *Mesh)
{
  auto Instantiator = NewObject<UInstancedStaticMeshComponent>(this);
  check(Instantiator != nullptr);
  Instantiator->SetMobility(EComponentMobility::Static);
  Instantiator->SetupAttachment(RootComponent);
  Instantiator->SetStaticMesh(Mesh);
  Instantiator->RegisterComponent();
  MeshInstatiators.Add(Instantiator);
}

void AStaticMeshCollection::SetStaticMesh(uint32 i, UStaticMesh *Mesh)
{
  check(GetNumberOfInstantiators() > i);
  MeshInstatiators[i]->SetStaticMesh(Mesh);
}

void AStaticMeshCollection::AddInstance(uint32 i, const FTransform &Transform)
{
  check(GetNumberOfInstantiators() > i);
  MeshInstatiators[i]->AddInstance(Transform);
}

void AStaticMeshCollection::ClearInstances()
{
  for (auto *Instantiator : MeshInstatiators) {
    Instantiator->ClearInstances();
  }
}

void AStaticMeshCollection::ClearInstantiators()
{
  ClearInstances();
  MeshInstatiators.Empty();
}
