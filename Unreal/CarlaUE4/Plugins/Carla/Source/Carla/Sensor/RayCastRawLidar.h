// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "Carla/Sensor/Sensor.h"

#include "Carla/Actor/ActorDefinition.h"
#include "Carla/Sensor/LidarDescription.h"

#include <compiler/disable-ue4-macros.h>
#include <carla/sensor/s11n/LidarMeasurement.h>
#include <compiler/enable-ue4-macros.h>

#include "RayCastRawLidar.generated.h"

/// A ray-cast based Lidar sensor.
UCLASS()
class CARLA_API ARayCastRawLidar : public ASensor
{
  GENERATED_BODY()

  using FLidarMeasurement = carla::sensor::s11n::LidarMeasurement;

public:

  static FActorDefinition GetSensorDefinition();

  ARayCastRawLidar(const FObjectInitializer &ObjectInitializer);

  void Set(const FActorDescription &Description) override;

  void Set(const FLidarDescription &LidarDescription);

protected:

  virtual void Tick(float DeltaTime) override;

private:

  /// Creates a Laser for each channel.
  void CreateLasers();

  /// Updates LidarMeasurement with the points read in DeltaTime.
  void ReadPoints(float DeltaTime);

  /// Shoot a laser ray-trace, return whether the laser hit something.
  bool ShootLaser(uint32 Channel, float HorizontalAngle, FVector &Point, float& Intensity) const;

  /// Compute the received intensity of the point
  float ComputeIntensity(const FVector &LidarBodyLoc, const FHitResult& HitInfo) const;

  UPROPERTY(EditAnywhere)
  FLidarDescription Description;

  TArray<float> LaserAngles;

  FLidarMeasurement LidarMeasurement;

  // Enable/Disable general dropoff of lidar points
  bool DropOffGenActive;

  // Slope for the intensity dropoff of lidar points, it is calculated
  // throught the dropoff limit and the dropoff at zero intensity
  // The points is kept with a probality alpha*Intensity + beta where
  // alpha = (1 - dropoff_zero_intensity) / droppoff_limit
  // beta = (1 - dropoff_zero_intensity)
  float DropOffAlpha;
  float DropOffBeta;
};
