// Copyright (c) 2024 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "StaticMeshFactory.h"
#include "Carla/Actor/ActorBlueprintFunctionLibrary.h"
#include "Carla/Game/CarlaEpisode.h"

#include <util/ue-header-guard-begin.h>
#include "Engine/StaticMeshActor.h"
#include <util/ue-header-guard-end.h>

TArray<FActorDefinition> AStaticMeshFactory::GetDefinitions()
{
  using ABFL = UActorBlueprintFunctionLibrary;
  auto StaticMeshDefinition = ABFL::MakeGenericDefinition(
      TEXT("static"),
      TEXT("prop"),
      TEXT("mesh"));
  StaticMeshDefinition.Class = AStaticMeshActor::StaticClass();
  StaticMeshDefinition.Variations.Emplace(FActorVariation{
      TEXT("mesh_path"),
      EActorAttributeType::String,
      {""}, false});
  StaticMeshDefinition.Variations.Emplace(FActorVariation{
      TEXT("mass"),
      EActorAttributeType::Float,
      {""}, false});
  StaticMeshDefinition.Variations.Emplace(FActorVariation{
      TEXT("scale"),
      EActorAttributeType::Float,
      {"1.0f"}, false});
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

  float Scale = ABFL::RetrieveActorAttributeToFloat("scale", ActorDescription.Variations, 1.0f);
  FTransform ScaledTransform(SpawnAtTransform);
  ScaledTransform.SetScale3D(FVector(Scale));

  FActorSpawnParameters SpawnParameters;
  SpawnParameters.SpawnCollisionHandlingOverride =
      ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
  auto *StaticMeshActor = World->SpawnActor<AStaticMeshActor>(
      ActorDescription.Class, ScaledTransform, SpawnParameters);

  auto *StaticMeshComponent = Cast<UStaticMeshComponent>(
      StaticMeshActor->GetRootComponent());
    
  if (StaticMeshComponent)
  {
    if (ActorDescription.Variations.Contains("mesh_path"))
    {
      FString MeshPath = ABFL::ActorAttributeToString(
          ActorDescription.Variations["mesh_path"], "");
      
      UStaticMesh *Mesh = LoadObject<UStaticMesh>(nullptr, *MeshPath);
      StaticMeshComponent->SetMobility(EComponentMobility::Movable);
      if (!StaticMeshComponent->SetStaticMesh(Mesh))
        UE_LOG(LogCarla, Warning, TEXT("Failed to set the mesh"));
      StaticMeshComponent->SetMobility(EComponentMobility::Static);

      if (ActorDescription.Variations.Contains("mass"))
      {
        float Mass = ABFL::RetrieveActorAttributeToFloat("mass", ActorDescription.Variations, 0.0f);
        if (Mass > 0)
        {
          StaticMeshComponent->SetMobility(EComponentMobility::Movable);
          StaticMeshComponent->SetSimulatePhysics(true);
          StaticMeshComponent->SetCollisionProfileName("PhysicsActor");
          StaticMeshComponent->SetMassOverrideInKg("", Mass);
        }
      }
    }
  }
  return FActorSpawnResult(StaticMeshActor);
}
