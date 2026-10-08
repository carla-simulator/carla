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

/// Parent class of the vehicle animation blueprints. Rolls the wheels by the
/// distance they travel while Chaos is not simulating the vehicle, which is
/// when the engine leaves them frozen.
UCLASS(transient)
class CARLA_API UCarlaVehicleAnimationInstance : public UVehicleAnimationInstance
{
  GENERATED_BODY()

public:

  virtual void NativeUpdateAnimation(float DeltaSeconds) override;

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

  bool ShouldRollWheels() const;

  void BeginRolling();

  void RollWheels(float DeltaSeconds);

  TArray<FWheelAnimationData> &GetWheelPoses();

  TArray<FWheel> Wheels;

  bool bRollingWheels = false;
};
