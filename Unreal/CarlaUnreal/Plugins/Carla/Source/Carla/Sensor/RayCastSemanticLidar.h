// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.



#pragma once

#include "Carla/Sensor/Sensor.h"
#include "Carla/Actor/ActorDefinition.h"
#include "Carla/Sensor/LidarDescription.h"
#include "Carla/Actor/ActorBlueprintFunctionLibrary.h"

#include <util/disable-ue4-macros.h>
#include <carla/sensor/data/SemanticLidarData.h>
#include <util/enable-ue4-macros.h>

#include <numeric>
#include <type_traits>
#include <vector>

#include "RayCastSemanticLidar.generated.h"

/// A ray-cast based Lidar sensor.
UCLASS()
class CARLA_API ARayCastSemanticLidar : public ASensor
{
  GENERATED_BODY()

protected:

  using FSemanticLidarData = carla::sensor::data::SemanticLidarData;
  using FSemanticDetection = carla::sensor::data::SemanticLidarDetection;

public:
  static FActorDefinition GetSensorDefinition();

  ARayCastSemanticLidar(const FObjectInitializer &ObjectInitializer);

  virtual void Set(const FActorDescription &Description) override;
  virtual void Set(const FLidarDescription &LidarDescription);

  virtual void SimulateLidar(const float DeltaTime, bool bLockPhysics = true);

  virtual void SendData(const float DeltaTime);

protected:
  virtual void BeginPlay() override;
  virtual void EndPlay(EEndPlayReason::Type EndPlayReason) override;

  /// Creates a Laser for each channel.
  void CreateLasers();

  /// Shoot a laser ray-trace, return whether the laser hit something.
  bool ShootLaser(const float VerticalAngle, float HorizontalAngle, FHitResult &HitResult, FCollisionQueryParams& TraceParams,
                  const FVector &LidarBodyLocation, const FRotator &LidarBodyRotation) const;

  /// Method that allow to preprocess if the rays will be traced.
  virtual void PreprocessRays(uint32_t Channels, uint32_t MaxPointsPerChannel);

  /// Compute all raw detection information
  void ComputeRawDetection(const FHitResult &HitInfo, const FTransform &InverseSensorTransform, const FVector &SensorLocation, FSemanticDetection &Detection) const;

  /// Computes the detection of a hit inside the raycast ParallelFor.
  virtual void WriteDetectionAsync(uint32_t Channel, const FHitResult &HitInfo, const FTransform &InverseSensorTransform, const FVector &SensorLocation);

  /// Clear the per-channel detection buffers
  virtual void ResetDetections(uint32_t Channels, uint32_t MaxPointsPerChannel);

  /// Copies the per-channel detections into the LidarData structure.
  virtual void ComputeAndSaveDetections(const FTransform &SensorTransform);

  /// Clears every channel buffer and reserves room for a full scan.
  template <typename DetectionT>
  static void ResetChannelDetections(std::vector<std::vector<DetectionT>> &Detections, uint32_t Channels, uint32_t MaxPointsPerChannel)
  {
    Detections.resize(Channels);
    for (auto &ChannelDetections : Detections)
    {
      ChannelDetections.clear();
      ChannelDetections.reserve(MaxPointsPerChannel);
    }
  }

  /// Post-processes the detections channel by channel, drops the rejected ones and writes the rest to LidarData.
  /// Runs sequentially in channel and point order, so the random sequence for noise and drop-off is unchanged.
  template <typename DetectionT, typename LidarDataT, typename PostprocessT>
  static void CompactAndWriteDetections(
      std::vector<std::vector<DetectionT>> &Detections,
      std::vector<uint32_t> &OutPointsPerChannel,
      uint32_t Channels,
      LidarDataT &LidarData,
      PostprocessT &&Postprocess)
  {
    for (uint32_t Channel = 0u; Channel < Channels; ++Channel)
    {
      auto &ChannelDetections = Detections[Channel];
      size_t Kept = 0;
      for (DetectionT &Detection : ChannelDetections)
      {
        if (Postprocess(Detection))
        {
          ChannelDetections[Kept++] = Detection;
        }
      }
      ChannelDetections.resize(Kept);
      OutPointsPerChannel[Channel] = static_cast<uint32_t>(Kept);
    }

    LidarData.ResetMemory(OutPointsPerChannel);
    for (uint32_t Channel = 0u; Channel < Channels; ++Channel)
    {
      LidarData.WritePoints(Detections[Channel]);
    }
    LidarData.WriteChannelCount(OutPointsPerChannel);
  }

  /// Copies the kept detections into a flat x, y, z, intensity array (editor debug point cloud).
  template <typename DetectionT>
  static void CopyDetectionsToPointCloud(
      const std::vector<std::vector<DetectionT>> &Detections,
      const std::vector<uint32_t> &InPointsPerChannel,
      TArray<float> &PointCloud)
  {
    static_assert(sizeof(DetectionT) == sizeof(float) * 4, "Detection must be 4 packed floats");
    static_assert(std::is_trivially_copyable_v<DetectionT>, "Detection must be trivially copyable");

    const uint32_t TotalPoints = std::accumulate(InPointsPerChannel.begin(), InPointsPerChannel.end(), 0u);
    PointCloud.SetNumUninitialized(static_cast<int32>(TotalPoints * 4));

    float *Dest = PointCloud.GetData();
    for (const auto &ChannelDetections : Detections)
    {
      if (ChannelDetections.empty())
      {
        continue;
      }
      FMemory::Memcpy(Dest, ChannelDetections.data(), ChannelDetections.size() * sizeof(DetectionT));
      Dest += ChannelDetections.size() * 4;
    }
  }

  UPROPERTY(EditAnywhere)
  FLidarDescription Description;

  TArray<float> LaserAngles;

  std::vector<std::vector<bool>> RayPreprocessCondition;
  std::vector<uint32_t> PointsPerChannel;

private:
  FSemanticLidarData SemanticLidarData;

  std::vector<std::vector<FSemanticDetection>> SemanticDetections;

};
