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
#include <unordered_set>
#include <vector>

// disable warnings for eigen 3.1.0
#if defined(__clang__)
#  pragma clang diagnostic push
#  pragma clang diagnostic ignored "-Wdeprecated-register"
#  pragma clang diagnostic ignored "-Wmisleading-indentation"
#  pragma clang diagnostic ignored "-Wint-in-bool-context"
#  pragma clang diagnostic ignored "-Wdeprecated-declarations"
#  pragma clang diagnostic ignored "-Wshadow"
#endif
#include <Eigen/Dense>
#include <Eigen/Cholesky>
#include <Eigen/Eigenvalues>
#if defined(__clang__)
#  pragma clang diagnostic pop
#endif

#include "carla/rpc/String.h"
#include <cmath>
#include <sstream>

#define SPRINGVEGETATIONLOGS 0

#if SPRINGVEGETATIONLOGS
#define SP_LOG(Level, Msg, ...) UE_LOG(LogCarla, Level, TEXT(Msg), ##__VA_ARGS__)
#else
#define SP_LOG(...)
#endif
 
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
  return EigenMatrix;
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

void FSkeletonHierarchy::ComputeChildrenJointsAndBones()
{
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
    SP_LOG(Warning, "Joint %d, Children: %s.", i, *ChildrenIds);
  }
}

void FSkeletonHierarchy::ComputeEndJoints()
{
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
  SP_LOG(Warning, "Tree order: %s", *IdOrder);
  SP_LOG(Warning, "Tree order (names): %s", *NameOrder);
  SP_LOG(Warning, "Num elements: %d", EndToRootOrder.Num());
}

void FSkeletonHierarchy::AddForce(const FString& BoneName, const FVector& Force)
{
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

void USpringBasedVegetationComponent::BeginPlay()
{
  Super::BeginPlay();
  SP_LOG(Warning, "USpringBasedVegetationComponent::BeginPlay");
  UActorComponent* Component = GetOwner()->GetComponentByClass(USkeletalMeshComponent::StaticClass());
  SkeletalMesh = Cast<USkeletalMeshComponent>(Component);
  if (!SkeletalMesh)
  {
    SetComponentTickEnabled(false);
    SP_LOG(Error, "Could not find skeletal mesh component.");
    return;
  }

  // set callbacks
  SkeletalMesh->OnComponentHit.AddDynamic(this, &USpringBasedVegetationComponent::OnCollisionEvent);
  
  // Get skeleton hierarchy
  TArray<FName> BoneNames;
  SkeletalMesh->GetBoneNames(BoneNames);
  for (int32 i = 0; i < BoneNames.Num(); i++)
  {
    FName& BoneName = BoneNames[i];
    FString BoneNameString = BoneName.ToString();
    FName ParentBoneName = SkeletalMesh->GetParentBone(BoneName);
    int32 ParentIndex = SkeletalMesh->GetBoneIndex(ParentBoneName);
    Skeleton.Joints.Add(FSkeletonJoint{i, ParentIndex, BoneNameString, FixedJointsList.Contains(BoneNameString)});
    SP_LOG(Log, "Added bone %s with id %d and parent %d", *BoneNameString, i, ParentIndex);
  }

  Skeleton.ComputeChildrenJointsAndBones();
  Skeleton.ComputeEndJoints();

  // Get resting pose for bones
  auto *AnimInst = SkeletalMesh->GetAnimInstance();
  if (!AnimInst)
  {
    SP_LOG(Error, "Could not get animation instance.");
    return;
  }
  UWalkerAnim *WalkerAnim = Cast<UWalkerAnim>(AnimInst);
  if (!WalkerAnim)
  {
    SP_LOG(Error, "Could not get UWalkerAnim.");
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
    SP_LOG(Log, "Getting info for bone %s, %f, %f, %f, %f", *Joint.JointName, Joint.RestingAngles.Pitch, Joint.RestingAngles.Yaw, Joint.RestingAngles.Roll);
    if(i > 0)
    {
      FSkeletonJoint& ParentJoint = Skeleton.Joints[Joint.ParentId];
      FVector BoneCOM = Joint.Transform.GetLocation()*0.5f;
      float BoneLength = Joint.Transform.GetLocation().Size();
      ParentJoint.Bones.Add({10, BoneLength, BoneCOM});
    }
  }
  for (int i=0; i<TempSnapshot.BoneNames.Num(); ++i)
  {
    SP_LOG(Log, "Joint list: %s", *TempSnapshot.BoneNames[i].ToString());
  }

  UpdateGlobalTransform();
  GenerateCollisionCapsules();
}

void USpringBasedVegetationComponent::GenerateCollisionCapsules()
{
  for (FSkeletonJoint& Joint : Skeleton.Joints)
  {
    for (FSkeletonBone& Bone : Joint.Bones)
    {
      if (Bone.Length < 1)
      {
        continue;
      }
      UCapsuleComponent* Capsule = NewObject<UCapsuleComponent>(GetOwner());
      Capsule->AttachToComponent(SkeletalMesh, FAttachmentTransformRules::KeepRelativeTransform, FName(*Joint.JointName));
      Capsule->RegisterComponent();
      FTransform CapsuleTransform(FRotator(90, 0, 0), Bone.CenterOfMass, FVector(1,1,1));
      Capsule->SetRelativeTransform(CapsuleTransform);
      Capsule->SetCapsuleHalfHeight(Bone.Length*0.5f);
      Capsule->SetCapsuleRadius(6);
      Capsule->SetGenerateOverlapEvents(true);
      Capsule->SetCollisionProfileName("OverlapAll");
      Capsule->OnComponentBeginOverlap.AddDynamic(this, &USpringBasedVegetationComponent::OnBeginOverlapEvent);
      Capsule->OnComponentEndOverlap.AddDynamic(this, &USpringBasedVegetationComponent::OnEndOverlapEvent);
      BoneCapsules.Add(Capsule);
      CapsuleToJointId.Add(Capsule, Joint.JointId);
    }
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

struct FJointProperties
{
  float Mass = 0.0;
  Eigen::Matrix3d InertiaTensor = Eigen::Matrix3d::Zero();
  Eigen::Vector3d Force = Eigen::Vector3d::Zero();
  Eigen::Vector3d Torque = Eigen::Vector3d::Zero();
  Eigen::Vector3d FictitiousTorque = Eigen::Vector3d::Zero();
  Eigen::Vector3d CenterOfMass = Eigen::Vector3d::Zero();
  Eigen::Matrix3d JointToGlobalMatrix = Eigen::Matrix3d::Zero();
  Eigen::Vector3d AngularVelocity = Eigen::Vector3d::Zero();
  Eigen::Vector3d LinearVelocity = Eigen::Vector3d::Zero();
  Eigen::Vector3d AngularAcceleration = Eigen::Vector3d::Zero();
  Eigen::Vector3d LinearAcceleration = Eigen::Vector3d::Zero();
  Eigen::Vector3d LocalAngularAcceleration = Eigen::Vector3d::Zero();
};


void USpringBasedVegetationComponent::TickComponent(
    float DeltaTime,
    enum ELevelTick TickType,
    FActorComponentTickFunction * ThisTickFunction)
{
  Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

  float DeltaTimeFinal = DeltaTime;
  if (DeltaTimeOverride > 0)
  {
    DeltaTimeFinal = DeltaTimeOverride;
  }
  std::vector<FJointProperties> JointPropertiesList;
  JointPropertiesList.resize(Skeleton.Joints.Num());
  std::vector<FJointProperties> JointLocalPropertiesList;
  JointLocalPropertiesList.resize(Skeleton.Joints.Num());


  // per joint computation
  for(FSkeletonJoint& Joint : Skeleton.Joints)
  {
    FJointProperties& Properties = JointLocalPropertiesList[Joint.JointId];
    Eigen::Vector3d JointGlobalPosition = ToEigenVector(Joint.GlobalTransform.GetLocation())/100.f;
    Properties.JointToGlobalMatrix = ToEigenMatrix(Joint.GlobalTransform);
    JointPropertiesList[Joint.JointId].JointToGlobalMatrix = Properties.JointToGlobalMatrix;
    if (!Joint.Bones.Num())
    {
      continue;
    }
    // COM and mass
    for (FSkeletonBone& Bone : Joint.Bones)
    {
      Properties.Mass += Bone.Mass;
      FVector GlobalCenterOfMass = Joint.GlobalTransform.TransformPositionNoScale(Bone.CenterOfMass);
      Properties.CenterOfMass += Bone.Mass*ToEigenVector(GlobalCenterOfMass)/100.f;
    }
    Properties.CenterOfMass = Properties.CenterOfMass/Properties.Mass;

    FVector UECOM = FVector(Properties.CenterOfMass(0),Properties.CenterOfMass(1),-Properties.CenterOfMass(2))*100.f;
    // DrawDebugPoint(GetWorld(),
    //     UECOM,
    //     5,
    //     FColor(0,0,255),
    //     false,
    //     0.1);
    // SP_LOG(Log, "Local COM: %s", *UECOM.ToString());
    // force
    Eigen::Vector3d GravityForce = ToEigenVector(Gravity)/100.f; // world space gravity
    Properties.Force = Properties.Mass * GravityForce + ToEigenVector(Joint.ExternalForces)/100.f;
    // torque
    Properties.Torque = (Properties.CenterOfMass - JointGlobalPosition).cross(Properties.Force);
    // inertia tensor
    for (FSkeletonBone& Bone : Joint.Bones)
    {
      if (Bone.Length < 1)
      {
        continue;
      }
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
      {
        LocalV2 = LocalV1.cross(Eigen::Vector3d(0,0,1));
      }
      LocalV2.normalize();
      Eigen::Vector3d LocalV3 = LocalV1.cross(LocalV2);
      Eigen::Matrix3d LocalToJointMatrix;
      LocalToJointMatrix << LocalV1, LocalV2, LocalV3;
      Eigen::Matrix3d JointSpaceInertia = LocalToJointMatrix*LocalCylinderInertia*(LocalToJointMatrix.transpose());
      Properties.InertiaTensor += Properties.JointToGlobalMatrix*JointSpaceInertia*Properties.JointToGlobalMatrix.transpose();
      Eigen::Vector3d BoneCenterWorldSpace = ToEigenVector(Joint.GlobalTransform.TransformPositionNoScale(Bone.CenterOfMass))/100.0;
      Properties.InertiaTensor += Properties.Mass*OuterProduct(Properties.CenterOfMass - BoneCenterWorldSpace, Properties.CenterOfMass - BoneCenterWorldSpace);
    }
    SP_LOG(Log, "Local Joint: %s \n Inertia \n %s \n Force \n %s \n Torque \n %s \n COM: \n %s \n Mass %f", *Joint.JointName, *EigenToFString(Properties.InertiaTensor), *EigenToFString(Properties.Force), *EigenToFString(Properties.Torque), *EigenToFString(Properties.CenterOfMass), Properties.Mass);
  }

  // accumulative computation
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

    FVector UECOM = FVector(JointProperties.CenterOfMass(0),JointProperties.CenterOfMass(1),-JointProperties.CenterOfMass(2))*100.f;
    // DrawDebugPoint(GetWorld(),
    //     UECOM,
    //     5,
    //     FColor(255,0,0),
    //     false,
    //     0.1);
    SP_LOG(Log, "Local COM: %s", *UECOM.ToString());

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
    SP_LOG(Log, "Accumulated Joint: %s \n Inertia \n %s \n Force \n %s \n Torque \n %s \n COM: \n %s \n Mass %f", *Joint.JointName, *EigenToFString(JointProperties.InertiaTensor), *EigenToFString(JointProperties.Force), *EigenToFString(JointProperties.Torque), *EigenToFString(JointProperties.CenterOfMass), JointProperties.Mass);
  }

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

    SP_LOG(Log, "Joint: %s \n Position \n %s \n Velocity \n %s \n Acceleration \n %s \n Fictitious forces: \n %s", *Joint.JointName, *EigenToFString(CompositeCenterOfMass), *EigenToFString(CompositeLinearVelocity), *EigenToFString(CompositeLinearAcceleration), *EigenToFString(JointProperties.FictitiousTorque));
  }

  // Contacts and collisions
  for (auto& ActorCapsules : OverlappingActors)
  {
    AActor* CollidingActor = ActorCapsules.Key;
    UPrimitiveComponent* Primitive = Cast<UPrimitiveComponent>(CollidingActor->GetRootComponent());
    if (!Primitive)
    {
      continue;
    }
    FVector Impulse = (Primitive->GetMass() * Primitive->GetComponentVelocity());
    Eigen::Vector3d CollisionImpulse = ToEigenVector(Impulse)/100.f;
    TArray<UPrimitiveComponent*>& CollidingCapsules = ActorCapsules.Value;
    for (UPrimitiveComponent* Capsule : CollidingCapsules)
    {
      Primitive->AddForce(-Impulse);
      int JointId = CapsuleToJointId[Capsule];
      FSkeletonJoint& Joint = Skeleton.Joints[JointId];
      FJointProperties& JointProperties = JointPropertiesList[Joint.JointId];
      Eigen::Vector3d JointGlobalPosition = ToEigenVector(Joint.GlobalTransform.GetLocation())/100.f;
      Eigen::Vector3d CapsulePosition = ToEigenVector(Capsule->GetComponentLocation())/100.f;
      Eigen::Vector3d ColliderPosition = ToEigenVector(Primitive->GetComponentLocation())/100.f;
      // Prevent too much overlap 
      Eigen::Vector3d CollisionForces = (CapsulePosition - ColliderPosition)*CollisionForceParameter;
      Primitive->AddForce(-ToUnrealVector(CollisionForces)*100.f);
      Eigen::Vector3d CollisionTorque = (JointProperties.CenterOfMass - JointGlobalPosition).cross(CollisionImpulse + CollisionForces);
      // Contact forces
      FRotator CurrRotator = Joint.Transform.Rotator();
      FRotator RestRotator = Joint.RestingAngles;
      FRotator DeltaRotator (
          CurrRotator.Pitch - RestRotator.Pitch, 
          CurrRotator.Yaw - RestRotator.Yaw, 
          CurrRotator.Roll - RestRotator.Roll);
      Eigen::Vector3d SpringTorque = SpringStrength*RotatorToEigenVector(DeltaRotator);
      Eigen::Vector3d JointCapsuleVector = JointGlobalPosition - CapsulePosition;
      Eigen::Vector3d SpringForce = SpringTorque.cross(JointCapsuleVector)/JointCapsuleVector.squaredNorm();
      Primitive->AddForce(ToUnrealVector(SpringForce)*100.f);
      JointProperties.Torque += CollisionTorque;
    }
  }

  // solver
  for (FSkeletonJoint& Joint : Skeleton.Joints)
  {
    if (!Joint.Bones.Num() || Joint.bIsStatic)
    {
      continue;
    }
    FJointProperties& JointProperties = JointPropertiesList[Joint.JointId];
    float Mass = JointProperties.Mass;
    Eigen::Vector3d CenterToJoint = JointProperties.CenterOfMass - ToEigenVector(Joint.GlobalTransform.GetLocation())/100.f;
    Eigen::Matrix3d GlobalToJointMatrix = JointProperties.JointToGlobalMatrix.transpose();
    Eigen::Matrix3d JointSpaceIntertiaTensor = JointProperties.Mass*OuterProduct(CenterToJoint, CenterToJoint)
        + GlobalToJointMatrix*JointProperties.InertiaTensor*GlobalToJointMatrix.transpose();
    Eigen::Vector3d Torque = GlobalToJointMatrix*JointProperties.Torque + JointProperties.FictitiousTorque;
    // build differential equation
    Eigen::Matrix3d I = JointSpaceIntertiaTensor;
    float beta = Beta;
    Eigen::Matrix3d K;
    K << SpringStrength,0.f,0.f,
         0.f,SpringStrength,0.f,
         0.f,0.f,SpringStrength;
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
    Eigen::Vector3d Coeffsb = beta*Lambda*Eigen::Vector3d(1,1,1);
    Eigen::Vector3d Coeffsk = Lambda*Eigen::Vector3d(1,1,1);
    Eigen::Vector3d Coeffsf = U.transpose()*Torque;
    FString StringI = EigenToFString(I);
    FString StringTorque = EigenToFString(Torque);
    FString StringU = EigenToFString(U);
    FString StringLambda = EigenToFString(Lambda);
    FString StringGlobalToJoint = EigenToFString(GlobalToJointMatrix);
    FString StringCenterToJoint = EigenToFString(CenterToJoint);
    SP_LOG(Log, "Bone %s: \n I: \n %s \n Tau: \n %s \n U: \n %s \n Lambda: \n %s \n X: \n %s \n GlobalToJoint \n %s \n CenterToJoint \n %s", *Joint.JointName, *StringI, *StringTorque, *StringU, *StringLambda, *EigenToFString(X), *StringGlobalToJoint, *StringCenterToJoint);

    FRotator CurrRotator = Joint.Transform.Rotator();
    FRotator RestRotator = Joint.RestingAngles;
    FRotator AngularVelocity = Joint.AngularVelocity;
    FRotator DeltaRotator (
        CurrRotator.Pitch - RestRotator.Pitch, 
        CurrRotator.Yaw - RestRotator.Yaw, 
        CurrRotator.Roll - RestRotator.Roll);
    Eigen::Vector3d InitialTheta = Uinv*RotatorToEigenVector(DeltaRotator);
    Eigen::Vector3d InitialThetaVelocity = Uinv*RotatorToEigenVector(AngularVelocity);
    SP_LOG(Log, "Old angle for joint %s, %s", *Joint.JointName, *CurrRotator.ToString());
    SP_LOG(Log, "InitialTheta \n %s", *EigenToFString(InitialTheta));
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

      SP_LOG(Log, "Bone %s, component %d, b %f, k %f, f %f, discriminant %f, theta0 %f", *Joint.JointName, i, b, k, f, discriminant, theta0);
      if(k == 0)
      {
        NewTheta(i) = theta0;
        SP_LOG(Log, "In Bone %s, K is 0 in component %d", *Joint.JointName, i);
      }
      double dtheta0 = InitialThetaVelocity(i); // compute
      double deltatheta = 0;
      double angulavelocity = 0;
      float angularaccel = 0;
      if (discriminant > 0)
      {
        double r1 = (-b + FMath::Sqrt(discriminant))*0.5f;
        double r2 = (-b - FMath::Sqrt(discriminant))*0.5f;
        double c1 = (r2*(theta0 - f/k) - dtheta0)/(r2-r1);
        double c2 = (dtheta0 - c1*r1)/r2;
        deltatheta = c1*std::exp(r1*DeltaTimeFinal) + c2*std::exp(r2*DeltaTimeFinal) + f/k;
        angulavelocity = c1*r1*std::exp(r1*DeltaTimeFinal) + c2*r2*std::exp(r2*DeltaTimeFinal);
        SP_LOG(Log, "r1 %f, r2 %f, c1 %f, c2 %f, deltatheta %f", r1, r2, c1, c2, deltatheta);
      }
      else if (discriminant == 0)
      {
        double r = -b/2.f;
        double c1 = theta0 - f/k;
        double c2 = dtheta0 - c1*r;
        deltatheta = (c1 + c2*DeltaTimeFinal)*std::exp(r*DeltaTimeFinal) + f/k;
        angulavelocity = (c1*r + c2 + c2*r*DeltaTimeFinal)*std::exp(r*DeltaTimeFinal);
        SP_LOG(Log, "r %f, c1 %f, c2 %f, deltatheta %f", r, c1, c2, deltatheta);
      }
      else
      {
        double gamma = -b/2.f;
        double mu = FMath::Sqrt(FMath::Abs(discriminant))/2.f;
        double c1 = theta0 - f/k;
        double c2 = (c1*gamma - dtheta0)/mu;
        deltatheta =
            c1*std::exp(gamma*DeltaTimeFinal)*std::cos(mu*DeltaTimeFinal) +
            c2*std::exp(gamma*DeltaTimeFinal)*std::sin(mu*DeltaTimeFinal) + f/k;
        angulavelocity =
            c1*std::exp(gamma*DeltaTimeFinal)*(gamma*std::cos(mu*DeltaTimeFinal) + mu*std::sin(mu*DeltaTimeFinal)) +
            c2*std::exp(gamma*DeltaTimeFinal)*(gamma*std::sin(mu*DeltaTimeFinal) - mu*std::cos(mu*DeltaTimeFinal));
        SP_LOG(Log, "gamma %f, mu %f, c1 %f, c2 %f, deltatheta %f", gamma, mu, c1, c2, deltatheta);
      }
      angularaccel = f - b*angulavelocity - k*deltatheta;
      if (!FMath::IsNaN(deltatheta))
      {
        NewTheta(i) = deltatheta;
        if (angulavelocity > 1e-4)
        {
          NewThetaVelocity(i) = angulavelocity;
        }
        if (angularaccel > 1e-4)
        {
          NewThetaAccel(i) = angularaccel;
        }
      }
    }
    Eigen::Vector3d FinalNewTheta = U*NewTheta;
    Eigen::Vector3d FinalNewThetaVelocity = U*NewThetaVelocity;
    Eigen::Vector3d FinalNewThetaAccel = U*NewThetaAccel;
    FRotator NewAngle(
        RestRotator.Pitch + FMath::RadiansToDegrees(FinalNewTheta(1)),
        RestRotator.Yaw - FMath::RadiansToDegrees(FinalNewTheta(2)),
        RestRotator.Roll + FMath::RadiansToDegrees(FinalNewTheta(0)));
    FRotator NewAngularVelocity = EigenVectorToRotator(FinalNewThetaVelocity);
    FRotator NewAngularAccel = EigenVectorToRotator(FinalNewThetaAccel);

    SP_LOG(Log, "FinalNewTheta \n %s \n FinalNewThetaVelocity \n %s \n FinalNewThetaAccel \n %s",
        *EigenToFString(FinalNewTheta), *EigenToFString(FinalNewThetaVelocity), *EigenToFString(FinalNewThetaAccel));
    SP_LOG(Log, "New angle %s, new angular velocity %s, new angular accel %s",
        *NewAngle.ToString(), *NewAngularVelocity.ToString(), *NewAngularAccel.ToString());
    Joint.Transform.SetRotation(NewAngle.Quaternion());
    Joint.AngularVelocity = NewAngularVelocity;
    Joint.AngularAcceleration = NewAngularAccel;
  }

  // SP_LOG(Log, "Vegetation tick");
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
  UE_LOG(LogCarla, Log, TEXT("Collision with bone %s, with impulse %s"), *Hit.MyBoneName.ToString(), *NormalImpulse.ToString());
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
  if(OtherActor == GetOwner())
  {
    return;
  }
  // OverlappingActors.Add(OtherActor);
  TArray<UPrimitiveComponent*>& OverlappingCapsules = OverlappingActors.FindOrAdd(OtherActor);
  OverlappingCapsules.Add(OverlapComponent);
}

void USpringBasedVegetationComponent::OnEndOverlapEvent(
    UPrimitiveComponent* OverlapComponent,
    AActor* OtherActor,
    UPrimitiveComponent* OtherComponent,
    int32 OtherBodyIndex)
{
  if(OtherActor == GetOwner())
  {
    return;
  }
  // OverlappingActors.RemoveSingle(OtherActor);
  TArray<UPrimitiveComponent*>& OverlappingCapsules = OverlappingActors.FindOrAdd(OtherActor);
  OverlappingCapsules.RemoveSingle(OverlapComponent);
  if (OverlappingCapsules.Num() == 0)
  {
    OverlappingActors.Remove(OtherActor);
  }
}

void USpringBasedVegetationComponent::UpdateSkeletalMesh()
{
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

  FTransform InitialTransform = SkeletalMesh->GetOwner()->GetActorTransform();
  FSkeletonJoint& RootJoint = Skeleton.Joints[0];
  RootJoint.GlobalTransform = RootJoint.Transform * InitialTransform;
  RootJoint.GolbalInverseTransform = RootJoint.GlobalTransform.Inverse();
  // RootJoint.GlobalAngularVelocity = RootJoint.GlobalTransform.TransformRotation(FQuat(RootJoint.AngularVelocity)).Rotator();
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
