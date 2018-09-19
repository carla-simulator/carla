// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Carla/Sensor/DeprecatedSensor.h"

#include "Carla/Sensor/LidarMeasurement.h"
#include "Carla/Settings/LidarDescription.h"

#include "Lidar.generated.h"

/// A ray-trace based Lidar sensor.
UCLASS()
class CARLA_API ALidar : public ADeprecatedSensor
{
  GENERATED_BODY()

public:

  ALidar(const FObjectInitializer &ObjectInitializer);

  void Set(const ULidarDescription &LidarDescription);

protected:

  virtual void Tick(float DeltaTime) override;

private:

  /// Creates a Laser for each channel.
  void CreateLasers();

  /// Updates LidarMeasurement with the points read in DeltaTime.
  void ReadPoints(float DeltaTime);

  /// Shoot a laser ray-trace, return whether the laser hit something.
  bool ShootLaser(uint32 Channel, float HorizontalAngle, FVector &Point) const;

  UPROPERTY(Category = "Lidar", VisibleAnywhere)
  const ULidarDescription *Description = nullptr;

  TArray<float> LaserAngles;

  FLidarMeasurement LidarMeasurement;
};
