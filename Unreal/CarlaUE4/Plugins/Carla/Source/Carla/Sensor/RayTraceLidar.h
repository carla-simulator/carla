// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "Carla/Sensor/Sensor.h"

#include "Carla/Sensor/LidarDescription.h"
#include "Carla/Sensor/LidarMeasurement.h"

#include "RayTraceLidar.generated.h"

/// A ray-trace based Lidar sensor.
UCLASS()
class CARLA_API ARayTraceLidar : public ASensor
{
  GENERATED_BODY()

public:

  ARayTraceLidar(const FObjectInitializer &ObjectInitializer);

  void Set(const FLidarDescription &LidarDescription);

protected:

  virtual void Tick(float DeltaTime) override;

private:

  /// Creates a Laser for each channel.
  void CreateLasers();

  /// Updates LidarMeasurement with the points read in DeltaTime.
  void ReadPoints(float DeltaTime);

  /// Shoot a laser ray-trace, return whether the laser hit something.
  bool ShootLaser(uint32 Channel, float HorizontalAngle, FVector &Point) const;

  UPROPERTY(EditAnywhere)
  FLidarDescription Description;

  TArray<float> LaserAngles;

  FLidarMeasurement LidarMeasurement;
};
