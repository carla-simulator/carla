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
#include <carla/sensor/data/LidarRawData.h>
#include <compiler/enable-ue4-macros.h>

#include "RayCastRawLidar.generated.h"

/// A ray-cast based Lidar sensor.
UCLASS()
class CARLA_API ARayCastRawLidar : public ASensor
{
  GENERATED_BODY()

  using FLidarData = carla::sensor::data::LidarRawData;
  using FLidarRawData = carla::sensor::data::LidarRawData;

  using FDetection = carla::sensor::data::LidarRawDetection;

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
  void SimulateLidar(float DeltaTime);

  /// Shoot a laser ray-trace, return whether the laser hit something.
  bool ShootLaser(const float VerticalAngle, float HorizontalAngle, FHitResult &RawData) const;

  /// Method that allow to preprocess the ray before shoot it
  bool PreprocessRay(const float& VerticalAngle, float &HorizontalAngle) const {
    // This method allows to introduce noise or drop points if needed
    // A true return value will make the proposed ray to be actually computed.
    return true;
  }

  /// Compute all raw detection information
  void ComputeRawDetection(const FHitResult &HitInfo, const FTransform &SensorTransf, FDetection &Detection) const;

  /// Saving the hits the raycast returns per channel
  void WritePointAsync(uint32_t Channel, FHitResult &Detection);

  /// Clear the recorded data structure
  void ResetRecordedHits(uint32_t Channels, uint32_t MaxPointsPerChannel);

  /// Clear the recorded data structure
  void ComputeAndSaveDetections(const FTransform &SensorTransform);



  UPROPERTY(EditAnywhere)
  FLidarDescription Description;

  TArray<float> LaserAngles;

  FLidarData LidarData;
  std::vector<std::vector<FHitResult>> RecordedHits;

};
