// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "Carla.h"
#include "Carla/Util/BoundingBoxCalculator.h"

#include "Carla/Traffic/TrafficSignBase.h"
#include "Carla/Vehicle/CarlaWheeledVehicle.h"

#include "Components/CapsuleComponent.h"
#include "GameFramework/Character.h"

#include "Rendering/SkeletalMeshRenderData.h"


static FBoundingBox ApplyTransformToBB(
    const FBoundingBox& InBoundingBox,
    const FTransform& Transform)
{
  const FRotator Rotation = Transform.GetRotation().Rotator();
  const FVector Translation = Transform.GetLocation();
  const FVector Scale = Transform.GetScale3D();

  FBoundingBox BoundingBox = InBoundingBox;
  BoundingBox.Origin *= Scale;
  BoundingBox.Origin = Rotation.RotateVector(BoundingBox.Origin) + Translation;
  BoundingBox.Extent *= Scale;
  BoundingBox.Rotation = Rotation;

  return BoundingBox;
}

FBoundingBox UBoundingBoxCalculator::GetActorBoundingBox(const AActor *Actor, uint8 InTagQueried)
{
  if (Actor != nullptr)
  {
    // Vehicle.
    auto Vehicle = Cast<ACarlaWheeledVehicle>(Actor);
    if (Vehicle != nullptr)
    {
      FVector Origin = Vehicle->GetVehicleBoundingBoxTransform().GetTranslation();
      FVector Extent = Vehicle->GetVehicleBoundingBoxExtent();
      return {Origin, Extent};
    }
    // Walker.
    auto Character = Cast<ACharacter>(Actor);
    if (Character != nullptr)
    {
      auto Capsule = Character->GetCapsuleComponent();
      if (Capsule != nullptr)
      {
        const auto Radius = Capsule->GetScaledCapsuleRadius();
        const auto HalfHeight = Capsule->GetScaledCapsuleHalfHeight();
        // Characters have the pivot point centered.
        FVector Origin = {0.0f, 0.0f, 0.0f};
        FVector Extent = {Radius, Radius, HalfHeight};
        return {Origin, Extent};
      }
    }
    // Traffic sign.
    auto TrafficSign = Cast<ATrafficSignBase>(Actor);
    if (TrafficSign != nullptr)
    {
      auto TriggerVolume = TrafficSign->GetTriggerVolume();
      if (TriggerVolume != nullptr)
      {
        FVector Origin = TriggerVolume->GetRelativeTransform().GetTranslation();
        FVector Extent = TriggerVolume->GetScaledBoxExtent();
        return {Origin, Extent};
      }
      else
      {
        UE_LOG(LogCarla, Warning, TEXT("Traffic sign missing trigger volume: %s"), *Actor->GetName());
        return {};
      }
    }


  }
  return {};
}

// TODO: update to calculate current animation pose
FBoundingBox UBoundingBoxCalculator::GetSkeletalMeshBoundingBox(const USkeletalMesh* SkeletalMesh)
{
  if(!SkeletalMesh)
  {
    UE_LOG(LogCarla, Error, TEXT("GetSkeletalMeshBoundingBox no SkeletalMesh"));
    return {};
  }

  // Get Vertex postion information from LOD 0 of the Skeletal Mesh
  FSkeletalMeshRenderData* SkeletalMeshRenderData = SkeletalMesh->GetResourceForRendering();
  FSkeletalMeshLODRenderData& LODRenderData = SkeletalMeshRenderData->LODRenderData[0];
  FStaticMeshVertexBuffers& StaticMeshVertexBuffers = LODRenderData.StaticVertexBuffers;
  FPositionVertexBuffer& FPositionVertexBuffer = StaticMeshVertexBuffers.PositionVertexBuffer;
  uint32 NumVertices = FPositionVertexBuffer.GetNumVertices();

  // Look for Skeletal Mesh bounds (vertex perfect)
  FVector MaxVertex(TNumericLimits<float>::Min());
  FVector MinVertex(TNumericLimits<float>::Max());
  for(uint32 i = 0; i < NumVertices; i++)
  {
    FVector& Pos = FPositionVertexBuffer.VertexPosition(i);
    MaxVertex.X = (Pos.X > MaxVertex.X) ? Pos.X : MaxVertex.X;
    MaxVertex.Y = (Pos.Y > MaxVertex.Y) ? Pos.Y : MaxVertex.Y;
    MaxVertex.Z = (Pos.Z > MaxVertex.Z) ? Pos.Z : MaxVertex.Z;
    MinVertex.X = (Pos.X < MinVertex.X) ? Pos.X : MinVertex.X;
    MinVertex.Y = (Pos.Y < MinVertex.Y) ? Pos.Y : MinVertex.Y;
    MinVertex.Z = (Pos.Z < MinVertex.Z) ? Pos.Z : MinVertex.Z;
  }

  // Calculate middle point
  FVector Origin (
    (MaxVertex.X + MinVertex.X) * 0.5f,
    (MaxVertex.Y + MinVertex.Y) * 0.5f,
    (MaxVertex.Z + MinVertex.Z) * 0.5f
  );

  // Calculate box extent
  FVector Extent (
    (MaxVertex.X - MinVertex.X) * 0.5f,
    (MaxVertex.Y - MinVertex.Y) * 0.5f,
    (MaxVertex.Z - MinVertex.Z) * 0.5f
  );

  return {Origin, Extent};
}

FBoundingBox UBoundingBoxCalculator::GetStaticMeshBoundingBox(const UStaticMesh* StaticMesh)
{
  if(!StaticMesh)
  {
    UE_LOG(LogCarla, Error, TEXT("GetStaticMeshBoundingBox no StaticMesh"));
    return {};
  }

  FBox Box = StaticMesh->GetBoundingBox();
  return {Box.GetCenter(), Box.GetExtent()};

}

void UBoundingBoxCalculator::GetISMBoundingBox(
    UInstancedStaticMeshComponent* ISMComp,
    TArray<FBoundingBox>& OutBoundingBox)
{
  if(!ISMComp)
  {
    UE_LOG(LogCarla, Error, TEXT("GetISMBoundingBox no ISMComp"));
    return;
  }

  const UStaticMesh *Mesh = ISMComp->GetStaticMesh();
  const FBoundingBox SMBoundingBox = GetStaticMeshBoundingBox(Mesh);

  if(SMBoundingBox.Extent.IsZero())
  {
    UE_LOG(LogCarla, Error, TEXT("%s has no SM assigned to the ISM"), *ISMComp->GetOwner()->GetName());
    return;
  }

  const TArray<FInstancedStaticMeshInstanceData>& PerInstanceSMData =  ISMComp->PerInstanceSMData;

  for(auto& InstSMIData : PerInstanceSMData)
  {
    const FTransform Transform = FTransform(InstSMIData.Transform);
    FBoundingBox BoundingBox = ApplyTransformToBB(SMBoundingBox, Transform);
    OutBoundingBox.Add(BoundingBox);
  }

}

// TODO: Add tag to the querry
// TODO: Dynamic vehicle, avoid SM of collision
TArray<FBoundingBox> UBoundingBoxCalculator::GetBoundingBoxOfActors(
  const TArray<AActor*>& Actors,
  uint8 InTagQueried)
{
  TArray<FBoundingBox> Result;
  ECityObjectLabel TagQueried = (ECityObjectLabel)InTagQueried;
  bool FilterByTagEnabled = (TagQueried != ECityObjectLabel::None);

  for(AActor* Actor : Actors)
  {
    const FTransform& ActorTransform = Actor->GetActorTransform();

    // Filter actors by tag
    //ATagger::GetTagOfTaggedComponent()
    //TSet<ECityObjectLabel> Tags;
    //ATagger::GetTagsOfTaggedActor(*Actor, Tags);
    //UE_LOG(LogCarla, Error, TEXT("%s %d"), *Actor->GetName(), Tags.Num() );
    //for(ECityObjectLabel Tag : Tags)
    //{
    //  UE_LOG(LogCarla, Error, TEXT(" - %s"), *ATagger::GetTagAsString(Tag));
    //}

    /*
    FBoundingBox BoundingBox = GetActorBoundingBox(Actor);
    if(!BoundingBox.Extent.IsZero())
    {
      BoundingBox = ApplyTransformToBB(BoundingBox, ActorTransform);
      //Result.Add(BoundingBox);
      continue;
    }
    */

    // Any other actor
    TArray<UMeshComponent*> MeshComps;
    Actor->GetComponents<UMeshComponent>(MeshComps);
    // FVector WorldLocation = Actor->GetActorLocation();
    // FVector WorldScale = Actor->GetActorScale();
    // FRotator WorldRotation = Actor->GetActorRotation();

    // Find if there is some geometry component
    TArray<UStaticMeshComponent*> StaticMeshComps;
    TArray<USkeletalMeshComponent*> SkeletalMeshComps;
    TArray<UInstancedStaticMeshComponent *> ISMComps;
    Actor->GetComponents<UStaticMeshComponent>(StaticMeshComps);
    Actor->GetComponents<USkeletalMeshComponent>(SkeletalMeshComps);
    Actor->GetComponents<UInstancedStaticMeshComponent>(ISMComps);

    // Calculate FBoundingBox of SM
    for(UStaticMeshComponent* StaticMeshComp : StaticMeshComps)
    {
      // Filter by tag
      ECityObjectLabel Tag = ATagger::GetTagOfTaggedComponent(*StaticMeshComp);
      if(FilterByTagEnabled && Tag != TagQueried) continue;

      UStaticMesh* StaticMesh = StaticMeshComp->GetStaticMesh();
      FBoundingBox BoundingBox = GetStaticMeshBoundingBox(StaticMesh);

      if(BoundingBox.Extent.IsZero())
      {
        UE_LOG(LogCarla, Error, TEXT("%s has no SM assigned"), *Actor->GetName());
      }
      else
      {
        BoundingBox = ApplyTransformToBB(BoundingBox, ActorTransform);
        Result.Add(BoundingBox);
      }
    }

    // Calculate FBoundingBox of SK_M
    for(USkeletalMeshComponent* SkeletalMeshComp : SkeletalMeshComps)
    {
      // Filter by tag
      ECityObjectLabel Tag = ATagger::GetTagOfTaggedComponent(*SkeletalMeshComp);
      if(FilterByTagEnabled && Tag != TagQueried) continue;

      USkeletalMesh* SkeletalMesh = SkeletalMeshComp->SkeletalMesh;
      FBoundingBox BoundingBox = GetSkeletalMeshBoundingBox(SkeletalMesh);
      if(BoundingBox.Extent.IsZero())
      {
        UE_LOG(LogCarla, Error, TEXT("%s has no SKM assigned"), *Actor->GetName());
      }
      else
      {
        BoundingBox = ApplyTransformToBB(BoundingBox, ActorTransform);
        Result.Add(BoundingBox);
      }
    }

    // Calculate FBoundingBox of ISM
    for(UInstancedStaticMeshComponent* ISMComp: ISMComps)
    {
      // Filter by tag
      ECityObjectLabel Tag = ATagger::GetTagOfTaggedComponent(*ISMComp);
      UE_LOG(LogCarla, Error, TEXT("%s - %s"), *Actor->GetName(), *ATagger::GetTagAsString(Tag));
      if(FilterByTagEnabled && Tag != TagQueried) continue;

      GetISMBoundingBox(ISMComp, Result);
    }

  }

  return Result;
}
