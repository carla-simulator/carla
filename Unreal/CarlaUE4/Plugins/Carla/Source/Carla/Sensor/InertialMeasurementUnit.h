// Copyright (c) 2019 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "Carla/Sensor/Sensor.h"

#include "Carla/Actor/ActorDefinition.h"
#include "Carla/Actor/ActorDescription.h"

#include <compiler/disable-ue4-macros.h>
#include "carla/geom/Vector3D.h"
#include <compiler/enable-ue4-macros.h>

#include "InertialMeasurementUnit.generated.h"

UCLASS()
class CARLA_API AInertialMeasurementUnit : public ASensor
{
  GENERATED_BODY()

public:

  void SetAccelerationStandardDeviation(const FVector &Vec);

  void SetGyroscopeStandardDeviation(const FVector &Vec);

  void SetGyroscopeBias(const FVector &Vec);

  AInertialMeasurementUnit(const FObjectInitializer &ObjectInitializer);

  static FActorDefinition GetSensorDefinition();

  void Set(const FActorDescription &ActorDescription) override;

  void SetOwner(AActor *Owner) override;

  void Tick(float DeltaTime) override;

  const FVector &GetAccelerationStandardDeviation() const;

  const FVector &GetGyroscopeStandardDeviation() const;

  const FVector &GetGyroscopeBias() const;

  const carla::geom::Vector3D ComputeAccelerometerNoise(const FVector &Accelerometer);

  const carla::geom::Vector3D ComputeGyroscopeNoise(const FVector &Gyroscope);

  /// Based on OpenDRIVE's lon and lat, North is in (0.0f, -1.0f, 0.0f)
  static const FVector CarlaNorthVector;

private:

  void BeginPlay() override;

  FVector PrevVelocity;

  // Noise Settings
  FVector StdDevAccel;

  FVector StdDevGyro;

  FVector BiasGyro;

};
