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

FBoundingBox UBoundingBoxCalculator::GetActorBoundingBox(const AActor *Actor)
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

// TODO: Dynamic vehicle, avoid SM of collision
TArray<FBoundingBox> UBoundingBoxCalculator::GetBoundingBoxOfActors(const TArray<AActor*>& Actors)
{
  TArray<FBoundingBox> Result;

  int ActorIndex = 0;
  for(AActor* Actor : Actors)
  {
    //UE_LOG(LogCarla, Warning, TEXT(" %d / %d"), ActorIndex, Actors.Num());
    ActorIndex++;

    AInstancedFoliageActor* InstancedFolliageActor = Cast<AInstancedFoliageActor>(Actor);
    if(InstancedFolliageActor != nullptr)
    {
      TMap<UFoliageType*, TUniqueObj<FFoliageInfo>>& FoliageInstancesMap = InstancedFolliageActor->FoliageInfos;


      UE_LOG(LogCarla, Warning, TEXT("FolliageActor with %d FoliageTypes"), FoliageInstancesMap.Num());

      int FoliageIndex = 0;
      for(auto& FoliagePair: FoliageInstancesMap)
      {
        const UFoliageType* FoliageType = FoliagePair.Key;
        const FFoliageInfo& FoliageInfo = FoliagePair.Value.Get();
        const UFoliageType_InstancedStaticMesh* FoliageType_ISM = Cast<UFoliageType_InstancedStaticMesh>(FoliageType);

        UHierarchicalInstancedStaticMeshComponent* HISMComp = FoliageInfo.GetComponent();

        if(!HISMComp)
        {
          continue;
        }

        UStaticMesh *Mesh = FoliageType_ISM->GetStaticMesh();
        FBoundingBox SMBoundingBox = GetStaticMeshBoundingBox(Mesh);

        int32 NumHISMInstances = HISMComp->GetNumRenderInstances();

        UE_LOG(LogCarla, Warning, TEXT("   %d/%d : NumHISMInstances = %d"),
          FoliageIndex, FoliageInstancesMap.Num(), NumHISMInstances);
        FoliageIndex++;

        const TArray<FInstancedStaticMeshInstanceData>& PerInstanceSMData =  HISMComp->PerInstanceSMData;

        for(auto& InstSMIData : PerInstanceSMData)
        {
          FTransform Transform = FTransform(InstSMIData.Transform);
          FRotator Rotation = Transform.GetRotation().Rotator();
          FVector Translation = Transform.GetLocation();
          FVector Scale = Transform.GetScale3D();

          FBoundingBox BoundingBox = SMBoundingBox;
          BoundingBox.Origin *= Scale;
          BoundingBox.Origin = Rotation.RotateVector(BoundingBox.Origin) + Translation;
          BoundingBox.Extent *= Scale;
          BoundingBox.Rotation = Rotation;
          Result.Add(BoundingBox);
        }
      }

    }
    else
    {
      TArray<UMeshComponent*> MeshComps;
      Actor->GetComponents<UMeshComponent>(MeshComps);

      FVector WorldLocation = Actor->GetActorLocation();
      FVector WorldScale = Actor->GetActorScale();
      FRotator WorldRotation = Actor->GetActorRotation();

      // Find if there is some geometry component
      TArray<UStaticMeshComponent*> StaticMeshComps;
      TArray<USkeletalMeshComponent*> SkeletalMeshComps;
      Actor->GetComponents<UStaticMeshComponent>(StaticMeshComps);
      Actor->GetComponents<USkeletalMeshComponent>(SkeletalMeshComps);

      // Calculate FBoundingBox of SM
      for(UStaticMeshComponent* StaticMeshComp : StaticMeshComps)
      {
        UStaticMesh* StaticMesh = StaticMeshComp->GetStaticMesh();
        FBoundingBox BoundingBox = GetStaticMeshBoundingBox(StaticMesh);

        if(BoundingBox.Extent.IsZero())
        {
          UE_LOG(LogCarla, Error, TEXT("%s has no SM assigned"), *Actor->GetName());
        }
        else
        {
          BoundingBox.Origin *= WorldScale;
          BoundingBox.Origin = WorldRotation.RotateVector(BoundingBox.Origin) + WorldLocation;
          BoundingBox.Extent *= WorldScale;
          BoundingBox.Rotation = WorldRotation;
          Result.Add(BoundingBox);
        }
      }

      // Calculate FBoundingBox of SK_M
      for(USkeletalMeshComponent* SkeletalMeshComp : SkeletalMeshComps)
      {
        USkeletalMesh* SkeletalMesh = SkeletalMeshComp->SkeletalMesh;
        FBoundingBox BoundingBox = GetSkeletalMeshBoundingBox(SkeletalMesh);
        BoundingBox.Origin *= WorldScale;
        BoundingBox.Origin = WorldRotation.RotateVector(BoundingBox.Origin) + WorldLocation;
        BoundingBox.Extent *= WorldScale;
        BoundingBox.Rotation = WorldRotation;
        Result.Add(BoundingBox);
      }
    }
  }

  return Result;
}