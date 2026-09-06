// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once


#include "Carla/Actor/ActorDefinition.h"
#include "Carla/Sensor/LidarDescription.h"
#include "Carla/Sensor/Sensor.h"
#include "Carla/Sensor/RayCastSemanticLidar.h"
#include "Carla/Actor/ActorBlueprintFunctionLibrary.h"

#include <util/disable-ue4-macros.h>
#include <carla/sensor/data/LidarData.h>
#include <util/enable-ue4-macros.h>

#include "HSSLidar.generated.h"

/// A fixed-sweep ray-cast Lidar approximating a Hybrid Solid-State LiDAR
/// (e.g. Hesai AT128). Unlike ARayCastLidar this sensor does not rotate; each
/// tick it casts a configurable horizontal field of view at a fixed angular
/// resolution centred on the sensor's forward axis.
UCLASS()
class CARLA_API AHSSLidar : public ARayCastSemanticLidar
{
  GENERATED_BODY()

  using FLidarData = carla::sensor::data::LidarData;
  using FDetection = carla::sensor::data::LidarDetection;

public:
  static FActorDefinition GetSensorDefinition();

  AHSSLidar(const FObjectInitializer &ObjectInitializer);
  virtual void Set(const FActorDescription &Description) override;
  virtual void Set(const FLidarDescription &LidarDescription) override;

  virtual void PostPhysTick(UWorld *World, ELevelTick TickType, float DeltaTime) override;

  const TArray<float>& GetTestPointCloud() const { return PointCloudLidarData; };

protected:
  virtual void SimulateLidar(const float DeltaTime);

private:
  FDetection ComputeDetection(const FHitResult& HitInfo, const FTransform& SensorTransf) const;

  void PreprocessRays(uint32_t Channels, uint32_t MaxPointsPerChannel) override;
  bool PostprocessDetection(FDetection& Detection) const;

  void ComputeAndSaveDetections(const FTransform& SensorTransform) override;

  FLidarData LidarData;

  /// Enable/Disable general dropoff of lidar points.
  bool DropOffGenActive;

  /// Slope for the intensity dropoff of lidar points: alpha*Intensity + beta
  /// where alpha = dropoff_zero_intensity / dropoff_limit and
  /// beta = 1 - dropoff_zero_intensity.
  float DropOffAlpha;
  float DropOffBeta;

  /// WITH_EDITOR-only mirror of the point cloud, used by the debug save path.
  TArray<float> PointCloudLidarData;

  void PointCloudResetMemory();
  void PointCloudWritePointSync(const FDetection& Detection);
};
