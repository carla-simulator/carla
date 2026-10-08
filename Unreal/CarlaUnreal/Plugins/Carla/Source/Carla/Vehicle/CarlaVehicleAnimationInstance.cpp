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

  UpdateWheelAnimationOverride();
  if (!ShouldRollWheels())
  {
    bRollingWheels = false;
    return;
  }
  if (!bRollingWheels)
  {
    BeginRolling();
    bRollingWheels = true;
  }
  RollWheels(DeltaSeconds);
}

bool UCarlaVehicleAnimationInstance::ShouldRollWheels() const
{
  const ACarlaWheeledVehicle *Vehicle = Cast<ACarlaWheeledVehicle>(GetOwningActor());
  return Vehicle != nullptr &&
      GetWheeledVehicleComponent() != nullptr &&
      !Vehicle->IsSimulatedByChaos() &&
      !Vehicle->IsWheelAnimationOverridden();
}

void UCarlaVehicleAnimationInstance::BeginRolling()
{
  const UChaosWheeledVehicleMovementComponent *Movement = GetWheeledVehicleComponent();
  const TArray<FWheelAnimationData> &WheelPoses = GetWheelPoses();
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
    // Carry on from the angle Chaos left.
    Wheel.SpinAngle = WheelPoses.IsValidIndex(i) ? WheelPoses[i].RotOffset.Pitch : 0.0f;
  }
}

void UCarlaVehicleAnimationInstance::RollWheels(float DeltaSeconds)
{
  const ACarlaWheeledVehicle *Vehicle = CastChecked<ACarlaWheeledVehicle>(GetOwningActor());
  TArray<FWheelAnimationData> &WheelPoses = GetWheelPoses();
  const FTransform MeshTransform = GetSkelMeshComponent()->GetComponentTransform();
  const FVector Forward = MeshTransform.GetUnitAxis(EAxis::X);
  const float MaxTravel = MaxTravelSpeed * DeltaSeconds;

  for (int32 i = 0; i < Wheels.Num() && i < WheelPoses.Num(); ++i)
  {
    FWheel &Wheel = Wheels[i];
    const FVector Location = MeshTransform.TransformPosition(Wheel.RestLocation);
    const float Travel = FVector::DotProduct(Location - Wheel.LastLocation, Forward);
    Wheel.SpinAngle = RollWheel(Wheel.SpinAngle, Travel, Vehicle->GetWheelRadius(i), MaxTravel);
    Wheel.LastLocation = Location;
    WheelPoses[i].RotOffset.Pitch = Wheel.SpinAngle;
  }
}

void UCarlaVehicleAnimationInstance::UpdateWheelAnimationOverride()
{
  const ACarlaWheeledVehicle *Vehicle = Cast<ACarlaWheeledVehicle>(GetOwningActor());
  const bool bOverridden = Vehicle != nullptr && Vehicle->IsWheelAnimationOverridden();
  TArray<FWheelAnimationData> &WheelPoses = GetWheelPoses();
  if (bOverridden)
  {
    const TArray<FWheelAnimationData> &OverriddenPoses = Vehicle->GetOverriddenWheelPoses();
    for (int32 i = 0; i < WheelPoses.Num() && i < OverriddenPoses.Num(); ++i)
    {
      WheelPoses[i].RotOffset = OverriddenPoses[i].RotOffset;
      WheelPoses[i].LocOffset = OverriddenPoses[i].LocOffset;
    }
  }
  else if (bWheelAnimationOverridden && Vehicle != nullptr && !Vehicle->IsSimulatedByChaos())
  {
    // Nothing else steers or compresses the wheels without Chaos. The roll
    // carries on from the pitch left.
    for (FWheelAnimationData &Pose : WheelPoses)
    {
      Pose.RotOffset.Yaw = 0.0f;
      Pose.LocOffset = FVector::ZeroVector;
    }
  }
  bWheelAnimationOverridden = bOverridden;
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
