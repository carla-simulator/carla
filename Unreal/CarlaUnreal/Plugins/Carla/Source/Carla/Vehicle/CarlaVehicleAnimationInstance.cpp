// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "Carla/Vehicle/CarlaVehicleAnimationInstance.h"
#include "Carla.h"
#include "Carla/Vehicle/CarlaWheeledVehicle.h"

#include <util/ue-header-guard-begin.h>
#include "AnimationRuntime.h"
#include "ChaosVehicleWheel.h"
#include "ChaosWheeledVehicleMovementComponent.h"
#include <util/ue-header-guard-end.h>

static float RollWheel(float SpinAngle, float Travel, float Radius, float MaxTravel)
{
  const bool bHasRadius = Radius > UE_KINDA_SMALL_NUMBER;
  const bool bTeleported = FMath::Abs(Travel) > MaxTravel;
  if (!bHasRadius || bTeleported)
  {
    return SpinAngle;
  }
  return FMath::UnwindDegrees(SpinAngle - FMath::RadiansToDegrees(Travel / Radius));
}

void UCarlaVehicleAnimationInstance::NativeUpdateAnimation(float DeltaSeconds)
{
  Super::NativeUpdateAnimation(DeltaSeconds);

  const ACarlaWheeledVehicle *Vehicle = Cast<ACarlaWheeledVehicle>(GetOwningActor());
  if (Vehicle == nullptr)
  {
    return;
  }
  UpdateWheelAnimationOverride(*Vehicle);
  if (ShouldRollWheels(*Vehicle))
  {
    if (!bRollingWheels)
    {
      BeginRolling();
      bRollingWheels = true;
    }
    RollWheels(*Vehicle, DeltaSeconds);
  }
  else
  {
    bRollingWheels = false;
    // So the first frame of rolling turns the wheels by that frame's travel.
    TrackWheelLocations();
  }
  UpdateAxleOffsets(*Vehicle);
  UpdateBodyOffset(*Vehicle);
}

void UCarlaVehicleAnimationInstance::UpdateAxleOffsets(const ACarlaWheeledVehicle &Vehicle)
{
  FrontAxleOffset = FVector::ZeroVector;
  RearAxleOffset = FVector::ZeroVector;
  FrontAxleRadius = 0.0f;
  RearAxleRadius = 0.0f;
  const UChaosWheeledVehicleMovementComponent *Movement = GetWheeledVehicleComponent();
  if (Movement == nullptr)
  {
    return;
  }
  const TArray<FWheelAnimationData> &WheelPoses = GetWheelPoses();
  // Not Movement->Wheels: disabling physics destroys them.
  const int32 NumWheels = FMath::Min(Movement->WheelSetups.Num(), WheelPoses.Num());
  const USkeletalMesh *SkeletalMesh = GetSkelMeshComponent()->GetSkeletalMeshAsset();
  const bool bReadRestLocations = !bHasAxleRestLocations && SkeletalMesh != nullptr;
  if (bReadRestLocations)
  {
    FrontAxleRestLocation = FVector::ZeroVector;
    RearAxleRestLocation = FVector::ZeroVector;
    const FReferenceSkeleton &RefSkeleton = SkeletalMesh->GetRefSkeleton();
    const int32 BodyBoneIndex =
        BodyOffsetBone.IsNone() ? INDEX_NONE : RefSkeleton.FindBoneIndex(BodyOffsetBone);
    BodyRestLocation = FAnimationRuntime::GetComponentSpaceTransformRefPose(
        RefSkeleton, BodyBoneIndex != INDEX_NONE ? BodyBoneIndex : 0).GetLocation();
  }
  int32 NumFront = 0;
  int32 NumRear = 0;
  for (int32 i = 0; i < NumWheels; ++i)
  {
    FVector RestLocation = FVector::ZeroVector;
    if (bReadRestLocations)
    {
      const FReferenceSkeleton &RefSkeleton = SkeletalMesh->GetRefSkeleton();
      const int32 BoneIndex = RefSkeleton.FindBoneIndex(Movement->WheelSetups[i].BoneName);
      if (BoneIndex != INDEX_NONE)
      {
        RestLocation =
            FAnimationRuntime::GetComponentSpaceTransformRefPose(RefSkeleton, BoneIndex).GetLocation();
      }
    }
    const UChaosVehicleWheel *Wheel = Movement->Wheels.IsValidIndex(i) ? Movement->Wheels[i].Get() : nullptr;
    if (Wheel == nullptr && Movement->WheelSetups[i].WheelClass != nullptr)
    {
      // Disabling physics destroys the wheels; their class keeps the setup.
      Wheel = Movement->WheelSetups[i].WheelClass->GetDefaultObject<UChaosVehicleWheel>();
    }
    EAxleType Axle = Wheel != nullptr ? Wheel->AxleType : EAxleType::Undefined;
    if (Axle == EAxleType::Undefined)
    {
      // Wheel setups list the front wheels first.
      Axle = i < NumWheels / 2 ? EAxleType::Front : EAxleType::Rear;
    }
    if (Axle == EAxleType::Front)
    {
      FrontAxleOffset += WheelPoses[i].LocOffset;
      FrontAxleRestLocation += RestLocation;
      FrontAxleRadius += Vehicle.GetWheelRadius(i);
      ++NumFront;
    }
    else
    {
      RearAxleOffset += WheelPoses[i].LocOffset;
      RearAxleRestLocation += RestLocation;
      RearAxleRadius += Vehicle.GetWheelRadius(i);
      ++NumRear;
    }
  }
  if (NumFront > 0)
  {
    FrontAxleOffset /= NumFront;
    FrontAxleRadius /= NumFront;
  }
  if (NumRear > 0)
  {
    RearAxleOffset /= NumRear;
    RearAxleRadius /= NumRear;
  }
  if (bReadRestLocations && NumFront > 0 && NumRear > 0)
  {
    FrontAxleRestLocation /= NumFront;
    RearAxleRestLocation /= NumRear;
    bHasAxleRestLocations = true;
  }
}

static float VisualRadiusDelta(float VisualRadius, float PhysicsRadius)
{
  // 0 = the tyre is as big as the physics wheel.
  return VisualRadius > 0.0f && PhysicsRadius > 0.0f ? PhysicsRadius - VisualRadius : 0.0f;
}

void UCarlaVehicleAnimationInstance::UpdateBodyOffset(const ACarlaWheeledVehicle &Vehicle)
{
  BodyOffsetTranslation = FVector::ZeroVector;
  BodyOffsetRotation = FRotator::ZeroRotator;
  const float Wheelbase = FrontAxleRestLocation.X - RearAxleRestLocation.X;
  if (!bHasAxleRestLocations || FMath::Abs(Wheelbase) < UE_KINDA_SMALL_NUMBER)
  {
    return;
  }
  // Line through the two axle offsets, dz = A + B * (x - body x). A pitch of
  // atan(B) about the body bone raises a point at x by about B * (x - body x).
  const float FrontZ =
      FrontAxleOffset.Z - VisualRadiusDelta(Vehicle.FrontWheelVisualRadius, FrontAxleRadius);
  const float RearZ =
      RearAxleOffset.Z - VisualRadiusDelta(Vehicle.RearWheelVisualRadius, RearAxleRadius);
  const float B = (FrontZ - RearZ) / Wheelbase;
  const float A = FrontZ - B * (FrontAxleRestLocation.X - BodyRestLocation.X);
  BodyOffsetRotation.Pitch = FMath::RadiansToDegrees(FMath::Atan(B));
  BodyOffsetTranslation.Z = A;
}

bool UCarlaVehicleAnimationInstance::ShouldRollWheels(const ACarlaWheeledVehicle &Vehicle) const
{
  return GetWheeledVehicleComponent() != nullptr &&
      !Vehicle.IsSimulatedByChaos() &&
      !Vehicle.IsWheelAnimationOverridden();
}

void UCarlaVehicleAnimationInstance::InitWheels()
{
  const UChaosWheeledVehicleMovementComponent *Movement = GetWheeledVehicleComponent();
  const FTransform MeshTransform = GetSkelMeshComponent()->GetComponentTransform();
  const USkeletalMesh *SkeletalMesh = GetSkelMeshComponent()->GetSkeletalMeshAsset();

  Wheels.Init(FWheel(), Movement->WheelSetups.Num());
  for (int32 i = 0; i < Wheels.Num(); ++i)
  {
    FWheel &Wheel = Wheels[i];
    if (SkeletalMesh != nullptr)
    {
      const FReferenceSkeleton &RefSkeleton = SkeletalMesh->GetRefSkeleton();
      const int32 BoneIndex = RefSkeleton.FindBoneIndex(Movement->WheelSetups[i].BoneName);
      if (BoneIndex != INDEX_NONE)
      {
        Wheel.RestLocation =
            FAnimationRuntime::GetComponentSpaceTransformRefPose(RefSkeleton, BoneIndex).GetLocation();
      }
    }
    Wheel.LastLocation = MeshTransform.TransformPosition(Wheel.RestLocation);
  }
}

void UCarlaVehicleAnimationInstance::TrackWheelLocations()
{
  const UChaosWheeledVehicleMovementComponent *Movement = GetWheeledVehicleComponent();
  if (Movement == nullptr)
  {
    return;
  }
  if (Wheels.Num() != Movement->WheelSetups.Num())
  {
    InitWheels();
    return;
  }
  const FTransform MeshTransform = GetSkelMeshComponent()->GetComponentTransform();
  for (FWheel &Wheel : Wheels)
  {
    Wheel.LastLocation = MeshTransform.TransformPosition(Wheel.RestLocation);
  }
}

void UCarlaVehicleAnimationInstance::BeginRolling()
{
  if (Wheels.Num() != GetWheeledVehicleComponent()->WheelSetups.Num())
  {
    InitWheels();
  }
  const TArray<FWheelAnimationData> &WheelPoses = GetWheelPoses();
  for (int32 i = 0; i < Wheels.Num(); ++i)
  {
    // Carry on from the angle Chaos left.
    Wheels[i].SpinAngle = WheelPoses.IsValidIndex(i) ? WheelPoses[i].RotOffset.Pitch : 0.0f;
  }
}

void UCarlaVehicleAnimationInstance::RollWheels(const ACarlaWheeledVehicle &Vehicle, float DeltaSeconds)
{
  TArray<FWheelAnimationData> &WheelPoses = GetWheelPoses();
  const FTransform MeshTransform = GetSkelMeshComponent()->GetComponentTransform();
  const FVector Forward = MeshTransform.GetUnitAxis(EAxis::X);
  const float MaxTravel = MaxTravelSpeed * DeltaSeconds;

  for (int32 i = 0; i < Wheels.Num() && i < WheelPoses.Num(); ++i)
  {
    FWheel &Wheel = Wheels[i];
    const FVector Location = MeshTransform.TransformPosition(Wheel.RestLocation);
    const float Travel = FVector::DotProduct(Location - Wheel.LastLocation, Forward);
    Wheel.SpinAngle = RollWheel(Wheel.SpinAngle, Travel, Vehicle.GetWheelRadius(i), MaxTravel);
    Wheel.LastLocation = Location;
    WheelPoses[i].RotOffset.Pitch = Wheel.SpinAngle;
  }
}

void UCarlaVehicleAnimationInstance::UpdateWheelAnimationOverride(const ACarlaWheeledVehicle &Vehicle)
{
  const bool bOverridden = Vehicle.IsWheelAnimationOverridden();
  TArray<FWheelAnimationData> &WheelPoses = GetWheelPoses();
  if (bOverridden)
  {
    const TArray<FWheelAnimationData> &OverriddenPoses = Vehicle.GetOverriddenWheelPoses();
    for (int32 i = 0; i < WheelPoses.Num() && i < OverriddenPoses.Num(); ++i)
    {
      WheelPoses[i].RotOffset = OverriddenPoses[i].RotOffset;
      WheelPoses[i].LocOffset = OverriddenPoses[i].LocOffset;
    }
  }
  else if (bWasWheelAnimationOverridden && !Vehicle.IsSimulatedByChaos())
  {
    // Nothing else steers or compresses the wheels without Chaos. The roll
    // carries on from the pitch left.
    for (FWheelAnimationData &Pose : WheelPoses)
    {
      Pose.RotOffset.Yaw = 0.0f;
      Pose.LocOffset = FVector::ZeroVector;
    }
  }
  bWasWheelAnimationOverridden = bOverridden;
}

const FWheelAnimationData *UCarlaVehicleAnimationInstance::GetWheelPose(int32 WheelIndex) const
{
  const TArray<FWheelAnimationData> &WheelPoses =
      GetProxyOnGameThread<FVehicleAnimationInstanceProxy>().GetWheelAnimData();
  return WheelPoses.IsValidIndex(WheelIndex) ? &WheelPoses[WheelIndex] : nullptr;
}

TArray<FWheelAnimationData> &UCarlaVehicleAnimationInstance::GetWheelPoses()
{
  // The engine has no setter for the wheel poses.
  return const_cast<TArray<FWheelAnimationData> &>(
      GetProxyOnGameThread<FVehicleAnimationInstanceProxy>().GetWheelAnimData());
}
