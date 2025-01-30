// Copyright (c) 2024 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "Carla/Sensor/RayCastSemanticLidar.h"
#include "Carla.h"
#include "Carla/Actor/ActorBlueprintFunctionLibrary.h"
#include "Carla/Game/Tagger.h"

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

void ARayCastSemanticLidar::PostPhysTick(UWorld *World, ELevelTick TickType, float DeltaTime)
{
  TRACE_CPUPROFILER_EVENT_SCOPE(ARayCastSemanticLidar::PostPhysTick);
  SimulateLidar(DeltaTime);

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

void ARayCastSemanticLidar::SimulateLidar(const float DeltaTime)
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

  ResetRecordedHits(ChannelCount, PointsToScanWithOneLaser);
  PreprocessRays(ChannelCount, PointsToScanWithOneLaser);

  auto LockedPhysObject = FPhysicsObjectExternalInterface::LockRead(GetWorld()->GetPhysicsScene());
  {
    TRACE_CPUPROFILER_EVENT_SCOPE(ParallelFor);
    ParallelFor(ChannelCount, [&](int32 idxChannel) {
      TRACE_CPUPROFILER_EVENT_SCOPE(ParallelForTask);

      FCollisionQueryParams TraceParams = FCollisionQueryParams(FName(TEXT("Laser_Trace")), true, this);
      TraceParams.bTraceComplex = true;
      TraceParams.bReturnPhysicalMaterial = false;

      for (auto idxPtsOneLaser = 0u; idxPtsOneLaser < PointsToScanWithOneLaser; idxPtsOneLaser++) {
        FHitResult HitResult;
        const float VertAngle = LaserAngles[idxChannel];
        const float HorizAngle = std::fmod(CurrentHorizontalAngle + AngleDistanceOfLaserMeasure
            * idxPtsOneLaser, Description.HorizontalFov) - Description.HorizontalFov / 2;
        const bool PreprocessResult = RayPreprocessCondition[idxChannel][idxPtsOneLaser];

        if (PreprocessResult && ShootLaser(VertAngle, HorizAngle, HitResult, TraceParams)) {
          WritePointAsync(idxChannel, HitResult);
        }
      };
    });
  }
  LockedPhysObject.Release();

  FTransform ActorTransf = GetTransform();
  ComputeAndSaveDetections(ActorTransf);

  const float HorizontalAngle = carla::geom::Math::ToRadians(
      std::fmod(CurrentHorizontalAngle + AngleDistanceOfTick, Description.HorizontalFov));
  SemanticLidarData.SetHorizontalAngle(HorizontalAngle);
}

void ARayCastSemanticLidar::ResetRecordedHits(uint32_t Channels, uint32_t MaxPointsPerChannel) {
  RecordedHits.resize(Channels);

  for (auto& hits : RecordedHits) {
    hits.clear();
    hits.reserve(MaxPointsPerChannel);
  }
}

void ARayCastSemanticLidar::PreprocessRays(uint32_t Channels, uint32_t MaxPointsPerChannel) {
  RayPreprocessCondition.resize(Channels);

  for (auto& conds : RayPreprocessCondition) {
    conds.clear();
    conds.resize(MaxPointsPerChannel);
    std::fill(conds.begin(), conds.end(), true);
  }
}

void ARayCastSemanticLidar::WritePointAsync(uint32_t channel, FHitResult &detection) {
	TRACE_CPUPROFILER_EVENT_SCOPE_STR(__FUNCTION__);
  DEBUG_ASSERT(GetChannelCount() > channel);
  RecordedHits[channel].emplace_back(detection);
}

void ARayCastSemanticLidar::ComputeAndSaveDetections(const FTransform& SensorTransform) {
	TRACE_CPUPROFILER_EVENT_SCOPE_STR(__FUNCTION__);
  for (auto idxChannel = 0u; idxChannel < Description.Channels; ++idxChannel)
    PointsPerChannel[idxChannel] = RecordedHits[idxChannel].size();
  SemanticLidarData.ResetMemory(PointsPerChannel);

  for (auto idxChannel = 0u; idxChannel < Description.Channels; ++idxChannel) {
    for (auto& hit : RecordedHits[idxChannel]) {
      FSemanticDetection detection;
      ComputeRawDetection(hit, SensorTransform, detection);
      SemanticLidarData.WritePointSync(detection);
    }
  }

  SemanticLidarData.WriteChannelCount(PointsPerChannel);
}

void ARayCastSemanticLidar::ComputeRawDetection(const FHitResult& HitInfo, const FTransform& SensorTransf, FSemanticDetection& Detection) const
{
    const FVector HitPoint = HitInfo.ImpactPoint;
    Detection.point = SensorTransf.Inverse().TransformPosition(HitPoint);

    const FVector VecInc = - (HitPoint - SensorTransf.GetLocation()).GetSafeNormal();
    Detection.cos_inc_angle = FVector::DotProduct(VecInc, HitInfo.ImpactNormal);

    const FActorRegistry &Registry = GetEpisode().GetActorRegistry();

    const AActor* actor = HitInfo.GetActor();
    Detection.object_idx = 0;
    
    // Given that landscapes do not have tags for now, asign it here if the actor is a landscape, otherwise get the component tag
    if (actor->IsA<ALandscape>()){
      Detection.object_tag = static_cast<uint32_t>(ATagger::GetTagFromString("Terrain"));
    }
    else {
      Detection.object_tag = static_cast<uint32_t>(ATagger::GetTagFromString(HitInfo.Component->ComponentTags[0].ToString()));
    }

    if (actor != nullptr) {

      const FCarlaActor* view = Registry.FindCarlaActor(actor);
      if(view)
        Detection.object_idx = view->GetActorId();

    }
    else {
      UE_LOG(LogCarla, Warning, TEXT("Actor not valid %p!!!!"), actor);
    }
}


bool ARayCastSemanticLidar::ShootLaser(const float VerticalAngle, const float HorizontalAngle, FHitResult& HitResult, FCollisionQueryParams& TraceParams) const
{
  TRACE_CPUPROFILER_EVENT_SCOPE_STR(__FUNCTION__);

  FHitResult HitInfo(ForceInit);

  FTransform ActorTransf = GetTransform();
  FVector LidarBodyLoc = ActorTransf.GetLocation();
  FRotator LidarBodyRot = ActorTransf.Rotator();

  FRotator LaserRot (VerticalAngle, HorizontalAngle, 0);  // float InPitch, float InYaw, float InRoll
  FRotator ResultRot = UKismetMathLibrary::ComposeRotators(
    LaserRot,
    LidarBodyRot
  );

  const auto Range = Description.Range;
  FVector EndTrace = Range * UKismetMathLibrary::GetForwardVector(ResultRot) + LidarBodyLoc;
  
  GetWorld()->ParallelLineTraceSingleByChannel(
    HitInfo,
    LidarBodyLoc,
    EndTrace,
    ECC_GameTraceChannel2,
    TraceParams,
    FCollisionResponseParams::DefaultResponseParam
  );

  if (HitInfo.bBlockingHit) {
    HitResult = HitInfo;
    return true;
  } else {
    return false;
  }
}
