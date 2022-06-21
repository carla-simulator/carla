// Copyright (c) 2020 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "StaticMeshFactory.h"
#include "Carla/Actor/ActorBlueprintFunctionLibrary.h"
#include "Engine/StaticMeshActor.h"
#include "Carla/Game/CarlaEpisode.h"

TArray<FActorDefinition> AStaticMeshFactory::GetDefinitions()
{
  using ABFL = UActorBlueprintFunctionLibrary;
  auto StaticMeshDefinition = ABFL::MakeGenericDefinition(
      TEXT("static"),
      TEXT("prop"),
      TEXT("mesh"));
  StaticMeshDefinition.Class = AStaticMeshActor::StaticClass();
  return { StaticMeshDefinition };
}

FActorSpawnResult AStaticMeshFactory::SpawnActor(
    const FTransform &SpawnAtTransform,
    const FActorDescription &ActorDescription)
{
  using ABFL = UActorBlueprintFunctionLibrary;
  auto *World = GetWorld();
  if (World == nullptr)
  {
    UE_LOG(LogCarla, Error, TEXT
        ("AStaticMeshFactory: cannot spawn mesh into an empty world."));
    return {};
  }

  FActorSpawnParameters SpawnParameters;
  SpawnParameters.SpawnCollisionHandlingOverride =
      ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
  auto *StaticMeshActor = World->SpawnActor<AStaticMeshActor>(
      ActorDescription.Class, SpawnAtTransform, SpawnParameters);

  auto *StaticMeshComponent = Cast<UStaticMeshComponent>(
      StaticMeshActor->GetRootComponent());
  if (StaticMeshComponent)
  {
    if (ActorDescription.Variations.Contains("mesh_path") &&
        ActorDescription.Variations.Contains("mass"))
    {
      FString MeshPath = ABFL::ActorAttributeToString(
          ActorDescription.Variations["mesh_path"], "");
      UObject* MeshObject = StaticLoadObject(UStaticMesh::StaticClass(),
          nullptr,
          *(MeshPath));
      UStaticMesh *Mesh = Cast<UStaticMesh>(MeshObject);
      StaticMeshComponent->SetMobility(EComponentMobility::Movable);
      StaticMeshComponent->SetStaticMesh(Mesh);
      StaticMeshComponent->SetSimulatePhysics(true);
      StaticMeshComponent->SetCollisionProfileName("PhysicsActor");
      StaticMeshComponent->SetMassOverrideInKg("",
         ABFL::ActorAttributeToFloat(ActorDescription.Variations["mass"], 1.0f));
    }
  }
  return FActorSpawnResult(StaticMeshActor);
}
