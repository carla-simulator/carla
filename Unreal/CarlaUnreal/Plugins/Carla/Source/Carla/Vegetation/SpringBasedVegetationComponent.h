// Copyright (c) 2024 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include <util/ue-header-guard-begin.h>
#include "Components/ActorComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include <util/ue-header-guard-end.h>

#include <vector>

#include "SpringBasedVegetationComponent.generated.h"

struct FJointProperties;

USTRUCT(BlueprintType)
struct FSkeletonBone
{
  GENERATED_BODY()

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skeleton Bone")
  float Mass = 1.f;
  UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Skeleton Bone")
  float Length = 0.5f;
  UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Skeleton Bone")
  FVector CenterOfMass = FVector(0,0,0);
};

USTRUCT(BlueprintType)
struct FSkeletonJoint
{
  GENERATED_BODY()

  UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Skeleton Bone")
  int JointId = 0;
  UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Skeleton Bone")
  int ParentId = 0;
  UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Skeleton Bone")
  FString JointName;
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skeleton Bone")
  bool bIsStatic = false;
  UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Skeleton Bone")
  TArray<int> ChildrenIds;
  UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Skeleton Bone")
  FTransform Transform = FTransform::Identity; // relative to parent
  UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Skeleton Bone")
  FRotator RestingAngles = FRotator::ZeroRotator; // resting angle position of springs
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skeleton Bone")
  float Mass = 10.f;
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skeleton Bone")
  float SpringStrength = 1000.f;
  UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Skeleton Bone")
  FRotator AngularVelocity = FRotator::ZeroRotator;
  UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Skeleton Bone")
  FRotator AngularAcceleration = FRotator::ZeroRotator;
  UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Skeleton Bone")
  FTransform GlobalTransform = FTransform::Identity;
  UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Skeleton Bone")
  FTransform GlobalInverseTransform = FTransform::Identity;
  UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Skeleton Bone")
  TArray<FSkeletonBone> Bones;
  UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Skeleton Bone")
  FVector ExternalForces = FVector::ZeroVector;
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skeleton Bone")
  float CollisionForceProportionalFactor = 1.0f;
};

struct FJointCollision
{
  bool CanRest = true;
  int Iteration = 1;
};

USTRUCT(BlueprintType)
struct FSkeletonHierarchy
{
  GENERATED_BODY()

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skeleton Bone")
  TArray<FSkeletonJoint> Joints;
  UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Skeleton Bone")
  TArray<int> EndJoints;
  UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Skeleton Bone")
  TArray<int> EndToRootOrder;
  UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Skeleton Bone")
  TArray<int> RootToEndOrder;
  
  void Clear();
  void ComputeChildrenJointsAndBones();
  void ComputeEndJoints();
  void AddForce(const FString& BoneName, const FVector& Force);
  void ClearExternalForces();
  FSkeletonJoint& GetRootJoint() { return Joints[0]; }
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

  void ResetComponent();

  UFUNCTION(CallInEditor, Category = "Spring Based Vegetation Component")
  void ComputeSpringStrengthForBranches();

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

  UFUNCTION(CallInEditor, Category = "Spring Based Vegetation Component")
  void GenerateSkeletonHierarchy();
  // UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spring Based Vegetation Component")
  // TArray<AActor*> OverlappingActors;
  // UPROPERTY(EditAnywhere, Category = "Spring Based Vegetation Component")
  TMap<AActor*, TArray<UPrimitiveComponent*>> OverlappingActors;

  UFUNCTION(BlueprintCallable)
  void AddForce(const FString& BoneName, const FVector& Force)
  {
    Skeleton.AddForce(BoneName, Force);
  }
  UPROPERTY(EditAnywhere, Category = "Spring Based Vegetation Component")
  TArray<FString> FixedJointsList = {"joint1"};

  UPROPERTY(EditAnywhere, Category = "Spring Based Vegetation Component")
  TArray<FString> DebugJointsToSimulate = {};

private:

  void ComputePerJointProperties(
      std::vector<FJointProperties>& JointLocalPropertiesList,
      std::vector<FJointProperties>& JointPropertiesList);
  void ComputeCompositeBodyContribution(
      std::vector<FJointProperties>& JointLocalPropertiesList,
      std::vector<FJointProperties>& JointPropertiesList);
  void ComputeFictitiousForces(
      std::vector<FJointProperties>& JointLocalPropertiesList,
      std::vector<FJointProperties>& JointPropertiesList);
  void ResolveContactsAndCollisions(
      std::vector<FJointProperties>& JointLocalPropertiesList,
      std::vector<FJointProperties>& JointPropertiesList);
  void SolveEquationOfMotion(std::vector<FJointProperties>& JointPropertiesList, float DeltaTime);

  std::vector<FJointCollision> JointCollisionList;

public:
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spring Based Vegetation Component")
  USkeletalMeshComponent* SkeletalMesh;

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spring Based Vegetation Component")
  TArray<UPrimitiveComponent*> BoneCapsules;

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spring Based Vegetation Component")
  TMap<UPrimitiveComponent*, int> CapsuleToJointId;

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spring Based Vegetation Component")
  float Beta = 0.5f;

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spring Based Vegetation Component")
  float Alpha = 0.f;

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spring Based Vegetation Component")
  FVector Gravity = FVector(0,0,-1);

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spring Based Vegetation Component")
  float BaseSpringStrength = 10000.f;

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spring Based Vegetation Component")
  float MinSpringStrength = 2000.f;

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spring Based Vegetation Component")
  float HorizontalFallof = 0.1f;

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spring Based Vegetation Component")
  float VerticalFallof = 0.1f;

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spring Based Vegetation Component")
  float RestFactor = 0.5f;
  
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spring Based Vegetation Component")
  float DeltaTimeOverride = -1.f;
  
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spring Based Vegetation Component")
  float CollisionForceParameter = 10.f;

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spring Based Vegetation Component")
  float CollisionForceMinVel = 1.f;

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spring Based Vegetation Component")
  float ForceDistanceFalloffExponent = 1.f;
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spring Based Vegetation Component")
  float ForceMaxDistance = 180.f;
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spring Based Vegetation Component")
  float MinForceFactor = 0.01;
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spring Based Vegetation Component")
  float LineTraceMaxDistance = 180.f;
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spring Based Vegetation Component")
  float CapsuleRadius = 6.0f;
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spring Based Vegetation Component")
  float MinBoneLength = 10.f;
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spring Based Vegetation Component")
  FVector SpringStrengthMulFactor = FVector(1,1,1);
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spring Based Vegetation Component")
  float VehicleCenterZOffset = 120.f;
  


public:
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spring Based Vegetation Component")
  FSkeletonHierarchy Skeleton;

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spring Based Vegetation Component")
  bool DebugEnableVisualization { false };

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spring Based Vegetation Component")
  bool DebugEnableAllCollisions { false };

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spring Based Vegetation Component")
  bool bAutoComputeStrength = true;
};
