// Copyright (c) 2022 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "Components/ActorComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "SpringBasedVegetationComponent.generated.h"

struct FSkeletonBone
{
  float Mass = 1.f;
  float Length = 0.5f;
  FVector CenterOfMass = FVector(0,0,0);
};

struct FSkeletonJoint
{
  int JointId;
  int ParentId;
  FString JointName;
  bool bIsStatic = false;
  TArray<int> ChildrenIds;
  FTransform Transform; // relative to parent
  FRotator RestingAngles; // resting angle position of springs
  float Mass = 10.f;
  FRotator AngularVelocity = FRotator(0,0,0);
  FRotator AngularAcceleration = FRotator(0,0,0);
  FTransform GlobalTransform;
  FTransform GolbalInverseTransform;
  TArray<FSkeletonBone> Bones;
  FVector ExternalForces = FVector(0,0,0);
};

struct FSkeletonHierarchy
{
  TArray<FSkeletonJoint> Joints;
  TArray<int> EndJoints;
  TArray<int> EndToRootOrder;
  TArray<int> RootToEndOrder;
  void ComputeChildrenJointsAndBones();
  void ComputeEndJoints();
  void AddForce(const FString& BoneName, const FVector& Force);
  void ClearExternalForces();
};

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class CARLA_API USpringBasedVegetationComponent : public UActorComponent
{
  GENERATED_BODY()

public:

  USpringBasedVegetationComponent(const FObjectInitializer& ObjectInitializer);

  void BeginPlay() override;

  void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

  void TickComponent(float DeltaTime,
      enum ELevelTick TickType,
      FActorComponentTickFunction * ThisTickFunction) override;

  void UpdateSkeletalMesh();

  void UpdateGlobalTransform();

  void GenerateCollisionCapsules();

  UFUNCTION()
  void OnCollisionEvent(
      UPrimitiveComponent* HitComponent,
      AActor* OtherActor,
      UPrimitiveComponent* OtherComponent,
      FVector NormalImpulse,
      const FHitResult& Hit);

  UFUNCTION()
  void OnBeginOverlapEvent(
      UPrimitiveComponent* OverlapComponent,
      AActor* OtherActor,
      UPrimitiveComponent* OtherComponent,
      int32 OtherBodyIndex,
      bool bFromSweep,
      const FHitResult& SweepResult);

  UFUNCTION()
  void OnEndOverlapEvent(
      UPrimitiveComponent* OverlapComponent,
      AActor* OtherActor,
      UPrimitiveComponent* OtherComponent,
      int32 OtherBodyIndex);

  // UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spring Based Vegetation Component")
  // TArray<AActor*> OverlappingActors;
  // UPROPERTY(EditAnywhere, Category = "Spring Based Vegetation Component")
  TMap<AActor*, TArray<UPrimitiveComponent*>> OverlappingActors;

  UFUNCTION(BlueprintCallable)
  void AddForce(const FString& BoneName, const FVector& Force)
  {
    Skeleton.AddForce(BoneName, Force);
  }

private:

  UPROPERTY(EditAnywhere, Category = "Spring Based Vegetation Component")
  USkeletalMeshComponent* SkeletalMesh;

  UPROPERTY(EditAnywhere, Category = "Spring Based Vegetation Component")
  TArray<UPrimitiveComponent*> BoneCapsules;

  UPROPERTY(EditAnywhere, Category = "Spring Based Vegetation Component")
  TMap<UPrimitiveComponent*, int> CapsuleToJointId;

  UPROPERTY(EditAnywhere, Category = "Spring Based Vegetation Component")
  float Beta = 0.5f;

  UPROPERTY(EditAnywhere, Category = "Spring Based Vegetation Component")
  FVector Gravity = FVector(0,0,-1);

  UPROPERTY(EditAnywhere, Category = "Spring Based Vegetation Component")
  float SpringStrength = 2.f;

  UPROPERTY(EditAnywhere, Category = "Spring Based Vegetation Component")
  float DeltaTimeOverride = -1.f;
  
  UPROPERTY(EditAnywhere, Category = "Spring Based Vegetation Component")
  float CollisionForceParameter = 10.f;

  FSkeletonHierarchy Skeleton;


};
