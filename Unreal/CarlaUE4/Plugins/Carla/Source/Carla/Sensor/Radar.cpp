// Copyright (c) 2019 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "Carla.h"
#include "Carla/Sensor/Radar.h"
#include "Carla/Actor/ActorBlueprintFunctionLibrary.h"
#include "Kismet/KismetMathLibrary.h"

FActorDefinition ARadar::GetSensorDefinition()
{
  return UActorBlueprintFunctionLibrary::MakeRadarDefinition();
}

ARadar::ARadar(const FObjectInitializer& ObjectInitializer)
  : Super(ObjectInitializer)
{
  PrimaryActorTick.bCanEverTick = true;

  TraceParams = FCollisionQueryParams(FName(TEXT("Laser_Trace")), true, this);
  TraceParams.bTraceComplex = true;
  TraceParams.bReturnPhysicalMaterial = false;
}

void ARadar::Set(const FActorDescription &ActorDescription)
{
  Super::Set(ActorDescription);
  UActorBlueprintFunctionLibrary::SetRadar(ActorDescription, this);
}

void ARadar::SetFOVAndSteps(float NewFov, int NewSteps)
{
  FOV = NewFov;
  Steps = NewSteps;
  PreCalculateCosSin();
  PreCalculateLineTraceIncrement();
}

void ARadar::SetDistance(float NewDistance)
{
  Distance = NewDistance;
  PreCalculateLineTraceIncrement();
}

void ARadar::SetAperture(int NewAperture)
{
  Aperture = NewAperture;
  PreCalculateLineTraceIncrement();
}

void ARadar::BeginPlay()
{
  Super::BeginPlay();

  World = GetWorld();

  PrevLocation = GetActorLocation();

  PreCalculateCosSin();
}

void ARadar::Tick(const float DeltaTime)
{
  Super::Tick(DeltaTime);

  namespace css = carla::sensor::s11n;

  CalculateCurrentVelocity(DeltaTime);

  RadarData.Reset();
  SendLineTraces(DeltaTime);

  auto DataStream = GetDataStream(*this);
  DataStream.Send(*this, RadarData, DataStream.PopBufferFromPool());
}

void ARadar::CalculateCurrentVelocity(const float DeltaTime)
{
  // Calculate Current Radar Velocity
  // Used to convert from UE4's cm to meters
  constexpr float TO_METERS = 1e-2;
  const FVector RadarLocation = GetActorLocation();
  CurrentVelocity = TO_METERS * (RadarLocation - PrevLocation) / DeltaTime;
  PrevLocation = RadarLocation;
}

void ARadar::PreCalculateCosSin()
{
  AngleIncrement = FMath::DegreesToRadians(360.0f / Steps);
  PreCalculatedCosSin.Empty(Steps);
  for(int i=0; i < Steps; i++)
  {
    float Sin, Cos;
    FMath::SinCos(&Sin, &Cos, AngleIncrement * i);
    PreCalculatedCosSin.Add({Cos, Sin});
  }
}

void ARadar::PreCalculateLineTraceIncrement()
{
  // Compute and set the total amount of rays
  uint32 TotalRayNumber = (Steps * Aperture) + 1u;
  RadarData.SetResolution(TotalRayNumber);
  LineTraceIncrement = FMath::Tan(
      FMath::DegreesToRadians(FOV * 0.5f)) * Distance / (Aperture + 1);
}

void ARadar::SendLineTraces(float DeltaSeconds)
{
  constexpr float TO_METERS = 1e-2;
  FHitResult OutHit(ForceInit);

  const FVector RadarLocation = GetActorLocation();
  const FVector ForwardVector = GetActorForwardVector();

  FVector WorldForwardVector = ForwardVector * Distance;
  FVector EndLocation = RadarLocation + WorldForwardVector;

  bool Hitted = World->LineTraceSingleByChannel(
    OutHit,
    RadarLocation,
    EndLocation,
    ECC_MAX,
    TraceParams,
    FCollisionResponseParams::DefaultResponseParam
  );

  if (Hitted)
  {
    const float RelativeVelocity = CalculateRelativeVelocity(OutHit, ForwardVector);
    RadarData.WriteDetection({RelativeVelocity, 0.0f, 0.0f, OutHit.Distance * TO_METERS});
  }

  for(int j = 0; j < Steps; j++)
  {
    const CosSinData& CosSin = PreCalculatedCosSin[j];

    EndLocation = RadarLocation + WorldForwardVector;

    for(int i = 1; i <= Aperture; i++)
    {
      EndLocation += FVector(0.0f, CosSin.Cos, CosSin.Sin) * LineTraceIncrement;

      Hitted = World->LineTraceSingleByChannel(
          OutHit,
          RadarLocation,
          EndLocation,
          ECC_MAX,
          TraceParams,
          FCollisionResponseParams::DefaultResponseParam
      );

      TWeakObjectPtr<AActor> HittedActor = OutHit.Actor;
      if (Hitted && HittedActor.Get()) {
        const float RelativeVelocity = CalculateRelativeVelocity(OutHit, ForwardVector);

        float Azimuth;
        float Elevation;
        UKismetMathLibrary::GetAzimuthAndElevation(
          (EndLocation - RadarLocation),
          GetActorTransform(),
          Azimuth,
          Elevation
        );

        RadarData.WriteDetection({
          RelativeVelocity,
          FMath::DegreesToRadians(Azimuth),
          FMath::DegreesToRadians(Elevation),
          OutHit.Distance * TO_METERS
        });
      }

      // TODO: delete debug?
      if(ShowDebug && CurrentDebugDelay == ShowDebugDelay)
      {
        if(ShowDebugLines)
        {
          DrawDebugLine(
              World,
              RadarLocation,
              (Hitted && !ShowCompleteLines) ? OutHit.ImpactPoint: EndLocation,
              FColor::Cyan,
              false,
              0.4f,
              0,
              LineThickness);
        }

        if(Hitted && ShowDebugHits)
        {
          DrawDebugSphere(World, OutHit.ImpactPoint, 25.0f, 6, FColor::Red, false, 0.5f, 0);
        }
      }
    }
  }

  CurrentDebugDelay++;
  if(CurrentDebugDelay > ShowDebugDelay) {
    CurrentDebugDelay = 0;
  }
}

float ARadar::CalculateRelativeVelocity(const FHitResult& OutHit, const FVector& ForwardVector) {
  // Calculate Doppler speed
  // 𝐹𝑑 = 2𝑉 (𝐹0/𝑐) cos(𝜃)
  // 𝐹𝑑 = Doppler shift (Hz)
  //  𝑉 = Velocity
  // 𝐹0 = Original wave frequency (Hz)
  //  𝑐 = Speed of light
  //  θ = Offset angle of sensor relative to direction of object motion
  // 𝐹0 = 35.5 ± 0.1 𝐺𝐻 ;
  //  The frequency of the output increases by 105.8 ± 0.3 Hz for every mph (65.74 ± 0.19 Hz per kph) of velocity

  constexpr float TO_METERS = 1e-2;
  constexpr float KMPH_TO_MPS = 3600.0f / 1000.0f;
  constexpr float F0 = 35.5e9;
  constexpr float FDeltaOutput = 62.1;
  constexpr float LIGHT_SPEED = 3e8;
  constexpr float Fd_CONSTANT = 2.0f * (F0 / LIGHT_SPEED);

  TWeakObjectPtr<AActor> HittedActor = OutHit.Actor;
  FVector TargetVelocity = TO_METERS * HittedActor->GetVelocity();
  const float V = TargetVelocity.ProjectOnTo(CurrentVelocity).Size();
  const float CosTheta = FVector::DotProduct(HittedActor->GetActorForwardVector(), ForwardVector);
  const float Fd = Fd_CONSTANT * V * CosTheta;
  const float RelativeVelocity = Fd / (FDeltaOutput * KMPH_TO_MPS);

  return RelativeVelocity;
}
