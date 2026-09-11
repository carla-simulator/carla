// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "Carla/Sensor/Sensor.h"

#include "Carla/Actor/ActorDefinition.h"
#include "Carla/Actor/ActorDescription.h"

#include <util/disable-ue4-macros.h>
#include <carla/sensor/s11n/VehicleStatusSerializer.h>
#include <util/enable-ue4-macros.h>

#include "VehicleStatusSensor.generated.h"

class ACarlaWheeledVehicle;

// Structure imported from LibCarla
using FVehicleStatusData = carla::sensor::s11n::VehicleStatusData;

USTRUCT(BlueprintType)
struct FVelocityInfo
{
  GENERATED_BODY()

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Velocity")
  FVector Velocity;

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Velocity")
  FVector AngularVelocity;

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Velocity")
  FRotator RotationRate;

  FVelocityInfo()
    : Velocity(FVector::ZeroVector)
    , AngularVelocity(FVector::ZeroVector)
    , RotationRate(FRotator::ZeroRotator)
  {
  }

  float GetSpeed() const
  {
    return Velocity.Size();
  }

  FString ToString() const
  {
    return FString::Printf(TEXT("Velocity: %s | Angular Velocity: %s | Rotation Rate: %s"),
      *Velocity.ToString(),
      *AngularVelocity.ToString(),
      *RotationRate.ToString());
  }
};

/// Vehicle status sensor ("sensor.other.vehicle_status"): attached to a
/// vehicle, streams a packed kinematic + control state snapshot each tick and
/// forwards it to the ROS2 side, where it feeds the six Autoware
/// /vehicle/status/* report publishers. Ported from tier4/autoware-support.
UCLASS()
class CARLA_API AVehicleStatusSensor : public ASensor
{
  GENERATED_BODY()

public:

  explicit AVehicleStatusSensor(const FObjectInitializer &ObjectInitializer);

  // Carla blueprint definition (get sensor.other.vehicle_status in BP)
  static FActorDefinition GetSensorDefinition();

  // Called by Carla's spawning pipeline with user attributes
  virtual void Set(const FActorDescription &ActorDescription) override;

  virtual void PostPhysTick(UWorld *World, ELevelTick TickType, float DeltaSeconds) override;

protected:

  virtual void BeginPlay() override;

  // Publish frequency
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Sensor|Rate", meta=(ClampMin="1.0", UIMin="1.0"))
  float MinRateHz = 1.0f;
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Sensor|Rate", meta=(ClampMin="1.0", UIMin="1.0"))
  float MaxRateHz = 1000.0f;

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Sensor|Rate", meta=(ClampMin="1.0", ClampMax="1000.0", UIMin="1.0", UIMax="1000.0"))
  float TargetRateHz = 30.0f;

private:

  // Cached parent vehicle
  TWeakObjectPtr<ACarlaWheeledVehicle> OwningVehicle;
  FVelocityInfo VelocityInfo;

  // Find parent helpers
  FTimerHandle CheckParentTimerHandle;
  void CheckForParentVehicle();
  TObjectPtr<ACarlaWheeledVehicle> FindAttachmentParentVehicle() const;

  // Helpers
  void CollectAndStream(float DeltaSeconds);
  void SetVelocityInfoToLocal(const AActor* VehicleActor);

  // Scale UE velocity from cm/s to m/s
  template<typename T>
  static FORCEINLINE T CmpsToMps(const T& v_cmps)
  {
    return v_cmps * 0.01f;
  }
};
