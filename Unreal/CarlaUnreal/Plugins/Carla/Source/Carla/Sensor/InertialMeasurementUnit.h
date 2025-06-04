// Copyright (c) 2024 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "Carla/Sensor/Sensor.h"

#include "Carla/Actor/ActorDefinition.h"
#include "Carla/Actor/ActorDescription.h"

#include <util/disable-ue4-macros.h>
#include "carla/geom/Vector3D.h"
#include <util/enable-ue4-macros.h>

#include <array>

#include "InertialMeasurementUnit.generated.h"

UCLASS()
class CARLA_API AInertialMeasurementUnit : public ASensor
{
  GENERATED_BODY()

public:

  AInertialMeasurementUnit(const FObjectInitializer &ObjectInitializer);

  static FActorDefinition GetSensorDefinition();

  void Set(const FActorDescription &ActorDescription) override;

  void SetOwner(AActor *Owner) override;

  virtual void PostPhysTick(UWorld *World, ELevelTick TickType, float DeltaTime) override;

  const carla::geom::Vector3D ComputeAccelerometerNoise(
      const FVector &Accelerometer);

  const carla::geom::Vector3D ComputeGyroscopeNoise(
      const FVector &Gyroscope);

  /// Accelerometer: measures linear acceleration in m/s^2
  carla::geom::Vector3D ComputeAccelerometer(const float DeltaTime);

  /// Gyroscope: measures angular velocity in rad/sec
  carla::geom::Vector3D ComputeGyroscope();

  /// Magnetometer: orientation with respect to the North in rad
  float ComputeCompass();

  void SetAccelerationStandardDeviation(const FVector &Vec);

  void SetGyroscopeStandardDeviation(const FVector &Vec);

  void SetGyroscopeBias(const FVector &Vec);

  const FVector &GetAccelerationStandardDeviation() const;

  const FVector &GetGyroscopeStandardDeviation() const;

  const FVector &GetGyroscopeBias() const;

  const carla::geom::Vector3D& GetAccelerometerValue() const;

  const carla::geom::Vector3D& GetGyroscopeValue() const;

  float GetCompassValue() const;

  /// Based on OpenDRIVE's lon and lat, North is in (0.0f, -1.0f, 0.0f)
  static const FVector CarlaNorthVector;

private:

  void BeginPlay() override;

  /// Standard deviation for acceleration settings.
  FVector StdDevAccel;

  /// Standard deviation for gyroscope settings.
  FVector StdDevGyro;

  /// Bias for gyroscope settings.
  FVector BiasGyro;

  /// Used to compute the acceleration
  std::array<FVector, 2> PrevLocation;

  /// Used to compute the acceleration
  float PrevDeltaTime;

  /// Accelerometer value calculated after each PostPhysTick
  carla::geom::Vector3D AccelerometerValue;

  /// Gyroscope value calculated after each PostPhysTick
  carla::geom::Vector3D GyroscopeValue;

  /// Magnetometer value calculated after each PostPhysTick
  float CompassValue;
	
};
