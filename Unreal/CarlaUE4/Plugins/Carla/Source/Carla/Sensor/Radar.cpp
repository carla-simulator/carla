// Copyright (c) 2019 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "Carla.h"
#include "Carla/Sensor/Radar.h"
#include "Carla/Actor/ActorBlueprintFunctionLibrary.h"
#include "Kismet/KismetMathLibrary.h"

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

  // TODO: default params

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

  World = GetWorld();

  PrevLocation = GetActorLocation();
}

void ARadar::Tick(const float DeltaTime)
{
  Super::Tick(DeltaTime);

  CalculateCurrentVelocity(DeltaTime);

  RadarData.Reset();
  SendLineTraces();

  auto DataStream = GetDataStream(*this);
  DataStream.Send(*this, RadarData, DataStream.PopBufferFromPool());
}

void ARadar::CalculateCurrentVelocity(const float DeltaTime)
{
  const FVector RadarLocation = GetActorLocation();
  CurrentVelocity = (RadarLocation - PrevLocation) / DeltaTime;
  PrevLocation = RadarLocation;
}

void ARadar::SendLineTraces()
{

  constexpr float TO_METERS = 1e-2;
  FHitResult OutHit(ForceInit);
  const FTransform& ActorTransform = GetActorTransform();
  const FRotator& TransformRotator = ActorTransform.Rotator();
  const FVector& RadarLocation = GetActorLocation();
  const FVector& ForwardVector = GetActorForwardVector();
  const FVector TransformXAxis = ActorTransform.GetUnitAxis(EAxis::X);
  const FVector TransformYAxis = ActorTransform.GetUnitAxis(EAxis::Y);
  const FVector TransformZAxis = ActorTransform.GetUnitAxis(EAxis::Z);

  // Maximun radar radius in horizontal and vertical direction
  const float MaxRx = FMath::Tan(FMath::DegreesToRadians(HorizontalFOV * 0.5f)) * Range;
  const float MaxRy = FMath::Tan(FMath::DegreesToRadians(VerticalFOV * 0.5f)) * Range;

  for (int i = 0; i < PointsPerSecond; i++)
  {
    float Radius = RandomEngine->GetUniformFloat();
    float Angle = RandomEngine->GetUniformFloatInRange(0.0f, carla::geom::Math::Pi2<float>());
    float Sin, Cos;
    FMath::SinCos(&Sin, &Cos, Angle);
    FVector Rotation = TransformRotator.RotateVector({
      0.0f,
      MaxRx * Radius * Cos,
      MaxRy * Radius * Sin
    });
    FVector EndLocation = Rotation * Range;

    bool Hitted = World->LineTraceSingleByChannel(
      OutHit,
      RadarLocation,
      EndLocation,
      ECC_MAX,
      TraceParams,
      FCollisionResponseParams::DefaultResponseParam
    );

    TWeakObjectPtr<AActor> HittedActor = OutHit.Actor;
    if (Hitted && HittedActor.Get()) {

      const float RelativeVelocity = CalculateRelativeVelocity(OutHit, RadarLocation, ForwardVector);

      FVector2D AzimuthAndElevation = FMath::GetAzimuthAndElevation (
        (EndLocation - RadarLocation).GetSafeNormal() * Range,
        TransformXAxis,
        TransformYAxis,
        TransformZAxis
      );

      RadarData.WriteDetection({
        RelativeVelocity,
        AzimuthAndElevation.X,
        AzimuthAndElevation.Y,
        OutHit.Distance * TO_METERS
      });
    }
  }
}

float ARadar::CalculateRelativeVelocity(const FHitResult& OutHit, const FVector& RadarLocation, const FVector& ForwardVector)
{
  constexpr float TO_METERS = 1e-2;

  TWeakObjectPtr<AActor> HittedActor = OutHit.Actor;
  FVector TargetVelocity = HittedActor->GetVelocity();
  FVector TargetLocation = OutHit.ImpactPoint;
  FVector Direction = (TargetLocation - RadarLocation).GetSafeNormal();
  const FVector DeltaVelocity = (TargetVelocity - CurrentVelocity);
  const float V = TO_METERS * FVector::DotProduct(DeltaVelocity, ForwardVector);

  return V;
}
