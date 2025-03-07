// Copyright (c) 2024 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "Carla/Sensor/Radar.h"
#include "Carla.h"
#include "Carla/Actor/ActorBlueprintFunctionLibrary.h"

#include <util/disable-ue4-macros.h>
#include <carla/geom/Math.h>
#include <carla/ros2/ROS2.h>
#include <util/enable-ue4-macros.h>

#include <util/ue-header-guard-begin.h>
#include "Kismet/KismetMathLibrary.h"
#include "Async/ParallelFor.h"
#include "PhysicsEngine/PhysicsObjectExternalInterface.h"
#include <util/ue-header-guard-end.h>

FActorDefinition ARadar::GetSensorDefinition()
{
  return UActorBlueprintFunctionLibrary::MakeRadarDefinition();
}

ARadar::ARadar(const FObjectInitializer& ObjectInitializer)
  : Super(ObjectInitializer)
{
  PrimaryActorTick.bCanEverTick = true;

  RandomEngine = CreateDefaultSubobject<URandomEngine>(TEXT("RandomEngine"));

  TraceParams = FCollisionQueryParams(FName(TEXT("Laser_Trace")), true, this);
  TraceParams.bTraceComplex = true;
  TraceParams.bReturnPhysicalMaterial = false;

}

void ARadar::Set(const FActorDescription &ActorDescription)
{
  Super::Set(ActorDescription);
  UActorBlueprintFunctionLibrary::SetRadar(ActorDescription, this);
}

void ARadar::SetHorizontalFOV(float NewHorizontalFOV)
{
  HorizontalFOV = NewHorizontalFOV;
}

void  ARadar::SetVerticalFOV(float NewVerticalFOV)
{
  VerticalFOV = NewVerticalFOV;
}

void ARadar::SetRange(float NewRange)
{
  Range = NewRange;
}

void ARadar::SetPointsPerSecond(int NewPointsPerSecond)
{
  PointsPerSecond = NewPointsPerSecond;
  RadarData.SetResolution(PointsPerSecond);
}

const carla::sensor::data::RadarData& ARadar::GetRadarData() const{
  return RadarData;
}

void ARadar::BeginPlay()
{
  Super::BeginPlay();

  PrevLocation = GetActorLocation();
}

void ARadar::PostPhysTick(UWorld *World, ELevelTick TickType, float DeltaTime)
{
  TRACE_CPUPROFILER_EVENT_SCOPE(ARadar::PostPhysTick);
  CalculateCurrentVelocity(DeltaTime);

  RadarData.Reset();
  SendLineTraces(DeltaTime);

  auto DataStream = GetDataStream(*this);

  // ROS2
  #if defined(WITH_ROS2)
  auto ROS2Carla = carla::ros2::ROS2Carla::GetInstance();
  if (ROS2Carla->IsEnabled())
  {
    TRACE_CPUPROFILER_EVENT_SCOPE_STR("ARadar::PostPhysTick ROS2 Send");
    auto StreamId = carla::streaming::detail::token_type(GetToken()).get_stream_id();
    AActor* ParentActor = GetAttachParentActor();
    if (ParentActor)
    {
      FTransform LocalTransformRelativeToParent = GetActorTransform().GetRelativeTransform(ParentActor->GetActorTransform());
      ROS2Carla->ProcessDataFromRadar(DataStream.GetSensorType(), StreamId, LocalTransformRelativeToParent, RadarData, this);
    }
    else
    {
      ROS2Carla->ProcessDataFromRadar(DataStream.GetSensorType(), StreamId, DataStream.GetSensorTransform(), RadarData, this);
    }
  }
  #endif

  {
    TRACE_CPUPROFILER_EVENT_SCOPE_STR("Send Stream");
    DataStream.SerializeAndSend(*this, RadarData, DataStream.PopBufferFromPool());
  }
}

void ARadar::CalculateCurrentVelocity(const float DeltaTime)
{
  const FVector RadarLocation = GetActorLocation();
  CurrentVelocity = (RadarLocation - PrevLocation) / DeltaTime;
  PrevLocation = RadarLocation;
}

void ARadar::SendLineTraces(float DeltaTime)
{
  TRACE_CPUPROFILER_EVENT_SCOPE(ARadar::SendLineTraces);
  constexpr float TO_METERS = 1e-2;
  const FTransform& ActorTransform = GetActorTransform();
  const FRotator& TransformRotator = ActorTransform.Rotator();
  const FVector& RadarLocation = GetActorLocation();
  const FVector& ForwardVector = GetActorForwardVector();
  const FVector TransformXAxis = ActorTransform.GetUnitAxis(EAxis::X);
  const FVector TransformYAxis = ActorTransform.GetUnitAxis(EAxis::Y);
  const FVector TransformZAxis = ActorTransform.GetUnitAxis(EAxis::Z);

  // Maximum radar radius in horizontal and vertical direction
  const float MaxRx = FMath::Tan(FMath::DegreesToRadians(HorizontalFOV * 0.5f)) * Range;
  const float MaxRy = FMath::Tan(FMath::DegreesToRadians(VerticalFOV * 0.5f)) * Range;
  const int NumPoints = (int)(PointsPerSecond * DeltaTime);

  // Generate the parameters of the rays in a deterministic way
  Rays.clear();
  Rays.resize(NumPoints);
  for (int i = 0; i < Rays.size(); i++) {
    Rays[i].Radius = RandomEngine->GetUniformFloat();
    Rays[i].Angle = RandomEngine->GetUniformFloatInRange(0.0f, carla::geom::Math::Pi2<float>());
    Rays[i].Hitted = false;
  }

  auto LockedPhysObject = FPhysicsObjectExternalInterface::LockRead(GetWorld()->GetPhysicsScene());
  {
    TRACE_CPUPROFILER_EVENT_SCOPE(ParallelFor);
    ParallelFor(NumPoints, [&](int32 idx) {
      TRACE_CPUPROFILER_EVENT_SCOPE(ParallelForTask);
      FHitResult OutHit(ForceInit);
      const float Radius = Rays[idx].Radius;
      const float Angle  = Rays[idx].Angle;

      float Sin, Cos;
      FMath::SinCos(&Sin, &Cos, Angle);

      const FVector EndLocation = RadarLocation + TransformRotator.RotateVector({
        Range,
        MaxRx * Radius * Cos,
        MaxRy * Radius * Sin
      });

      // Neya TODO: Changed from ParallelLineTraceSingleByChannel
      const bool Hitted = GetWorld()->ParallelLineTraceSingleByChannel(
        OutHit,
        RadarLocation,
        EndLocation,
        ECC_GameTraceChannel2,
        TraceParams,
        FCollisionResponseParams::DefaultResponseParam
      );

      const TWeakObjectPtr<AActor> HittedActor = OutHit.GetActor();
      if (Hitted && HittedActor.Get()) {
        Rays[idx].Hitted = true;

        Rays[idx].RelativeVelocity = CalculateRelativeVelocity(OutHit, RadarLocation);

        Rays[idx].AzimuthAndElevation = FMath::GetAzimuthAndElevation (
          (EndLocation - RadarLocation).GetSafeNormal() * Range,
          TransformXAxis,
          TransformYAxis,
          TransformZAxis
        );

        Rays[idx].Distance = OutHit.Distance * TO_METERS;
      }
    });
  }
  LockedPhysObject.Release();
  
  // Write the detections in the output structure
  for (auto& ray : Rays)
  {
    if (ray.Hitted)
    {
      RadarData.WriteDetection(
      {
        ray.RelativeVelocity,
        UKismetMathLibrary::Conv_DoubleToFloat(ray.AzimuthAndElevation.X),
        UKismetMathLibrary::Conv_DoubleToFloat(ray.AzimuthAndElevation.Y),
        ray.Distance
      });
    }
  }
}

float ARadar::CalculateRelativeVelocity(const FHitResult& OutHit, const FVector& RadarLocation)
{
  constexpr float TO_METERS = 1e-2;

  const TWeakObjectPtr<AActor> HittedActor = OutHit.GetActor();
  const FVector TargetVelocity = HittedActor->GetVelocity();
  const FVector TargetLocation = OutHit.ImpactPoint;
  const FVector Direction = (TargetLocation - RadarLocation).GetSafeNormal();
  const FVector DeltaVelocity = (TargetVelocity - CurrentVelocity);
  const float V = TO_METERS * FVector::DotProduct(DeltaVelocity, Direction);

  return V;
}
