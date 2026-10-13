// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include <util/ue-header-guard-begin.h>
#include "VehicleAnimationInstance.h"
#include <util/ue-header-guard-end.h>

#include "CarlaVehicleAnimationInstance.generated.h"

class ACarlaWheeledVehicle;

/// Parent class of the vehicle animation blueprints. Draws the vehicle's
/// overridden wheel poses, and otherwise, while Chaos is not simulating the
/// vehicle, which is when the engine leaves the wheels frozen, rolls them by
/// the distance they travel. Also exposes the suspension offset per axle, and
/// the rigid motion that puts a mesh whose wheels are not animated by the
/// suspension (the two-wheelers: their visual wheels are separate bones, and
/// fork and swingarm are rigid) on the wheels Chaos simulates.
UCLASS(transient)
class CARLA_API UCarlaVehicleAnimationInstance : public UVehicleAnimationInstance
{
  GENERATED_BODY()

public:

  virtual void NativeUpdateAnimation(float DeltaSeconds) override;

  /// The pose last written for a wheel, or nullptr if there is no such wheel.
  const FWheelAnimationData *GetWheelPose(int32 WheelIndex) const;

  /// Suspension offset of the front axle: the average of the front wheels'
  /// location offsets, in component space (cm). The same offset the engine's
  /// wheel controller node adds to a physics wheel bone; add it to a visual
  /// wheel bone with a Transform (Modify) Bone node in "Add to Existing" mode,
  /// component space.
  UPROPERTY(BlueprintReadOnly, Category = "CARLA Wheeled Vehicle")
  FVector FrontAxleOffset = FVector::ZeroVector;

  /// Same as FrontAxleOffset for the rear axle.
  UPROPERTY(BlueprintReadOnly, Category = "CARLA Wheeled Vehicle")
  FVector RearAxleOffset = FVector::ZeroVector;

  /// Rigid motion of BodyOffsetBone that moves each axle of the reference
  /// pose by its suspension offset, minus the difference between the physics
  /// wheel radius and the vehicle's visual tyre radius (FrontWheelVisualRadius,
  /// RearWheelVisualRadius): a pitch about BodyOffsetBone and a vertical
  /// translation, in component space. Apply both to BodyOffsetBone with a
  /// Transform (Modify) Bone node in "Add to Existing" mode, component space.
  UPROPERTY(BlueprintReadOnly, Category = "CARLA Wheeled Vehicle")
  FVector BodyOffsetTranslation = FVector::ZeroVector;

  UPROPERTY(BlueprintReadOnly, Category = "CARLA Wheeled Vehicle")
  FRotator BodyOffsetRotation = FRotator::ZeroRotator;

  /// Bone the body offset is applied to, and pitched about. Must not be a
  /// bone the physics simulation drives, or the offset is overwritten. None
  /// means the root bone.
  UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "CARLA Wheeled Vehicle")
  FName BodyOffsetBone = NAME_None;

private:

  /// cm/s. A wheel moving faster than this was teleported, and is not turned.
  UPROPERTY(EditDefaultsOnly, Category = "CARLA Wheeled Vehicle")
  float MaxTravelSpeed = 15000.0f;

  struct FWheel
  {
    /// Reference pose, component space.
    FVector RestLocation = FVector::ZeroVector;

    /// World space, previous update.
    FVector LastLocation = FVector::ZeroVector;

    /// Degrees.
    float SpinAngle = 0.0f;
  };

  bool ShouldRollWheels(const ACarlaWheeledVehicle &Vehicle) const;

  void InitWheels();

  /// World locations of the wheels, kept while not rolling.
  void TrackWheelLocations();

  void BeginRolling();

  void RollWheels(const ACarlaWheeledVehicle &Vehicle, float DeltaSeconds);

  void UpdateWheelAnimationOverride(const ACarlaWheeledVehicle &Vehicle);

  void UpdateAxleOffsets(const ACarlaWheeledVehicle &Vehicle);

  void UpdateBodyOffset(const ACarlaWheeledVehicle &Vehicle);

  /// Reference pose, component space: average location of the front and rear
  /// wheel bones, and BodyOffsetBone. Read on the first update.
  bool bHasAxleRestLocations = false;
  FVector FrontAxleRestLocation = FVector::ZeroVector;
  FVector RearAxleRestLocation = FVector::ZeroVector;
  FVector BodyRestLocation = FVector::ZeroVector;

  /// Average physics wheel radius of each axle, cm, this update.
  float FrontAxleRadius = 0.0f;
  float RearAxleRadius = 0.0f;

  TArray<FWheelAnimationData> &GetWheelPoses();

  TArray<FWheel> Wheels;

  bool bRollingWheels = false;

  bool bWasWheelAnimationOverridden = false;
};
