// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "Carla/Sensor/RayCastSemanticLidar.h"
#include "Carla.h"
#include "Carla/Actor/ActorBlueprintFunctionLibrary.h"
#include "Carla/Game/Tagger.h"
#include "Carla/Sensor/CarlaLidarSubsystem.h"

#include <util/disable-ue4-macros.h>
#include "carla/geom/Math.h"
#include "carla/ros2/ROS2.h"
#include <util/enable-ue4-macros.h>

#include <util/ue-header-guard-begin.h>
#include "DrawDebugHelpers.h"
#include "Engine/CollisionProfile.h"
#include "Kismet/KismetMathLibrary.h"
#include "PhysicsEngine/PhysicsObjectExternalInterface.h"
#include "Async/ParallelFor.h"
#include <util/ue-header-guard-end.h>
#include "Landscape.h"

#include <cmath>

namespace crp = carla::rpc;

FActorDefinition ARayCastSemanticLidar::GetSensorDefinition()
{
  return UActorBlueprintFunctionLibrary::MakeLidarDefinition(TEXT("ray_cast_semantic"));
}

ARayCastSemanticLidar::ARayCastSemanticLidar(const FObjectInitializer& ObjectInitializer)
  : Super(ObjectInitializer)
{
  PrimaryActorTick.bCanEverTick = true;
}

void ARayCastSemanticLidar::BeginPlay()
{
  Super::BeginPlay();
  if (UCarlaLidarSubsystem* LidarSubsystem = GetWorld()->GetSubsystem<UCarlaLidarSubsystem>())
  {
    LidarSubsystem->RegisterLidar(this);
  }
}

void ARayCastSemanticLidar::EndPlay(EEndPlayReason::Type EndPlayReason)
{
  if (UCarlaLidarSubsystem* LidarSubsystem = GetWorld()->GetSubsystem<UCarlaLidarSubsystem>())
  {
    LidarSubsystem->UnregisterLidar(this);
  }
  Super::EndPlay(EndPlayReason);
}

void ARayCastSemanticLidar::Set(const FActorDescription &ActorDescription)
{
  Super::Set(ActorDescription);
  FLidarDescription LidarDescription;
  UActorBlueprintFunctionLibrary::SetLidar(ActorDescription, LidarDescription);
  Set(LidarDescription);
}

void ARayCastSemanticLidar::Set(const FLidarDescription &LidarDescription)
{
  Description = LidarDescription;
  SemanticLidarData = FSemanticLidarData(Description.Channels);
  CreateLasers();
  PointsPerChannel.resize(Description.Channels);
}

void ARayCastSemanticLidar::CreateLasers()
{
  const auto NumberOfLasers = Description.Channels;
  check(NumberOfLasers > 0u);
  const float DeltaAngle = NumberOfLasers == 1u ? 0.f :
    (Description.UpperFovLimit - Description.LowerFovLimit) /
    static_cast<float>(NumberOfLasers - 1);
  LaserAngles.Empty(NumberOfLasers);
  for(auto i = 0u; i < NumberOfLasers; ++i)
  {
    const float VerticalAngle =
        Description.UpperFovLimit - static_cast<float>(i) * DeltaAngle;
    LaserAngles.Emplace(VerticalAngle);
  }
}

void ARayCastSemanticLidar::SendData(const float DeltaTime)
{
  TRACE_CPUPROFILER_EVENT_SCOPE(ARayCastSemanticLidar::SendData);
  auto DataStream = GetDataStream(*this);
  auto SensorTransform = DataStream.GetSensorTransform();
  {
    TRACE_CPUPROFILER_EVENT_SCOPE_STR("Send Stream");
    DataStream.SerializeAndSend(*this, SemanticLidarData, DataStream.PopBufferFromPool());
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
      ROS2->ProcessDataFromSemanticLidar(DataStream.GetSensorType(), StreamId, LocalTransformRelativeToParent, SemanticLidarData, this);
    }
    else
    {
      ROS2->ProcessDataFromSemanticLidar(DataStream.GetSensorType(), StreamId, SensorTransform, SemanticLidarData, this);
    }
  }
  #endif
}

void ARayCastSemanticLidar::SimulateLidar(const float DeltaTime, bool bLockPhysics)
{
  TRACE_CPUPROFILER_EVENT_SCOPE(ARayCastSemanticLidar::SimulateLidar);
  const uint32 ChannelCount = Description.Channels;
  const uint32 PointsToScanWithOneLaser =
    FMath::RoundHalfFromZero(
        Description.PointsPerSecond * DeltaTime / float(ChannelCount));

  if (PointsToScanWithOneLaser <= 0)
  {
    UE_LOG(
        LogCarla,
        Warning,
        TEXT("%s: no points requested this frame, try increasing the number of points per second."),
        *GetName());
    return;
  }

  check(ChannelCount == LaserAngles.Num());

  const float CurrentHorizontalAngle = carla::geom::Math::ToDegrees(
      SemanticLidarData.GetHorizontalAngle());
  const float AngleDistanceOfTick = Description.RotationFrequency * Description.HorizontalFov
      * DeltaTime;
  const float AngleDistanceOfLaserMeasure = AngleDistanceOfTick / PointsToScanWithOneLaser;
  const float HalfHorizontalFov = Description.HorizontalFov / 2;

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
        // Rays dropped by a subclass's PreprocessRays.
        if (!ChannelPreprocessCondition[idxPtsOneLaser])
        {
          continue;
        }

        FHitResult HitResult;
        const float HorizAngle = std::fmod(CurrentHorizontalAngle + AngleDistanceOfLaserMeasure
            * idxPtsOneLaser, Description.HorizontalFov) - HalfHorizontalFov;

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

  const float HorizontalAngle = carla::geom::Math::ToRadians(
      std::fmod(CurrentHorizontalAngle + AngleDistanceOfTick, Description.HorizontalFov));
  SemanticLidarData.SetHorizontalAngle(HorizontalAngle);
}

void ARayCastSemanticLidar::PreprocessRays(uint32_t Channels, uint32_t MaxPointsPerChannel) {
  RayPreprocessCondition.resize(Channels);

  for (auto& conds : RayPreprocessCondition) {
    conds.clear();
    conds.resize(MaxPointsPerChannel);
    std::fill(conds.begin(), conds.end(), true);
  }
}

void ARayCastSemanticLidar::ResetDetections(uint32_t Channels, uint32_t MaxPointsPerChannel) {
  ResetChannelDetections(SemanticDetections, Channels, MaxPointsPerChannel);
}

void ARayCastSemanticLidar::WriteDetectionAsync(uint32_t Channel, const FHitResult& HitInfo, const FTransform& InverseSensorTransform, const FVector& SensorLocation) {
  DEBUG_ASSERT(GetChannelCount() > Channel);
  FSemanticDetection& Detection = SemanticDetections[Channel].emplace_back();
  ComputeRawDetection(HitInfo, InverseSensorTransform, SensorLocation, Detection);
}

void ARayCastSemanticLidar::ComputeAndSaveDetections(const FTransform& SensorTransform)
{
  TRACE_CPUPROFILER_EVENT_SCOPE_STR(__FUNCTION__);

  for (auto idxChannel = 0u; idxChannel < Description.Channels; ++idxChannel)
  {
    PointsPerChannel[idxChannel] = SemanticDetections[idxChannel].size();
  }

  SemanticLidarData.ResetMemory(PointsPerChannel);
  for (auto idxChannel = 0u; idxChannel < Description.Channels; ++idxChannel)
  {
    SemanticLidarData.WritePointsSync(SemanticDetections[idxChannel]);
  }
  SemanticLidarData.WriteChannelCount(PointsPerChannel);
}

void ARayCastSemanticLidar::ComputeRawDetection(const FHitResult& HitInfo, const FTransform& InverseSensorTransform, const FVector& SensorLocation, FSemanticDetection& Detection) const
{
    static const uint32_t TerrainTag = static_cast<uint32_t>(ATagger::GetTagFromString("Terrain"));

    const FVector HitPoint = HitInfo.ImpactPoint;
    Detection.point = InverseSensorTransform.TransformPosition(HitPoint);

    const FVector VecInc = - (HitPoint - SensorLocation).GetSafeNormal();
    Detection.cos_inc_angle = FVector::DotProduct(VecInc, HitInfo.ImpactNormal);

    const FActorRegistry &Registry = GetEpisode().GetActorRegistry();

    const AActor* actor = HitInfo.GetActor();
    Detection.object_idx = 0;
    Detection.object_tag = static_cast<uint32_t>(crp::CityObjectLabel::None);

    // Given that landscapes do not have tags for now, assign it here if the
    // actor is a landscape, otherwise get the component tag.
    // GetTagOfTaggedComponent tolerates untagged components (World Partition
    // streams in HLOD/proxy geometry that never went through ATagger) by
    // falling back to the stencil value, where reading ComponentTags[0]
    // directly asserts on an empty array.
    if (actor != nullptr && actor->IsA<ALandscape>())
    {
      Detection.object_tag = TerrainTag;
    }
    else if (HitInfo.Component.IsValid())
    {
      Detection.object_tag = static_cast<uint32_t>(ATagger::GetTagOfTaggedComponent(*HitInfo.Component));
    }

    if (actor != nullptr)
    {
      const FCarlaActor* view = Registry.FindCarlaActor(actor);
      if (view != nullptr)
      {
        Detection.object_idx = view->GetActorId();
      }
    }
    else
    {
      UE_LOG(LogCarla, Warning, TEXT("Actor not valid %p!!!!"), actor);
    }
}


bool ARayCastSemanticLidar::ShootLaser(const float VerticalAngle, const float HorizontalAngle, FHitResult& HitResult, FCollisionQueryParams& TraceParams,
                                       const FVector& LidarBodyLocation, const FRotator& LidarBodyRotation) const
{
  TRACE_CPUPROFILER_EVENT_SCOPE_STR(__FUNCTION__);

  FRotator LaserRotation(VerticalAngle, HorizontalAngle, 0);  // float InPitch, float InYaw, float InRoll
  FRotator ResultRotation = UKismetMathLibrary::ComposeRotators(
    LaserRotation,
    LidarBodyRotation
  );

  const auto Range = Description.Range;
  FVector EndTrace = Range * UKismetMathLibrary::GetForwardVector(ResultRotation) + LidarBodyLocation;
  
  return GetWorld()->ParallelLineTraceSingleByChannel(
    HitResult,
    LidarBodyLocation,
    EndTrace,
    ECC_GameTraceChannel2,
    TraceParams,
    FCollisionResponseParams::DefaultResponseParam
  );
}
