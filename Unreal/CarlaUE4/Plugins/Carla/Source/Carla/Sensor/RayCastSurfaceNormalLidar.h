// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
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

#include <compiler/disable-ue4-macros.h>
#include <carla/sensor/data/SurfaceNormalLidarData.h>
#include <compiler/enable-ue4-macros.h>

#include "RayCastSurfaceNormalLidar.generated.h"

/// A ray-cast based surface normal Lidar sensor.
UCLASS()
class CARLA_API ARayCastSurfaceNormalLidar : public ARayCastSemanticLidar
{
  GENERATED_BODY()

  using FSurfaceNormalLidarData = carla::sensor::data::SurfaceNormalLidarData;
  using FSurfaceNormalLidarDetection = carla::sensor::data::SurfaceNormalLidarDetection;

public:
  static FActorDefinition GetSensorDefinition();

  ARayCastSurfaceNormalLidar(const FObjectInitializer &ObjectInitializer);
  virtual void Set(const FActorDescription &Description) override;
  virtual void Set(const FLidarDescription &LidarDescription) override;

  virtual void PostPhysTick(UWorld *World, ELevelTick TickType, float DeltaTime);

private:
  /// Compute the received intensity of the point
  float ComputeIntensity(const FSurfaceNormalLidarDetection& RawDetection) const;
  FSurfaceNormalLidarDetection ComputeDetection(const FHitResult& HitInfo, const FTransform& SensorTransf) const;

  void PreprocessRays(uint32_t Channels, uint32_t MaxPointsPerChannel) override;
  bool PostprocessDetection(FSurfaceNormalLidarDetection& Detection) const;

  void ComputeAndSaveDetections(const FTransform& SensorTransform) override;

  FSurfaceNormalLidarData LidarData;

  /// Enable/Disable general dropoff of lidar points
  bool DropOffGenActive;

  /// Slope for the intensity dropoff of lidar points, it is calculated
  /// throught the dropoff limit and the dropoff at zero intensity
  /// The points is kept with a probality alpha*Intensity + beta where
  /// alpha = (1 - dropoff_zero_intensity) / droppoff_limit
  /// beta = (1 - dropoff_zero_intensity)
  float DropOffAlpha;
  float DropOffBeta;
};
