// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.
//
// Lidar implemented by Csonthó Mihály based on RayCastLidar code.

#include "Carla/Sensor/HSSLidar.h"
#include "Carla.h"
#include "Carla/Actor/ActorBlueprintFunctionLibrary.h"

#include <util/disable-ue4-macros.h>
#include "carla/geom/Math.h"
#include "carla/ros2/ROS2.h"
#include "carla/geom/Location.h"
#include <util/enable-ue4-macros.h>

#include <util/ue-header-guard-begin.h>
#include "DrawDebugHelpers.h"
#include "Engine/CollisionProfile.h"
#include "Kismet/KismetMathLibrary.h"
#include "PhysicsEngine/PhysicsObjectExternalInterface.h"
#include "Async/ParallelFor.h"
#include <util/ue-header-guard-end.h>

#include <cmath>
#include <numeric>

FActorDefinition AHSSLidar::GetSensorDefinition()
{
  return UActorBlueprintFunctionLibrary::MakeLidarDefinition(TEXT("hss_lidar"));
}

AHSSLidar::AHSSLidar(const FObjectInitializer& ObjectInitializer)
  : Super(ObjectInitializer)
{
  RandomEngine = CreateDefaultSubobject<URandomEngine>(TEXT("RandomEngine"));
  SetSeed(Description.RandomSeed);
}

void AHSSLidar::Set(const FActorDescription &ActorDescription)
{
  ASensor::Set(ActorDescription);
  FLidarDescription LidarDescription;
  UActorBlueprintFunctionLibrary::SetLidar(ActorDescription, LidarDescription);
  Set(LidarDescription);
}

void AHSSLidar::Set(const FLidarDescription &LidarDescription)
{
  Description = LidarDescription;
  LidarData = FLidarData(Description.Channels);
  CreateLasers();
  PointsPerChannel.resize(Description.Channels);

  // Apply the user-supplied noise_seed; the constructor's SetSeed ran before
  // Description was populated from the blueprint attributes, so the engine
  // would otherwise be stuck on the FLidarDescription default seed.
  SetSeed(Description.RandomSeed);

  DropOffBeta = 1.0f - Description.DropOffAtZeroIntensity;
  // Guard against dropoff_intensity_limit == 0: alpha = x/0 would propagate
  // inf/NaN through PostprocessDetection and silently disable the
  // intensity-based dropoff branch. With alpha = 0 the postprocess simply
  // falls back to the constant beta probability for low-intensity points.
  DropOffAlpha = (Description.DropOffIntensityLimit > std::numeric_limits<float>::epsilon())
      ? Description.DropOffAtZeroIntensity / Description.DropOffIntensityLimit
      : 0.0f;
  DropOffGenActive = Description.DropOffGenRate > std::numeric_limits<float>::epsilon();
}

void AHSSLidar::SendData(const float DeltaTime)
{
  TRACE_CPUPROFILER_EVENT_SCOPE(AHSSLidar::SendData);
  auto DataStream = GetDataStream(*this);
  auto SensorTransform = DataStream.GetSensorTransform();

  {
    TRACE_CPUPROFILER_EVENT_SCOPE_STR("Send Stream");
    DataStream.SerializeAndSend(*this, LidarData, DataStream.PopBufferFromPool());
  }
  // ROS2
  #if defined(WITH_ROS2)
  auto ROS2 = carla::ros2::ROS2::GetInstance();
  if (ROS2->IsEnabled())
  {
    TRACE_CPUPROFILER_EVENT_SCOPE_STR("ROS2 Send");
    auto StreamId = carla::streaming::detail::token_type(GetToken()).get_stream_id();
    AActor* ParentActor = GetAttachParentActor();
    if (ParentActor)
    {
      FTransform LocalTransformRelativeToParent = GetActorTransform().GetRelativeTransform(ParentActor->GetActorTransform());
      ROS2->ProcessDataFromLidar(DataStream.GetSensorType(), StreamId, LocalTransformRelativeToParent, LidarData, this);
    }
    else
    {
      ROS2->ProcessDataFromLidar(DataStream.GetSensorType(), StreamId, SensorTransform, LidarData, this);
    }
  }
  #endif
}

AHSSLidar::FDetection AHSSLidar::ComputeDetection(
    const FHitResult& HitInfo, const FTransform& InverseSensorTransform) const
{
  FDetection Detection;
  const FVector HitPoint = HitInfo.ImpactPoint;
  Detection.point = InverseSensorTransform.TransformPosition(HitPoint);

  const float Distance = Detection.point.Length();

  const float AttenAtm = Description.AtmospAttenRate;
  const float AbsAtm = exp(-AttenAtm * Distance);

  const float IntRec = AbsAtm;

  Detection.intensity = IntRec;

  return Detection;
}

void AHSSLidar::PreprocessRays(uint32_t Channels, uint32_t MaxPointsPerChannel)
{
  Super::PreprocessRays(Channels, MaxPointsPerChannel);

  if (!DropOffGenActive)
  {
    return;
  }

  for (auto ch = 0u; ch < Channels; ch++) {
    for (auto p = 0u; p < MaxPointsPerChannel; p++) {
      RayPreprocessCondition[ch][p] =
          !(DropOffGenActive && RandomEngine->GetUniformFloat() < Description.DropOffGenRate);
    }
  }
}

bool AHSSLidar::PostprocessDetection(FDetection& Detection) const
{
  if (Description.NoiseStdDev > std::numeric_limits<float>::epsilon()) {
    const auto ForwardVector = Detection.point.MakeUnitVector();
    const auto Noise = ForwardVector * RandomEngine->GetNormalDistribution(0.0f, Description.NoiseStdDev);
    Detection.point += Noise;
  }

  const float Intensity = Detection.intensity;
  if(Intensity > Description.DropOffIntensityLimit)
    return true;
  else
    return RandomEngine->GetUniformFloat() < DropOffAlpha * Intensity + DropOffBeta;
}

void AHSSLidar::ResetDetections(uint32_t Channels, uint32_t MaxPointsPerChannel)
{
  Detections.resize(Channels);

  for (auto& ChannelDetections : Detections)
  {
    ChannelDetections.clear();
    ChannelDetections.reserve(MaxPointsPerChannel);
  }
}

void AHSSLidar::WriteDetectionAsync(uint32_t Channel, const FHitResult& HitInfo, const FTransform& InverseSensorTransform, const FVector& SensorLocation)
{
  DEBUG_ASSERT(GetChannelCount() > Channel);
  Detections[Channel].emplace_back(ComputeDetection(HitInfo, InverseSensorTransform));
}

void AHSSLidar::ComputeAndSaveDetections(const FTransform& SensorTransform)
{
  TRACE_CPUPROFILER_EVENT_SCOPE_STR(__FUNCTION__);

  for (auto idxChannel = 0u; idxChannel < Description.Channels; ++idxChannel)
  {
    auto& ChannelDetections = Detections[idxChannel];
    size_t Kept = 0;
    for (FDetection& Detection : ChannelDetections)
    {
      if (PostprocessDetection(Detection))
      {
        ChannelDetections[Kept++] = Detection;
      }
    }
    ChannelDetections.resize(Kept);
    PointsPerChannel[idxChannel] = static_cast<uint32_t>(Kept);
  }

  LidarData.ResetMemory(PointsPerChannel);
  for (auto idxChannel = 0u; idxChannel < Description.Channels; ++idxChannel)
  {
    LidarData.WritePoints(Detections[idxChannel]);
  }
  LidarData.WriteChannelCount(PointsPerChannel);

#if WITH_EDITOR
  if (bSavingDataToDisk)
  {
    const uint32_t TotalPoints = std::accumulate(PointsPerChannel.begin(), PointsPerChannel.end(), 0u);

    static_assert(sizeof(FDetection) == sizeof(float) * 4);
    static_assert(std::is_trivially_copyable_v<FDetection>);

    PointCloudLidarData.SetNumUninitialized(static_cast<int32>(TotalPoints * 4));

    float* Dest = PointCloudLidarData.GetData();

    for (const auto& ChannelDetections : Detections)
    {
      if (ChannelDetections.empty())
      {
        continue;
      }
      const size_t NumBytes = ChannelDetections.size() * sizeof(FDetection);
      FMemory::Memcpy(Dest, ChannelDetections.data(), NumBytes);
      Dest += ChannelDetections.size() * 4;
    }
  }
#endif
}

// SimulateLidar(float DeltaTime)
//
// Fixed-sweep scan centred on the sensor's forward axis. Casts
// HorizontalFov / HorizontalResolution rays per vertical channel each tick;
// no horizontal-angle accumulation across ticks (the sensor does not rotate).
static float SnapToStep(float value, float step)
{
  return std::round(value / step) * step;
}

void AHSSLidar::SimulateLidar(const float DeltaTime, bool bLockPhysics)
{
  TRACE_CPUPROFILER_EVENT_SCOPE(AHSSLidar::SimulateLidar);
  const uint32 ChannelCount = Description.Channels;

  // horizontal_resolution and horizontal_fov are user-supplied attributes;
  // clamp them to safe positive values before dividing. A zero or negative
  // resolution would divide to inf/NaN and then cast to a huge uint32,
  // triggering an OOM-sized allocation in ResetDetections/PreprocessRays.
  constexpr float MinHorizontalResolution = 0.01f;
  const float HorizontalResolution = FMath::Max(
      SnapToStep(Description.HorizontalResolution, MinHorizontalResolution),
      MinHorizontalResolution);
  const float HorizontalFov = FMath::Max(Description.HorizontalFov, 0.0f);
  const uint32 PointsToScanWithOneLaser = static_cast<uint32>(
      FMath::RoundHalfFromZero(HorizontalFov / HorizontalResolution));

  if (PointsToScanWithOneLaser == 0)
  {
    UE_LOG(
        LogCarla,
        Warning,
        TEXT("%s: no points requested this frame, increase horizontal_fov or decrease horizontal_resolution."),
        *GetName());
    return;
  }

  check(ChannelCount == LaserAngles.Num());

  ResetDetections(ChannelCount, PointsToScanWithOneLaser);
  PreprocessRays(ChannelCount, PointsToScanWithOneLaser);

  const FTransform ActorTransform = GetTransform();
  const FTransform InverseSensorTransform = ActorTransform.Inverse();
  const FVector LidarBodyLocation = ActorTransform.GetLocation();
  const FRotator LidarBodyRotation = ActorTransform.Rotator();

  auto RunChannelRaycasts = [&]()
  {
    TRACE_CPUPROFILER_EVENT_SCOPE(ParallelFor);
    ParallelFor(ChannelCount, [&](int32 idxChannel) {
      TRACE_CPUPROFILER_EVENT_SCOPE(ParallelForTask);

      FCollisionQueryParams TraceParams = FCollisionQueryParams(FName(TEXT("Laser_Trace")), true, this);
      TraceParams.bTraceComplex = true;
      TraceParams.bReturnPhysicalMaterial = false;

      const float VertAngle = LaserAngles[idxChannel];
      const std::vector<bool>& ChannelPreprocessCondition = RayPreprocessCondition[idxChannel];

      for (auto idxPtsOneLaser = 0u; idxPtsOneLaser < PointsToScanWithOneLaser; idxPtsOneLaser++) {
        if (!ChannelPreprocessCondition[idxPtsOneLaser])
        {
          continue;
        }

        FHitResult HitResult;
        const float HorizAngle =
            -HorizontalFov / 2.0f + static_cast<float>(idxPtsOneLaser) * HorizontalResolution;

        if (ShootLaser(VertAngle, HorizAngle, HitResult, TraceParams, LidarBodyLocation, LidarBodyRotation)) {
          WriteDetectionAsync(idxChannel, HitResult, InverseSensorTransform, LidarBodyLocation);
        }
      };
    });
  };

  if (bLockPhysics)
  {
    auto LockedPhysObject = FPhysicsObjectExternalInterface::LockRead(GetWorld()->GetPhysicsScene());
    RunChannelRaycasts();
    LockedPhysObject.Release();
  }
  else
  {
    RunChannelRaycasts();
  }

  ComputeAndSaveDetections(ActorTransform);
}
