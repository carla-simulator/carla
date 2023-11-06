// Copyright (c) 2022 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "SpringBasedVegetationComponent.h"
#include "Carla/Walker/WalkerAnim.h"
#include "Math/Matrix.h"
#include "Components/CapsuleComponent.h"
#include "DrawDebugHelpers.h"
#include "Kismet/KismetMathLibrary.h"
#include "BaseVegetationActor.h"
#include "Vehicle/CarlaWheeledVehicle.h"
#include <unordered_set>
#include <vector>
#include <cmath>
#include <sstream>

#include <compiler/disable-ue4-macros.h>
#include "carla/rpc/String.h"
#include <compiler/enable-ue4-macros.h>

#define SPRINGVEGETATIONLOGS 0
#define SOLVERLOGS 0
#define COLLISIONLOGS 0
#define ACCUMULATIONLOGS 0
#define FICTITIOUSFORCELOGS 0
#define OTHERLOGS 0

#if SOLVERLOGS && SPRINGVEGETATIONLOGS
#define SOLVER_LOG(Level, Msg, ...) UE_LOG(LogCarla, Level, TEXT(Msg), ##__VA_ARGS__)
#else
#define SOLVER_LOG(...)
#endif
#if COLLISIONLOGS && SPRINGVEGETATIONLOGS
#define COLLISION_LOG(Level, Msg, ...) UE_LOG(LogCarla, Level, TEXT(Msg), ##__VA_ARGS__)
#else
#define COLLISION_LOG(...)
#endif
#if ACCUMULATIONLOGS && SPRINGVEGETATIONLOGS
#define ACC_LOG(Level, Msg, ...) UE_LOG(LogCarla, Level, TEXT(Msg), ##__VA_ARGS__)
#else
#define ACC_LOG(...)
#endif
#if FICTITIOUSFORCELOGS && SPRINGVEGETATIONLOGS
#define FICT_LOG(Level, Msg, ...) UE_LOG(LogCarla, Level, TEXT(Msg), ##__VA_ARGS__)
#else
#define FICT_LOG(...)
#endif
#if OTHERLOGS && SPRINGVEGETATIONLOGS
#define OTHER_LOG(Level, Msg, ...) UE_LOG(LogCarla, Level, TEXT(Msg), ##__VA_ARGS__)
#else
#define OTHER_LOG(...)
#endif


FRotator GetDeltaRotator(const FRotator & Rotator1, const FRotator & Rotator2)
{
  float HalfCircle = 180.f;
  float FullCircle = 360.f;
  auto GetDeltaAngle = [&](float Angle1, float Angle2) -> float
  {
    if (Angle1 < 0)
    {
      Angle1 = FullCircle + Angle1;
    }
    if (Angle2 < 0)
    {
      Angle2 = FullCircle + Angle2;
    }
    float Diff = fmod( Angle1 - Angle2 + HalfCircle , FullCircle) - HalfCircle;
    return Diff < -HalfCircle ? Diff + FullCircle : Diff;
  };
  
  return FRotator(
    GetDeltaAngle(Rotator1.Pitch, Rotator2.Pitch),
    GetDeltaAngle(Rotator1.Yaw, Rotator2.Yaw),
    GetDeltaAngle(Rotator1.Roll, Rotator2.Roll)
  );
}

template <class T>
static T GetSign(T n)
{
  return n < 0.0f ? -1.0f : 1.0f;
}
template <class T>
static FString EigenToFString(T& t)
{
  std::stringstream ss;
  ss << t;
  return carla::rpc::ToFString(ss.str());
}
static Eigen::Matrix3d OuterProduct(const Eigen::Vector3d& V1, const Eigen::Vector3d& V2)
{
  return V1 * V2.transpose();
}
static Eigen::Matrix3d OuterProduct(const Eigen::Vector3d& V1)
{
  return V1 * V1.transpose();
}
// convert to right handed frame by flipping z coordinate
static Eigen::Vector3d ToEigenVector(const FVector& V1)
{
  return Eigen::Vector3d(V1.X, V1.Y, -V1.Z);
}
static FVector ToUnrealVector(const Eigen::Vector3d& V1)
{
  return FVector(V1(0), V1(1), -V1(2));
}
static Eigen::Matrix3d ToEigenMatrix(const FMatrix& Matrix) // Matrix[row][column]
{
  Eigen::Matrix3d EigenMatrix;
  EigenMatrix << Matrix.M[0][0], Matrix.M[0][1], -Matrix.M[0][2],
                 Matrix.M[1][0], Matrix.M[1][1], -Matrix.M[1][2],
                 Matrix.M[2][0], Matrix.M[2][1], -Matrix.M[2][2];
  return EigenMatrix.transpose();
}
static Eigen::Matrix3d ToEigenMatrix(const FTransform& Transform)
{
  FMatrix Matrix = Transform.ToMatrixNoScale(); // Matrix[row][column]
  return ToEigenMatrix(Matrix);
}
static Eigen::Vector3d RotatorToEigenVector(const FRotator& Rotator)
{
  return Eigen::Vector3d(
      FMath::DegreesToRadians(Rotator.Roll),
      FMath::DegreesToRadians(Rotator.Pitch),
      -FMath::DegreesToRadians(Rotator.Yaw));
}
static FRotator EigenVectorToRotator(const Eigen::Vector3d& Vector)
{
  return FRotator(
      FMath::RadiansToDegrees(Vector(1)),
      -FMath::RadiansToDegrees(Vector(2)),
      FMath::RadiansToDegrees(Vector(0)));
}

void FSkeletonHierarchy::Clear()
{
  Joints.Empty();
  EndJoints.Empty();
  EndToRootOrder.Empty();
  RootToEndOrder.Empty();
}
void FSkeletonHierarchy::ComputeChildrenJointsAndBones()
{
  TRACE_CPUPROFILER_EVENT_SCOPE(FSkeletonHierarchy::ComputeChildrenJointsAndBones);
  for (int i = 1; i < Joints.Num(); i++)
  {
    FSkeletonJoint& Joint = Joints[i];
    FSkeletonJoint& ParentJoint = Joints[Joint.ParentId];
    ParentJoint.ChildrenIds.Add(i);
  }

  // debug
  for (int i = 0; i < Joints.Num(); i++)
  {
    FSkeletonJoint& Joint = Joints[i];
    FString ChildrenIds = "";
    for (int ChildrenId : Joint.ChildrenIds)
    {
      ChildrenIds += FString::FromInt(ChildrenId) + ", ";
    }
    OTHER_LOG(Warning, "Joint %d, Children: %s.", i, *ChildrenIds);
  }
}

void FSkeletonHierarchy::ComputeEndJoints()
{
  TRACE_CPUPROFILER_EVENT_SCOPE(FSkeletonHierarchy::ComputeEndJoints);
  EndJoints.Empty();
  for (int i = 0; i < Joints.Num(); i++)
  {
    FSkeletonJoint& Joint = Joints[i];
    if (!Joint.ChildrenIds.Num())
    {
      EndJoints.Add(i);
    }
  }

  // build traversal order so that parent nodes are visited before any children
  RootToEndOrder.Empty();
  std::vector<int> JointsToVisit;
  JointsToVisit.emplace_back(0); // root element in the hierarchy
  RootToEndOrder.Add(0);
  while (JointsToVisit.size())
  {
    FSkeletonJoint& Joint = Joints[JointsToVisit.back()];
    JointsToVisit.pop_back();

    for (int ChildrenId : Joint.ChildrenIds)
    {
      RootToEndOrder.Add(ChildrenId);
      JointsToVisit.emplace_back(ChildrenId);
    }
  }

  // build the order in reverse, children visited before parents
  EndToRootOrder.Empty();
  FString IdOrder = "";
  FString NameOrder = "";
  for(int i = RootToEndOrder.Num() - 1; i >= 0; i--)
  {
    int Id = RootToEndOrder[i];
    EndToRootOrder.Add(Id);
    IdOrder += FString::FromInt(Id) + " ";
    NameOrder += Joints[Id].JointName + " ";
  }
  //debug
  OTHER_LOG(Warning, "Tree order: %s", *IdOrder);
  OTHER_LOG(Warning, "Tree order (names): %s", *NameOrder);
  OTHER_LOG(Warning, "Num elements: %d", EndToRootOrder.Num());
}

void FSkeletonHierarchy::AddForce(const FString& BoneName, const FVector& Force)
{
  TRACE_CPUPROFILER_EVENT_SCOPE(FSkeletonHierarchy::AddForce);
  for (FSkeletonJoint& Joint : Joints)
  {
    if(Joint.JointName == BoneName)
    {
      Joint.ExternalForces += Force;
    }
  }
}
void FSkeletonHierarchy::ClearExternalForces()
{
  TRACE_CPUPROFILER_EVENT_SCOPE(FSkeletonHierarchy::ClearExternalForces);
  for (FSkeletonJoint& Joint : Joints)
  {
    Joint.ExternalForces = FVector(0,0,0);
  }
}

USpringBasedVegetationComponent::USpringBasedVegetationComponent(const FObjectInitializer& ObjectInitializer)
  : Super(ObjectInitializer)
{
  PrimaryComponentTick.bCanEverTick = true;
  PrimaryComponentTick.bStartWithTickEnabled = true;
}

void USpringBasedVegetationComponent::GenerateSkeletonHierarchy()
{
  TRACE_CPUPROFILER_EVENT_SCOPE(USpringBasedVegetationComponent::GenerateSkeletonHierarchy);
  OTHER_LOG(Warning, "Get skeleton hierarchy");
  // Get skeleton hierarchy
  if (!SkeletalMesh)
  {
    UActorComponent* Component = GetOwner()->GetComponentByClass(USkeletalMeshComponent::StaticClass());
    SkeletalMesh = Cast<USkeletalMeshComponent>(Component);
  }
  TArray<FName> BoneNames;
  SkeletalMesh->GetBoneNames(BoneNames);
  Skeleton.Clear();
  for (int32 i = 0; i < BoneNames.Num(); i++)
  {
    FName& BoneName = BoneNames[i];
    FString BoneNameString = BoneName.ToString();
    bool bIsFixedJoint = FixedJointsList.Contains(BoneNameString) || 
        (DebugJointsToSimulate.Num() && !DebugJointsToSimulate.Contains(BoneNameString));
    FName ParentBoneName = SkeletalMesh->GetParentBone(BoneName);
    int32 ParentIndex = SkeletalMesh->GetBoneIndex(ParentBoneName);
    Skeleton.Joints.Add(FSkeletonJoint{i, ParentIndex, BoneNameString, bIsFixedJoint});
    OTHER_LOG(Log, "Added bone %s with id %d and parent %d", *BoneNameString, i, ParentIndex);
  }

  Skeleton.ComputeChildrenJointsAndBones();
  Skeleton.ComputeEndJoints();

    // Get resting pose for bones
  auto *AnimInst = SkeletalMesh->GetAnimInstance();
  if (!AnimInst)
  {
    OTHER_LOG(Error, "Could not get animation instance.");
    return;
  }
  UWalkerAnim *WalkerAnim = Cast<UWalkerAnim>(AnimInst);
  if (!WalkerAnim)
  {
    OTHER_LOG(Error, "Could not get UWalkerAnim.");
    return;
  }

  // get current pose
  FPoseSnapshot TempSnapshot;
  SkeletalMesh->SnapshotPose(TempSnapshot);

  // copy pose
  WalkerAnim->Snap = TempSnapshot;

  for (int i=0; i<Skeleton.Joints.Num(); ++i)
  {
    FSkeletonJoint& Joint = Skeleton.Joints[i];
    FTransform JointTransform = SkeletalMesh->GetSocketTransform(FName(*Joint.JointName), ERelativeTransformSpace::RTS_ParentBoneSpace);
    Joint.Transform = JointTransform;
    Joint.RestingAngles = JointTransform.Rotator();
    OTHER_LOG(Log, "Getting info for bone %s, %f, %f, %f, %f", *Joint.JointName, Joint.RestingAngles.Pitch, Joint.RestingAngles.Yaw, Joint.RestingAngles.Roll);
    if(i > 0)
    {
      FSkeletonJoint& ParentJoint = Skeleton.Joints[Joint.ParentId];
      FVector BoneCOM = Joint.Transform.GetLocation()*0.5f;
      float BoneLength = Joint.Transform.GetLocation().Size();
      ParentJoint.Bones.Add({10, BoneLength, BoneCOM});
    }
  }

  UpdateGlobalTransform();
}

void USpringBasedVegetationComponent::BeginPlay()
{
  TRACE_CPUPROFILER_EVENT_SCOPE(USpringBasedVegetationComponent::BeginPlay);
  Super::BeginPlay();
  OTHER_LOG(Warning, "USpringBasedVegetationComponent::BeginPlay");
  OTHER_LOG(Warning, "Params: BaseSpringStrength %f, Num joints: %d, CollisionForceParameter %f", BaseSpringStrength, Skeleton.Joints.Num(), CollisionForceParameter);
  if (!SkeletalMesh)
  {
    UActorComponent* Component = GetOwner()->GetComponentByClass(USkeletalMeshComponent::StaticClass());
    SkeletalMesh = Cast<USkeletalMeshComponent>(Component);
  }
  if (!SkeletalMesh)
  {
    SetComponentTickEnabled(false);
    OTHER_LOG(Error, "Could not find skeletal mesh component.");
    return;
  }

  ABaseVegetationActor* BaseVegetation = Cast<ABaseVegetationActor>(GetOwner());
  if(BaseVegetation)
  {
    BaseVegetation->SetParametersToComponent();
  }

  // set callbacks
  SkeletalMesh->OnComponentHit.AddDynamic(this, &USpringBasedVegetationComponent::OnCollisionEvent);
  
  if (!Skeleton.Joints.Num())
  {
    GenerateSkeletonHierarchy();
  }

  UpdateGlobalTransform();
  GenerateCollisionCapsules();
  if(bAutoComputeStrength)
  {
    ComputeSpringStrengthForBranches();
  }

  JointCollisionList.resize(Skeleton.Joints.Num());
}

void USpringBasedVegetationComponent::ResetComponent()
{
  Skeleton.ClearExternalForces();
  SkeletalMesh->ResetAllBodiesSimulatePhysics();
}

void USpringBasedVegetationComponent::GenerateCollisionCapsules()
{
  for (FSkeletonJoint& Joint : Skeleton.Joints)
  {
    for (FSkeletonBone& Bone : Joint.Bones)
    {
      if (Bone.Length < MinBoneLength)
      {
        continue;
      }
      UCapsuleComponent* Capsule = NewObject<UCapsuleComponent>(GetOwner());
      Capsule->AttachToComponent(SkeletalMesh, FAttachmentTransformRules::KeepRelativeTransform, FName(*Joint.JointName));
      Capsule->RegisterComponent();
      // create rotation from z direction to align the capsule
      FRotator BoneRotation = UKismetMathLibrary::MakeRotFromZ(Bone.CenterOfMass.GetSafeNormal());
      FTransform CapsuleTransform(BoneRotation, Bone.CenterOfMass, FVector(1,1,1));
      Capsule->SetRelativeTransform(CapsuleTransform);
      Capsule->SetCapsuleHalfHeight(Bone.Length*0.5f);
      Capsule->SetCapsuleRadius(CapsuleRadius);
      if (Joint.bIsStatic)
      {
        Capsule->SetGenerateOverlapEvents(false);
        Capsule->SetCollisionProfileName("BlockAll");
        Capsule->OnComponentHit.AddDynamic(this, &USpringBasedVegetationComponent::OnCollisionEvent);
      }
      else
      {
        Capsule->SetGenerateOverlapEvents(true);
        Capsule->SetCollisionProfileName("OverlapAll");
        Capsule->OnComponentBeginOverlap.AddDynamic(this, &USpringBasedVegetationComponent::OnBeginOverlapEvent);
        Capsule->OnComponentEndOverlap.AddDynamic(this, &USpringBasedVegetationComponent::OnEndOverlapEvent);
      }
      
      BoneCapsules.Add(Capsule);
      CapsuleToJointId.Add(Capsule, Joint.JointId);
    }
  }
}

void USpringBasedVegetationComponent::ComputeSpringStrengthForBranches()
{
  TRACE_CPUPROFILER_EVENT_SCOPE(USpringBasedVegetationComponent::ComputeSpringStrengthForBranches);
  FTransform RootTransform = Skeleton.GetRootJoint().GlobalTransform;
  FVector RootLocation = RootTransform.GetLocation();
  // FVector TreeAxis = RootTransform.GetRotation().GetUpVector();
  FVector TreeAxis = FVector(0,0,1);
  for (FSkeletonJoint& Joint : Skeleton.Joints)
  {
    FVector JointLocation = Joint.GlobalTransform.GetLocation();
    FVector ClosestPoint;
    float HorizontalDistance = FMath::PointDistToLine(JointLocation, TreeAxis, RootLocation, ClosestPoint);
    float VerticalDistance = FVector::Distance(ClosestPoint, RootLocation);

    Joint.SpringStrength = FMath::Max(
          BaseSpringStrength - HorizontalFallof*HorizontalDistance - VerticalFallof*VerticalDistance,
          MinSpringStrength);

    OTHER_LOG(Log, "Joint: %s, location %s, Strength %f", *Joint.JointName, *JointLocation.ToString(), Joint.SpringStrength);
  }
}

void USpringBasedVegetationComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
  for (UPrimitiveComponent* Capsule : BoneCapsules)
  {
    Capsule->DestroyComponent();
  }
  BoneCapsules.Empty();
}

// Compute a single joint properties (Center of Mass, Inertia, Forces and Torque)
void USpringBasedVegetationComponent::ComputePerJointProperties(
    std::vector<FJointProperties>& JointLocalPropertiesList,
    std::vector<FJointProperties>& JointPropertiesList)
{
  TRACE_CPUPROFILER_EVENT_SCOPE(USpringBasedVegetationComponent::ComputePerJointProperties);
  for(FSkeletonJoint& Joint : Skeleton.Joints)
  {
    FJointProperties& Properties = JointLocalPropertiesList[Joint.JointId];
    Eigen::Vector3d JointGlobalPosition = ToEigenVector(Joint.GlobalTransform.GetLocation())/100.f;
    Properties.JointToGlobalMatrix = ToEigenMatrix(Joint.GlobalTransform);
    JointPropertiesList[Joint.JointId].JointToGlobalMatrix = Properties.JointToGlobalMatrix;
    if (!Joint.Bones.Num())
      continue;
    // COM and mass
    for (FSkeletonBone& Bone : Joint.Bones)
    {
      Properties.Mass += Bone.Mass;
      FVector GlobalCenterOfMass = Joint.GlobalTransform.TransformPositionNoScale(Bone.CenterOfMass);
      Properties.CenterOfMass += Bone.Mass*ToEigenVector(GlobalCenterOfMass)/100.f;
    }
    Properties.CenterOfMass = Properties.CenterOfMass/Properties.Mass;

    // force
    Eigen::Vector3d GravityForce = ToEigenVector(Gravity)/100.f; // world space gravity
    Properties.Force = Properties.Mass * GravityForce + ToEigenVector(Joint.ExternalForces)/100.f;
    // torque
    Properties.Torque = (Properties.CenterOfMass - JointGlobalPosition).cross(Properties.Force);
    // inertia tensor
    for (FSkeletonBone& Bone : Joint.Bones)
    {
      if (Bone.Length < 1)
        continue;
      float CylinderRadius = 0.1f;
      float CylinderHeight = Bone.Length/100.f;
      Eigen::Matrix3d LocalCylinderInertia;
      LocalCylinderInertia <<
              0.5 * Bone.Mass*CylinderRadius*CylinderRadius, 0.f, 0.f,
              0.f, 1.f/12.f * Bone.Mass*(3*CylinderRadius*CylinderRadius + CylinderHeight*CylinderHeight), 0.f,
              0.f, 0.f, 1.f/12.f * Bone.Mass*(3*CylinderRadius*CylinderRadius + CylinderHeight*CylinderHeight);
      Eigen::Vector3d BoneVector = ToEigenVector(Bone.CenterOfMass)/100.f;
      Eigen::Vector3d LocalV1 = BoneVector.normalized();
      Eigen::Vector3d LocalV2 = LocalV1.cross(Eigen::Vector3d(0,1,0));
      if (LocalV2.norm() == 0)
        LocalV2 = LocalV1.cross(Eigen::Vector3d(0,0,1));
      LocalV2.normalize();
      Eigen::Vector3d LocalV3 = LocalV1.cross(LocalV2);
      Eigen::Matrix3d LocalToJointMatrix;
      LocalToJointMatrix << LocalV1, LocalV2, LocalV3;
      Eigen::Matrix3d JointSpaceInertia = LocalToJointMatrix*LocalCylinderInertia*(LocalToJointMatrix.transpose());
      Properties.InertiaTensor += Properties.JointToGlobalMatrix*JointSpaceInertia*Properties.JointToGlobalMatrix.transpose();
      Eigen::Vector3d BoneCenterWorldSpace = ToEigenVector(Joint.GlobalTransform.TransformPositionNoScale(Bone.CenterOfMass))/100.0;
      Properties.InertiaTensor += Properties.Mass*OuterProduct(Properties.CenterOfMass - BoneCenterWorldSpace, Properties.CenterOfMass - BoneCenterWorldSpace);
    }
    ACC_LOG(Log, "Local Joint: %s \n Inertia \n %s \n Force \n %s \n Torque \n %s \n COM: \n %s \n Mass %f", *Joint.JointName, *EigenToFString(Properties.InertiaTensor), *EigenToFString(Properties.Force), *EigenToFString(Properties.Torque), *EigenToFString(Properties.CenterOfMass), Properties.Mass);
  }
}

// Compute accumulated properties (Center of Mass, Inertia, Forces and Torque)
void USpringBasedVegetationComponent::ComputeCompositeBodyContribution(
    std::vector<FJointProperties>& JointLocalPropertiesList,
    std::vector<FJointProperties>& JointPropertiesList)
{
  TRACE_CPUPROFILER_EVENT_SCOPE(USpringBasedVegetationComponent::ComputeCompositeBodyContribution);
  for (int Id : Skeleton.EndToRootOrder)
  {
    FSkeletonJoint& Joint = Skeleton.Joints[Id];
    FJointProperties& JointProperties = JointPropertiesList[Joint.JointId];
    FJointProperties& JointLocalProperties = JointLocalPropertiesList[Joint.JointId];

    if (!Joint.ChildrenIds.Num())
    {
      continue;
    }

    Eigen::Vector3d CenterOfMass = JointLocalProperties.Mass*JointLocalProperties.CenterOfMass;
    // compute COM and accumulate mass
    JointProperties.Mass = JointLocalProperties.Mass;
    for(int ChildrenId : Joint.ChildrenIds)
    {
      FSkeletonJoint& ChildrenJoint = Skeleton.Joints[ChildrenId];
      FJointProperties& ChildrenProperties = JointPropertiesList[ChildrenId];
      CenterOfMass += ChildrenProperties.Mass*ChildrenProperties.CenterOfMass;
      JointProperties.Mass += ChildrenProperties.Mass;
    }
    JointProperties.CenterOfMass = CenterOfMass / JointProperties.Mass;

    // compute forces
    JointProperties.Force = JointLocalProperties.Force;
    for(int ChildrenId : Joint.ChildrenIds)
    {
      FSkeletonJoint& ChildrenJoint = Skeleton.Joints[ChildrenId];
      FJointProperties& ChildrenProperties = JointPropertiesList[ChildrenId];
      JointProperties.Force += ChildrenProperties.Force;
    }

    // compute torque
    JointProperties.Torque = JointLocalProperties.Torque;
    for(int ChildrenId : Joint.ChildrenIds)
    {
      FSkeletonJoint& ChildrenJoint = Skeleton.Joints[ChildrenId];
      FJointProperties& ChildrenProperties = JointPropertiesList[ChildrenId];
      Eigen::Vector3d ChildrenJointGlobalPosition = ToEigenVector(ChildrenJoint.GlobalTransform.GetLocation())/100.f;
      Eigen::Vector3d ParentJointGlobalPosition = ToEigenVector(Joint.GlobalTransform.GetLocation())/100.f;
      JointProperties.Torque += ChildrenProperties.Torque + (ChildrenJointGlobalPosition - ParentJointGlobalPosition).cross(ChildrenProperties.Force);
    }

    // compute inertia tensor
    JointProperties.InertiaTensor = JointLocalProperties.InertiaTensor + JointLocalProperties.Mass*OuterProduct(JointProperties.CenterOfMass - JointLocalProperties.CenterOfMass, JointProperties.CenterOfMass - JointLocalProperties.CenterOfMass);
    for(int ChildrenId : Joint.ChildrenIds)
    {
      FSkeletonJoint& ChildrenJoint = Skeleton.Joints[ChildrenId];
      FJointProperties& ChildrenProperties = JointPropertiesList[ChildrenId];
      Eigen::Vector3d ChildrenToCenterOfMass = JointProperties.CenterOfMass - ChildrenProperties.CenterOfMass;

      JointProperties.InertiaTensor += ChildrenProperties.InertiaTensor + ChildrenProperties.Mass*OuterProduct(ChildrenToCenterOfMass, ChildrenToCenterOfMass);
    }
    ACC_LOG(Log, "Accumulated Joint: %s \n Inertia \n %s \n Force \n %s \n Torque \n %s \n COM: \n %s \n Mass %f", *Joint.JointName, *EigenToFString(JointProperties.InertiaTensor), *EigenToFString(JointProperties.Force), *EigenToFString(JointProperties.Torque), *EigenToFString(JointProperties.CenterOfMass), JointProperties.Mass);
  }
}

void USpringBasedVegetationComponent::ComputeFictitiousForces(
    std::vector<FJointProperties>& JointLocalPropertiesList,
    std::vector<FJointProperties>& JointPropertiesList)
{
  TRACE_CPUPROFILER_EVENT_SCOPE(USpringBasedVegetationComponent::ComputeFictitiousForces);
  // fictitious forces
  FSkeletonJoint& RootJoint = Skeleton.Joints[0];
  FJointProperties& RootProperties = JointPropertiesList[0];
  for (int i = 1; i < Skeleton.RootToEndOrder.Num(); i++)
  {
    int Id = Skeleton.RootToEndOrder[i];
    FSkeletonJoint& Joint = Skeleton.Joints[Id];
    FJointProperties& JointProperties = JointPropertiesList[Joint.JointId];
    FSkeletonJoint& ParentJoint = Skeleton.Joints[Joint.ParentId];
    FJointProperties& ParentProperties = JointPropertiesList[Joint.ParentId];

    Eigen::Matrix3d JointToGlobalMatrix = JointProperties.JointToGlobalMatrix;
    Eigen::Matrix3d GlobalToJointMatrix = JointToGlobalMatrix.transpose();
    Eigen::Matrix3d ParentJointToGlobalMatrix = ParentProperties.JointToGlobalMatrix;
    Eigen::Matrix3d GlobalToParentJointMatrix = ParentJointToGlobalMatrix.transpose();
    Eigen::Vector3d JointGlobalPosition = ToEigenVector(Joint.GlobalTransform.GetLocation())/100.f;
    Eigen::Vector3d ParentJointGlobalPosition = ToEigenVector(ParentJoint.GlobalTransform.GetLocation())/100.f;

    JointProperties.AngularVelocity = ParentProperties.AngularVelocity
        + ParentJointToGlobalMatrix*RotatorToEigenVector(ParentJoint.AngularVelocity);
    JointProperties.LinearVelocity = ParentProperties.LinearVelocity
        + ParentProperties.AngularVelocity.cross(JointGlobalPosition - ParentJointGlobalPosition);
    JointProperties.AngularAcceleration = ParentProperties.AngularAcceleration
        + ParentJointToGlobalMatrix*RotatorToEigenVector(ParentJoint.AngularAcceleration)
        + ParentProperties.AngularVelocity.cross(JointProperties.AngularVelocity);
    JointProperties.LinearAcceleration = ParentProperties.LinearAcceleration
        + ParentProperties.LinearAcceleration.cross(JointGlobalPosition - ParentJointGlobalPosition)
        + ParentProperties.AngularVelocity.cross(JointProperties.AngularVelocity - ParentProperties.AngularVelocity);
    
    Eigen::Vector3d CompositeCenterOfMass = JointProperties.CenterOfMass;
    Eigen::Vector3d CompositeLinearVelocity = JointProperties.LinearVelocity
        + JointProperties.AngularVelocity.cross(CompositeCenterOfMass - JointGlobalPosition);
    Eigen::Vector3d CompositeLinearAcceleration = JointProperties.LinearAcceleration
        + JointProperties.AngularAcceleration.cross(CompositeCenterOfMass - JointGlobalPosition)
        + JointProperties.AngularVelocity.cross(CompositeLinearVelocity - JointProperties.LinearVelocity);
        
    Eigen::Vector3d TotalForces = -Joint.Mass * JointProperties.LinearAcceleration;
    Eigen::Vector3d TotalTorque = (CompositeCenterOfMass - JointGlobalPosition).cross(TotalForces);
    JointProperties.FictitiousTorque = GlobalToJointMatrix*TotalTorque;

    FICT_LOG(Log, "Joint: %s \n Position \n %s \n Velocity \n %s \n Acceleration \n %s \n Fictitious forces: \n %s", *Joint.JointName, *EigenToFString(CompositeCenterOfMass), *EigenToFString(CompositeLinearVelocity), *EigenToFString(CompositeLinearAcceleration), *EigenToFString(JointProperties.FictitiousTorque));
  }
}

void USpringBasedVegetationComponent::ResolveContactsAndCollisions(
    std::vector<FJointProperties>& JointLocalPropertiesList,
    std::vector<FJointProperties>& JointPropertiesList)
{
  TRACE_CPUPROFILER_EVENT_SCOPE(USpringBasedVegetationComponent::ResolveContactsAndCollisions);

  // set all joints that can rest
  for (auto &Joint : JointCollisionList)
  {
    Joint.CanRest = true;
  }

  for (auto& ActorCapsules : OverlappingActors)
  {
    TRACE_CPUPROFILER_EVENT_SCOPE(ActorLoop);
    AActor* CollidingActor = ActorCapsules.Key;
    if (!IsValid(CollidingActor))
      continue;
    UPrimitiveComponent* Primitive = Cast<UPrimitiveComponent>(CollidingActor->GetRootComponent());
    if (!IsValid(Primitive))
      continue;
    // force transferring momentum (for the initial collision frame)
    const FVector PrimitiveVelocity = Primitive->GetComponentVelocity();
    const Eigen::Vector3d ColliderVelocity = ToEigenVector(PrimitiveVelocity) / 100.f;
    const FVector Impulse = (Primitive->GetMass() * PrimitiveVelocity);
    const Eigen::Vector3d CollisionImpulse = 0*ToEigenVector(Impulse) / 100.f;
    TArray<UPrimitiveComponent*>& CollidingCapsules = ActorCapsules.Value;
    for (UPrimitiveComponent* Capsule : CollidingCapsules)
    { 
      TRACE_CPUPROFILER_EVENT_SCOPE(CapsuleLoop);
      if (!IsValid(Capsule))
        continue;
      const FVector CapsuleLocation = Capsule->GetComponentLocation();
      FVector PrimitiveLocation = Primitive->GetComponentLocation();
      PrimitiveLocation = PrimitiveLocation + Primitive->GetUpVector()*VehicleCenterZOffset;
      FVector ClosestPointOnCapsule;
      float DistanceOnCapsule;
      FHitResult HitResult;
      bool HitFound;
      FVector ClosestPointOnCollider;
      float DistanceToCollider;
      {
        TRACE_CPUPROFILER_EVENT_SCOPE(ColliderPenetrationDistance);
        DistanceOnCapsule = Capsule->GetClosestPointOnCollision(PrimitiveLocation, ClosestPointOnCapsule);
        FVector LineDirection = (ClosestPointOnCapsule - PrimitiveLocation).GetSafeNormal();
        FVector LineTraceStart = ClosestPointOnCapsule + LineTraceMaxDistance*LineDirection;
        FVector LineTraceEnd = ClosestPointOnCapsule;
        HitFound = Primitive->LineTraceComponent(HitResult,
            LineTraceStart, LineTraceEnd, FCollisionQueryParams());
        ClosestPointOnCollider = HitResult.Location;
        DistanceToCollider = (ClosestPointOnCollider - ClosestPointOnCapsule).Size();
        if (DebugEnableVisualization)
        {
          DrawDebugLine(GetWorld(), LineTraceStart, LineTraceEnd, FColor::Orange, false, 0.1f, 0.0f, 1.f);
        }
      }
      if(!HitFound)
      {
        continue;
      }

      if (DebugEnableVisualization)
      {
        TRACE_CPUPROFILER_EVENT_SCOPE(DebugEnableVisualization);
        DrawDebugLine(GetWorld(), ClosestPointOnCapsule, ClosestPointOnCollider, FColor::Green, false, 0.1f, 0.0f, 1.f);
        static constexpr float DEBUG_SPHERE_SIZE = 5.0f;
        DrawDebugSphere(GetWorld(), ClosestPointOnCapsule, DEBUG_SPHERE_SIZE, 64, FColor(255, 0, 255, 255));
        DrawDebugSphere(GetWorld(), ClosestPointOnCollider, DEBUG_SPHERE_SIZE, 64, FColor(255, 0, 255, 255));
        DrawDebugSphere(GetWorld(), CapsuleLocation, DEBUG_SPHERE_SIZE, 64, FColor(255, 255, 255, 255));
        DrawDebugSphere(GetWorld(), PrimitiveLocation, DEBUG_SPHERE_SIZE, 64, FColor(255, 255, 255, 255));
      } 

      const int JointId = CapsuleToJointId[Capsule];
      const FSkeletonJoint& Joint = Skeleton.Joints[JointId];
      FJointProperties& JointProperties = JointLocalPropertiesList[Joint.JointId];
      FJointCollision& JointCollision = JointCollisionList[Joint.JointId];
      const Eigen::Vector3d JointGlobalPosition = ToEigenVector(Joint.GlobalTransform.GetLocation()) / 100.f;
      const Eigen::Vector3d CapsulePosition = ToEigenVector(CapsuleLocation) / 100.f;
      const Eigen::Vector3d PointOnCapsulePosition = ToEigenVector(ClosestPointOnCapsule) / 100.f;
      const Eigen::Vector3d ColliderPosition = ToEigenVector(ClosestPointOnCollider) / 100.f;
      Eigen::Vector3d CollisionTorque = Eigen::Vector3d::Zero();
      
      // Contact forces due to spring strength
      const FRotator CurrRotator = Joint.Transform.Rotator();
      const FRotator RestRotator = Joint.RestingAngles;
      const FRotator DeltaRotator = 
          GetDeltaRotator(CurrRotator, RestRotator);
      const Eigen::Vector3d SpringTorque = Joint.SpringStrength * RotatorToEigenVector(DeltaRotator);
      const Eigen::Vector3d JointCapsuleVector = JointGlobalPosition - CapsulePosition;
      const Eigen::Vector3d RepulsionForce = SpringTorque.cross(JointCapsuleVector) * JointCapsuleVector.squaredNorm();

      FVector RepulsionForceUE = -ToUnrealVector(RepulsionForce) * 100.f;
      Primitive->AddForceAtLocation(RepulsionForceUE, ClosestPointOnCollider);

      // force to repel geometry overlapping
      float ForceFactor = 1.f;
      // from eq f = 1 - a*d^p, f: ForceFactor, a: ProportionalConstant, p: ForceDistanceFalloffExponent, d: DistanceOnCapsule
      // float ProportionalConstant = 1.f/(FMath::Pow(ForceMaxDistance, ForceDistanceFalloffExponent));
      // ForceFactor = 1.f - ProportionalConstant * FMath::Pow(DistanceOnCapsule, ForceDistanceFalloffExponent);
      // from eq f = a*d^p, f: ForceFactor, a: ProportionalConstant, p: ForceDistanceFalloffExponent, d: DistanceToCollider
      float ProportionalConstant = 1.f/(FMath::Pow(ForceMaxDistance, ForceDistanceFalloffExponent));
      ForceFactor = ProportionalConstant * FMath::Pow(DistanceToCollider, ForceDistanceFalloffExponent);
      ForceFactor = FMath::Clamp(ForceFactor, MinForceFactor, 1.f);
      
      // const Eigen::Vector3d OverlappingForces = (ColliderPosition - CapsulePosition).normalized() * CollisionForceParameter * ForceFactor;
      // const Eigen::Vector3d OverlappingForces = (ColliderPosition - PointOnCapsulePosition).normalized() * CollisionForceParameter * ForceFactor;
      float Factor = 1.0f - ((JointCollision.Iteration / 100.0f) * RestFactor);
      const Eigen::Vector3d OverlappingForces = (ColliderPosition - PointOnCapsulePosition).normalized() * CollisionForceParameter * ForceFactor * Factor * Joint.CollisionForceProportionalFactor;
      Primitive->AddForceAtLocation(-ToUnrealVector(OverlappingForces) * 100.f, ClosestPointOnCollider);
      CollisionTorque += (PointOnCapsulePosition - JointGlobalPosition).cross(CollisionImpulse + OverlappingForces);
      JointProperties.Torque += CollisionTorque;
      // COLLISION_LOG(Log, "Joint: %s \n ProjectedSpeed %f, ProportionalFactor %f \n RepulsionForce %s \n", *Joint.JointName,ProjectedSpeed,ProportionalFactor,*EigenToFString(RepulsionForce),*EigenToFString(CollisionTorque));
      //UE_LOG(LogCarla, Display, TEXT("DistanceToCollider: %f, ForceFactor: %f"), DistanceToCollider, ForceFactor);
      
      // block forces to go to rest angles
      int TempId = JointId;
      do
      {
        FJointCollision& TempJointCollision = JointCollisionList[TempId];
        TempJointCollision.CanRest = false;
        
        FSkeletonJoint &TempJoint = Skeleton.Joints[TempId];
        TempId = TempJoint.ParentId;
      }
      while (TempId != -1);

      if (DebugEnableVisualization)
      {
        static constexpr float DEBUG_SPHERE_SIZE = 5.0f;
        // drawing
        const FVector Start = Capsule->GetComponentLocation();
        const FVector End = Primitive->GetComponentLocation();
        const FColor LineColor(FColor::Green);
        DrawDebugLine(GetWorld(), Start, End, LineColor, false, 0.1f, 0.0f, 1.f);
        DrawDebugLine(GetWorld(), ClosestPointOnCollider, ClosestPointOnCollider+RepulsionForceUE.GetSafeNormal()*20.f, FColor::Red, false, 0.1f, 0.0f, 1.f);
        FVector UEOverlapForces = ToUnrealVector(OverlappingForces)*100.f;
        DrawDebugLine(GetWorld(), ClosestPointOnCapsule, ClosestPointOnCapsule+UEOverlapForces.GetSafeNormal()*20.f, FColor::Turquoise, false, 0.1f, 0.0f, 1.f);
        FVector UECOM = ToUnrealVector(JointProperties.CenterOfMass )*100.f;
        DrawDebugSphere(GetWorld(), UECOM, DEBUG_SPHERE_SIZE, 64, FColor::Emerald);
        FVector UEJointPos = ToUnrealVector(JointGlobalPosition )*100.f;
        DrawDebugSphere(GetWorld(), UEJointPos, DEBUG_SPHERE_SIZE, 64, FColor::Purple);
        DrawDebugLine(GetWorld(), ClosestPointOnCapsule, UEJointPos, FColor::Cyan, false, 0.1f, 0.0f, 1.f);
      }
    }
  }

    // reset iteration
  for (auto &Joint : JointCollisionList)
  {
    if (Joint.CanRest)
    {
      if (Joint.Iteration > 1)
      {
        --Joint.Iteration;
      }
    }
    else
    {
      if (Joint.Iteration <= 95)
      {
        Joint.Iteration += 5;
      }
    }
  }
}

void USpringBasedVegetationComponent::SolveEquationOfMotion(
    std::vector<FJointProperties>& JointPropertiesList,
    float DeltaTime)
{
  TRACE_CPUPROFILER_EVENT_SCOPE(USpringBasedVegetationComponent::SolveEquationOfMotion);
  // solver
  for (FSkeletonJoint& Joint : Skeleton.Joints)
  {
    if (!Joint.Bones.Num() || Joint.bIsStatic)
    {
      continue;
    }
    FJointProperties& JointProperties = JointPropertiesList[Joint.JointId];
    FJointCollision& JointCollision = JointCollisionList[Joint.JointId];

    // debug drawing
    if (DebugEnableVisualization)
    {
      if (Joint.ParentId != -1)
      {
        FVector Start = Joint.GlobalTransform.GetLocation();
        FVector End = Skeleton.Joints[Joint.ParentId].GlobalTransform.GetLocation();
        if (!JointCollision.CanRest)
        {
          DrawDebugLine(GetWorld(), Start, End, FColor::Red, false, 0.3f, 0.0f, 1.f);
        }
        else
        {
          DrawDebugLine(GetWorld(), Start, End, FColor::Blue, false, 0.1f, 0.0f, 1.f);
        }
      }
    }
    
    float Mass = JointProperties.Mass;
    Eigen::Vector3d CenterToJoint = JointProperties.CenterOfMass - ToEigenVector(Joint.GlobalTransform.GetLocation())/100.f;
    Eigen::Matrix3d GlobalToJointMatrix = JointProperties.JointToGlobalMatrix.transpose();
    Eigen::Matrix3d JointSpaceIntertiaTensor = JointProperties.Mass*OuterProduct(CenterToJoint, CenterToJoint)
        + GlobalToJointMatrix*JointProperties.InertiaTensor*GlobalToJointMatrix.transpose();
    Eigen::Vector3d Torque = GlobalToJointMatrix*JointProperties.Torque + JointProperties.FictitiousTorque;
    // build differential equation
    Eigen::Matrix3d I = JointSpaceIntertiaTensor;
    float SpringStrength = Joint.SpringStrength;
    float beta = Beta;
    float alpha = Alpha;
    Eigen::Matrix3d K;
    K << SpringStrength*SpringStrengthMulFactor.X,0.f,0.f,
         0.f,SpringStrength*SpringStrengthMulFactor.Y,0.f,
         0.f,0.f,SpringStrength*SpringStrengthMulFactor.Z;
    Eigen::LLT<Eigen::Matrix3d> lltofI (I);
    Eigen::Matrix3d L = lltofI.matrixL();
    Eigen::Matrix3d Linv = L.inverse();
    Eigen::EigenSolver<Eigen::Matrix3d> EigenDecomposition(Linv*K*Linv.transpose());
    Eigen::Matrix3d Lambda = EigenDecomposition.eigenvalues().real().asDiagonal();
    Eigen::Matrix3d X = EigenDecomposition.eigenvectors().real();
    // Eigen::Matrix3d Lambda = U.transpose()*K*U;
    Eigen::Matrix3d U = Linv.transpose()*X;
    // Eigen::Matrix3d Uinv = U.inverse();
    Eigen::Matrix3d Uinv = X.transpose()*L.transpose();
    Eigen::Vector3d Coeffsb = Eigen::Vector3d(alpha,alpha,alpha) + beta*Lambda*Eigen::Vector3d(1,1,1);
    Eigen::Vector3d Coeffsk = Lambda*Eigen::Vector3d(1,1,1);
    Eigen::Vector3d Coeffsf = U.transpose()*Torque;
    FString StringI = EigenToFString(I);
    FString StringTorque = EigenToFString(Torque);
    FString StringU = EigenToFString(U);
    FString StringLambda = EigenToFString(Lambda);
    FString StringGlobalToJoint = EigenToFString(GlobalToJointMatrix);
    FString StringCenterToJoint = EigenToFString(CenterToJoint);
    SOLVER_LOG(Log, "Bone %s: \n I: \n %s \n Tau: \n %s \n U: \n %s \n Lambda: \n %s \n X: \n %s \n GlobalToJoint \n %s \n CenterToJoint \n %s", *Joint.JointName, *StringI, *StringTorque, *StringU, *StringLambda, *EigenToFString(X), *StringGlobalToJoint, *StringCenterToJoint);

    FRotator CurrRotator = Joint.Transform.Rotator();
    FRotator RestRotator = Joint.RestingAngles;
    FRotator AngularVelocity = Joint.AngularVelocity;
    FRotator DeltaRotator = 
        GetDeltaRotator(CurrRotator, RestRotator);
    if (!JointCollision.CanRest)
    {
      float Factor = 1.0f - ((JointCollision.Iteration / 100.0f) * RestFactor);
      // DeltaRotator *= Factor;
      AngularVelocity *= Factor;
    }
    Eigen::Vector3d InitialTheta = Uinv*RotatorToEigenVector(DeltaRotator);
    Eigen::Vector3d InitialThetaVelocity = Uinv*RotatorToEigenVector(AngularVelocity);
    SOLVER_LOG(Log, "Old angle for joint %s, %s", *Joint.JointName, *CurrRotator.ToString());
    SOLVER_LOG(Log, "InitialTheta \n %s", *EigenToFString(InitialTheta));
    Eigen::Vector3d NewTheta (0.f,0.f,0.f);
    Eigen::Vector3d NewThetaVelocity (0.f,0.f,0.f);
    Eigen::Vector3d NewThetaAccel (0.f,0.f,0.f);
    Eigen::Vector3d Discriminant = Coeffsb.cwiseProduct(Coeffsb) - 4*Coeffsk;
    // 1st row
    for (int i = 0; i < 3; i++)
    {
      double b = Coeffsb(i);
      double k = Coeffsk(i);
      double f = Coeffsf(i);
      double discriminant = Discriminant(i);
      double theta0 = InitialTheta(i);

      SOLVER_LOG(Log, "Bone %s, component %d, b %f, k %f, f %f, discriminant %f, theta0 %f", *Joint.JointName, i, b, k, f, discriminant, theta0);
      if(k == 0)
      {
        NewTheta(i) = theta0;
        SOLVER_LOG(Log, "In Bone %s, K is 0 in component %d", *Joint.JointName, i);
      }
      double dtheta0 = InitialThetaVelocity(i); // compute
      double deltatheta = 0;
      double angularvelocity = 0;
      float angularaccel = 0;
      if (discriminant > 0)
      {
        double r1 = (-b + FMath::Sqrt(discriminant))*0.5f;
        double r2 = (-b - FMath::Sqrt(discriminant))*0.5f;
        double c1 = (r2*(theta0 - f/k) - dtheta0)/(r2-r1);
        double c2 = (dtheta0 - c1*r1)/r2;
        deltatheta = c1*std::exp(r1*DeltaTime) + c2*std::exp(r2*DeltaTime) + f/k;
        angularvelocity = c1*r1*std::exp(r1*DeltaTime) + c2*r2*std::exp(r2*DeltaTime);
        SOLVER_LOG(Log, "r1 %f, r2 %f, c1 %f, c2 %f, deltatheta %f", r1, r2, c1, c2, deltatheta);
      }
      else if (discriminant == 0)
      {
        double r = -b/2.f;
        double c1 = theta0 - f/k;
        double c2 = dtheta0 - c1*r;
        deltatheta = (c1 + c2*DeltaTime)*std::exp(r*DeltaTime) + f/k;
        angularvelocity = (c1*r + c2 + c2*r*DeltaTime)*std::exp(r*DeltaTime);
        SOLVER_LOG(Log, "r %f, c1 %f, c2 %f, deltatheta %f", r, c1, c2, deltatheta);
      }
      else
      {
        double gamma = -b/2.f;
        double mu = FMath::Sqrt(FMath::Abs(discriminant))/2.f;
        double c1 = theta0 - f/k;
        double c2 = (c1*gamma - dtheta0)/mu;
        deltatheta =
            c1*std::exp(gamma*DeltaTime)*std::cos(mu*DeltaTime) +
            c2*std::exp(gamma*DeltaTime)*std::sin(mu*DeltaTime) + f/k;
        angularvelocity =
            c1*std::exp(gamma*DeltaTime)*(gamma*std::cos(mu*DeltaTime) + mu*std::sin(mu*DeltaTime)) +
            c2*std::exp(gamma*DeltaTime)*(gamma*std::sin(mu*DeltaTime) - mu*std::cos(mu*DeltaTime));
        SOLVER_LOG(Log, "gamma %f, mu %f, c1 %f, c2 %f, deltatheta %f", gamma, mu, c1, c2, deltatheta);
      }
      angularaccel = f - b*angularvelocity - k*deltatheta;
      if (!FMath::IsNaN(deltatheta))
      {
        NewTheta(i) = deltatheta;
        if (angularvelocity > 1e-4)
        {
          NewThetaVelocity(i) = angularvelocity;
        }
        if (angularaccel > 1e-2)
        {
          NewThetaAccel(i) = angularaccel;
        }
      }
    }
    Eigen::Vector3d FinalNewTheta = U*NewTheta;
    Eigen::Vector3d FinalNewThetaVelocity = U*NewThetaVelocity;
    Eigen::Vector3d FinalNewThetaAccel = U*NewThetaAccel;

    auto NewPitch = FMath::RadiansToDegrees(FinalNewTheta(1));
    auto NewYaw = FMath::RadiansToDegrees(FinalNewTheta(2));
    auto NewRoll = FMath::RadiansToDegrees(FinalNewTheta(0));
    
    FRotator NewAngularVelocity = EigenVectorToRotator(FinalNewThetaVelocity);
    FRotator NewAngularAccel = EigenVectorToRotator(FinalNewThetaAccel);

    FRotator NewAngle(
            RestRotator.Pitch + NewPitch,
            RestRotator.Yaw - NewYaw,
            RestRotator.Roll + NewRoll);
    SOLVER_LOG(Log, "FinalNewTheta \n %s \n FinalNewThetaVelocity \n %s \n FinalNewThetaAccel \n %s",
        *EigenToFString(FinalNewTheta), *EigenToFString(FinalNewThetaVelocity), *EigenToFString(FinalNewThetaAccel));
    SOLVER_LOG(Log, "New angle %s, new angular velocity %s, new angular accel %s",
        *NewAngle.ToString(), *NewAngularVelocity.ToString(), *NewAngularAccel.ToString());
    Joint.Transform.SetRotation(NewAngle.Quaternion());
    Joint.AngularVelocity = NewAngularVelocity;
    Joint.AngularAcceleration = NewAngularAccel;
  }
}

void USpringBasedVegetationComponent::TickComponent(
    float DeltaTime,
    enum ELevelTick TickType,
    FActorComponentTickFunction * ThisTickFunction)
{
  TRACE_CPUPROFILER_EVENT_SCOPE(USpringBasedVegetationComponent::TickComponent);
  Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

  float DeltaTimeFinal = DeltaTime;
  if (DeltaTimeOverride > 0)
    DeltaTimeFinal = DeltaTimeOverride;

  std::vector<FJointProperties> JointPropertiesList;
  JointPropertiesList.resize(Skeleton.Joints.Num());
  std::vector<FJointProperties> JointLocalPropertiesList;
  JointLocalPropertiesList.resize(Skeleton.Joints.Num());

  ComputePerJointProperties(JointLocalPropertiesList, JointPropertiesList);

  ResolveContactsAndCollisions(JointLocalPropertiesList, JointPropertiesList);

  ComputeCompositeBodyContribution(JointLocalPropertiesList, JointPropertiesList);

  ComputeFictitiousForces(JointLocalPropertiesList, JointPropertiesList);

  SolveEquationOfMotion(JointPropertiesList, DeltaTimeFinal);

  UpdateSkeletalMesh();
  UpdateGlobalTransform();
  Skeleton.ClearExternalForces();
}

void USpringBasedVegetationComponent::OnCollisionEvent(
    UPrimitiveComponent* HitComponent,
    AActor* OtherActor,
    UPrimitiveComponent* OtherComponent,
    FVector NormalImpulse,
    const FHitResult& Hit)
{
  // prevent self collision
  if (OtherActor == GetOwner())
    return;
  // prevent collision with other tree actors
  if(OtherActor->GetComponentByClass(USpringBasedVegetationComponent::StaticClass()) != nullptr)
    return;
  ACarlaWheeledVehicle* Vehicle = nullptr;
  if (DebugEnableAllCollisions)
  {
    if (!IsValid(OtherActor))
      return;
  }
  else
  {
    Vehicle = Cast<ACarlaWheeledVehicle>(OtherActor);
    if (!IsValid(Vehicle))
      return;
  }
  COLLISION_LOG(LogCarla, Log, TEXT("Collision with bone %s, with impulse %s"), *Hit.MyBoneName.ToString(), *NormalImpulse.ToString());
  Skeleton.AddForce(Hit.MyBoneName.ToString(), NormalImpulse);
}

void USpringBasedVegetationComponent::OnBeginOverlapEvent(
    UPrimitiveComponent* OverlapComponent,
    AActor* OtherActor,
    UPrimitiveComponent* OtherComponent,
    int32 OtherBodyIndex,
    bool bFromSweep,
    const FHitResult& SweepResult)
{
  // prevent self collision
  if (OtherActor == GetOwner())
    return;
  // prevent collision with other tree actors
  if(OtherActor->GetComponentByClass(USpringBasedVegetationComponent::StaticClass()) != nullptr)
    return;
  ACarlaWheeledVehicle* Vehicle = nullptr;
  if (DebugEnableAllCollisions)
  {
    if (!IsValid(OtherActor))
      return;
  }
  else
  {
    Vehicle = Cast<ACarlaWheeledVehicle>(OtherActor);
    if (!IsValid(Vehicle))
      return;
  }
  
  if (!OverlappingActors.Contains(OtherActor))
  {
    OverlappingActors.Add(OtherActor);
  }
  TArray<UPrimitiveComponent*>& OverlappingCapsules = OverlappingActors.FindOrAdd(OtherActor);
  if (!OverlappingCapsules.Contains(OverlapComponent))
  {
    OverlappingCapsules.Add(OverlapComponent);
  }
}

void USpringBasedVegetationComponent::OnEndOverlapEvent(
    UPrimitiveComponent* OverlapComponent,
    AActor* OtherActor,
    UPrimitiveComponent* OtherComponent,
    int32 OtherBodyIndex)
{
  // prevent self collision
  if (OtherActor == GetOwner())
    return;
  // prevent collision with other tree actors
  if(OtherActor->GetComponentByClass(USpringBasedVegetationComponent::StaticClass()) != nullptr)
    return;
  ACarlaWheeledVehicle* Vehicle = nullptr;
  if (DebugEnableAllCollisions)
  {
    if (!IsValid(OtherActor))
      return;
  }
  else
  {
    Vehicle = Cast<ACarlaWheeledVehicle>(OtherActor);
    if (!IsValid(Vehicle))
      return;
  }

  if (!OverlappingActors.Contains(OtherActor))
    return;
  TArray<UPrimitiveComponent*>& OverlappingCapsules = OverlappingActors.FindOrAdd(OtherActor);
  if (OverlappingCapsules.Contains(OverlapComponent))
  {
    OverlappingCapsules.RemoveSingle(OverlapComponent);
    if (OverlappingCapsules.Num() == 0)
    {
      OverlappingActors.Remove(OtherActor);
    }
  }
}

void USpringBasedVegetationComponent::UpdateSkeletalMesh()
{
  TRACE_CPUPROFILER_EVENT_SCOPE(USpringBasedVegetationComponent::UpdateSkeletalMesh);
  // get the walker animation class
  auto *AnimInst = SkeletalMesh->GetAnimInstance();
  if (!AnimInst) return;
  UWalkerAnim *WalkerAnim = Cast<UWalkerAnim>(AnimInst);
  if (!WalkerAnim) return;

  // if pose is empty, then get a first version
  if (WalkerAnim->Snap.BoneNames.Num() == 0)
  {
    // get current pose
    SkeletalMesh->SnapshotPose(WalkerAnim->Snap);
  }

  TMap<FName, FTransform> JointsMap;
  for (int i=0; i<Skeleton.Joints.Num(); ++i)
  {
    FSkeletonJoint& Joint = Skeleton.Joints[i];
    FName JointName = FName(*Joint.JointName);
    JointsMap.Add(JointName, Joint.Transform);
  }

  // assign common bones
  for (int i=0; i<WalkerAnim->Snap.BoneNames.Num(); ++i)
  {
    FTransform *Trans = JointsMap.Find(WalkerAnim->Snap.BoneNames[i]);
    if (Trans)
    {
      WalkerAnim->Snap.LocalTransforms[i] = *Trans;
    }
  }
}

void USpringBasedVegetationComponent::UpdateGlobalTransform()
{
  TRACE_CPUPROFILER_EVENT_SCOPE(USpringBasedVegetationComponent::UpdateGlobalTransform);
  FTransform InitialTransform = SkeletalMesh->GetOwner()->GetActorTransform();
  FSkeletonJoint& RootJoint = Skeleton.Joints[0];
  RootJoint.GlobalTransform = RootJoint.Transform * InitialTransform;
  RootJoint.GolbalInverseTransform = RootJoint.GlobalTransform.Inverse();
  for (int i = 1; i < Skeleton.RootToEndOrder.Num(); i++)
  {
    int Id = Skeleton.RootToEndOrder[i];
    FSkeletonJoint& Joint = Skeleton.Joints[Id];
    FSkeletonJoint& ParentJoint = Skeleton.Joints[Joint.ParentId];
    FTransform Transform = Joint.Transform * ParentJoint.GlobalTransform;
    Joint.GlobalTransform = Transform;
    Joint.GolbalInverseTransform = Transform.Inverse();
  }
}
