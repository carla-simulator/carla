// Copyright (c) 2019 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include <PxScene.h>

#include "Carla.h"
#include "Carla/Sensor/Radar.h"
#include "Carla/Actor/ActorBlueprintFunctionLibrary.h"
#include "Kismet/KismetMathLibrary.h"
#include "Runtime/Core/Public/Async/ParallelFor.h"

#include "carla/geom/Math.h"

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

  {
    TRACE_CPUPROFILER_EVENT_SCOPE_STR("Send Stream");
    auto DataStream = GetDataStream(*this);
    DataStream.Send(*this, RadarData, DataStream.PopBufferFromPool());
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

  FCriticalSection Mutex;
  GetWorld()->GetPhysicsScene()->GetPxScene()->lockRead();
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

      const bool Hitted = GetWorld()->ParallelLineTraceSingleByChannel(
        OutHit,
        RadarLocation,
        EndLocation,
        ECC_GameTraceChannel2,
        TraceParams,
        FCollisionResponseParams::DefaultResponseParam
      );

      const TWeakObjectPtr<AActor> HittedActor = OutHit.Actor;
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
  GetWorld()->GetPhysicsScene()->GetPxScene()->unlockRead();

  // Write the detections in the output structure
  for (auto& ray : Rays) {
    if (ray.Hitted) {
      RadarData.WriteDetection({
        ray.RelativeVelocity,
        ray.AzimuthAndElevation.X,
        ray.AzimuthAndElevation.Y,
        ray.Distance
      });
    }
  }

}

float ARadar::CalculateRelativeVelocity(const FHitResult& OutHit, const FVector& RadarLocation)
{
  constexpr float TO_METERS = 1e-2;

  const TWeakObjectPtr<AActor> HittedActor = OutHit.Actor;
  const FVector TargetVelocity = HittedActor->GetVelocity();
  const FVector TargetLocation = OutHit.ImpactPoint;
  const FVector Direction = (TargetLocation - RadarLocation).GetSafeNormal();
  const FVector DeltaVelocity = (TargetVelocity - CurrentVelocity);
  const float V = TO_METERS * FVector::DotProduct(DeltaVelocity, Direction);

  return V;
}
