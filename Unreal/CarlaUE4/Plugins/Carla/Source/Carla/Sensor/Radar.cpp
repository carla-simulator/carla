// Copyright (c) 2019 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "Carla.h"
#include "Carla/Sensor/Radar.h"
#include "Carla/Actor/ActorBlueprintFunctionLibrary.h"

FActorDefinition ARadar::GetSensorDefinition()
{
  return UActorBlueprintFunctionLibrary::MakeRadarDefinition();
}

ARadar::ARadar(const FObjectInitializer& ObjectInitializer)
  : Super(ObjectInitializer)
{
  PrimaryActorTick.bCanEverTick = true;

  Distance = 5000.0f;
  FOV = 30.0f;

  Overture = 10;
  Steps = 10;
}

void ARadar::Set(const FActorDescription &ActorDescription)
{
  Super::Set(ActorDescription);
  UActorBlueprintFunctionLibrary::SetRadar(ActorDescription, this);
}

void ARadar::SetResolution(int Value)
{
  Resolution = Value;
  RadarData.SetResolution(Resolution);
}

void ARadar::SetFOVAndSteps(float NewFov, int NewSteps)
{
  FOV = NewFov;
  Steps = NewSteps;
  PreCalculateCosSin();
}

void ARadar::BeginPlay()
{
  Super::BeginPlay();

  World = GetWorld();

  PrevLocation = GetActorLocation();

  PreCalculateCosSin();

  // Prepare LineTrace params
  LineTraceObjectQueryParams.AddObjectTypesToQuery(ECC_WorldStatic);
  LineTraceObjectQueryParams.AddObjectTypesToQuery(ECC_WorldDynamic);
  LineTraceObjectQueryParams.AddObjectTypesToQuery(ECC_Pawn);
  LineTraceObjectQueryParams.AddObjectTypesToQuery(ECC_PhysicsBody);
  LineTraceObjectQueryParams.AddObjectTypesToQuery(ECC_Vehicle);
  LineTraceObjectQueryParams.AddObjectTypesToQuery(ECC_Destructible);

  LineTraceQueryParams.bTraceComplex = false;
  LineTraceQueryParams.AddIgnoredActor(this);

}

void ARadar::Tick(const float DeltaTime)
{
  Super::Tick(DeltaTime);

  namespace css = carla::sensor::s11n;

  SendLineTraces(DeltaTime);

  // RadarData usage example:
  //RadarData.Reset();
  //css::RadarDetection Detection = {0.0f, 0.0f, 0.0f, 0.0f};
  //RadarData.WriteDetection(Detection);

  auto DataStream = GetDataStream(*this);
  DataStream.Send(*this, RadarData, DataStream.PopBufferFromPool());
}

void ARadar::PreCalculateCosSin()
{

  const float Angle = FMath::DegreesToRadians(360.0f / Steps);

  PreCalculatedCosSin.Empty(Steps);
  for(int i=0; i < Steps; i++)
  {
    float Sin, Cos;
    FMath::SinCos(&Sin, &Cos, Angle * i );

    PreCalculatedCosSin.Add({Cos, Sin});
  }
}

void ARadar::SendLineTraces(float DeltaSeconds)
{
  FHitResult OutHit;

  const FVector RadarLocation = GetActorLocation();
  const FVector ForwardVector = GetActorForwardVector();

  // Calculate Current Radar Velocity
  // Used to convert from UE4's cm to meters
  constexpr float TO_METERS = 1e-2;
  const FVector CurrentVelocity = TO_METERS * (RadarLocation - PrevLocation) / DeltaSeconds;
  PrevLocation = RadarLocation;

  DetectedActors.Reset();

  float tan_half_fov = FMath::Tan(FMath::DegreesToRadians(FOV * 0.5f));
  float outer_radius = tan_half_fov * Distance;
  // TODO: if overture is zero => error; just center ray
  float inv_overture = 1.0f / Overture;
  float inner_increment = outer_radius * inv_overture;

  FVector WorldForwardVector = ForwardVector * Distance;
  FVector EndLocation = RadarLocation + WorldForwardVector;

  // The center should be a single line
  //DrawDebugLine(GetWorld(), RadarLocation, EndLocation, FColor::Cyan, false, 0.5f, 0, 10.0f);
  bool Hitted = World->LineTraceSingleByObjectType(
    OutHit,
    RadarLocation,
    EndLocation,
    LineTraceObjectQueryParams,
    LineTraceQueryParams
  );

  if (Hitted)
  {
    DetectedActors.Add(OutHit);
  }

  for(int j = 0; j < Steps; j++)
  {
    const CosSinData& CosSin = PreCalculatedCosSin[j];

    EndLocation = RadarLocation + WorldForwardVector;

    for(int i = 1; i <= Overture; i++)
    {
      EndLocation += FVector(0.0f, CosSin.Cos, CosSin.Sin) * inner_increment;

      Hitted = World->LineTraceSingleByObjectType(
          OutHit,
          RadarLocation,
          EndLocation,
          LineTraceObjectQueryParams,
          LineTraceQueryParams
        );

      TWeakObjectPtr<AActor> HittedActor = OutHit.Actor;

      if (Hitted && HittedActor.Get() && Cast<AWheeledVehicle>(HittedActor) ) {

        // Calculate Doppler speed
        // 𝐹𝑑 = 2𝑉 (𝐹0/𝑐) cos(𝜃)
        //  𝐹𝑑 = Doppler shift (Hz)
        //  𝑉 = Velocity
        //  𝐹0 = Original wave frequency (Hz)
        //  𝑐 = Speed of light
        //  θ = Offset angle of sensor relative to direction of object motion
        // 𝐹0 = 35.5 ± 0.1 𝐺𝐻 ;
        //  The frequency of the output increases by 105.8 ± 0.3 Hz for every mph (65.74 ± 0.19 Hz per kph) of velocity

        FVector V = TO_METERS * (HittedActor->GetVelocity() - CurrentVelocity);
        constexpr float F0 = 35.5e9;
        constexpr float FDeltaOutput = 62.1;
        constexpr float LIGHT_SPEED = 3e8;
        float CosTheta = FVector::DotProduct(HittedActor->GetActorForwardVector(), ForwardVector);
        FVector Fd = 2.0f * V * (F0 / LIGHT_SPEED) * CosTheta;

        constexpr float KMPH_TO_MPS = 3600.0f / 1000.0f;
        FVector RelativeVelocity = Fd / (FDeltaOutput * KMPH_TO_MPS);

        UE_LOG(LogCarla, Warning, TEXT("Fd %s - RelativeVelocity %s - Current %s - OtherVelocity %s"),
        *Fd.ToString(), *RelativeVelocity.ToString(), *CurrentVelocity.ToString(), *(TO_METERS * Cast<AWheeledVehicle>(HittedActor)->GetVelocity()).ToString());

        DetectedActors.Add(OutHit);
      }


      if(ShowDebug && CurrentDebugDelay == ShowDebugDelay)
      {

        if(ShowDebugLines)
        {
          DrawDebugLine(World, RadarLocation, (Hitted && !ShowCompleteLines) ? OutHit.ImpactPoint: EndLocation, FColor::Cyan, false, 0.5f, 0, LineThickness);
        }

        if(Hitted)
        {
          if(ShowDebugHits)
          {
            DrawDebugSphere(World, OutHit.ImpactPoint, 50.0f, 12, FColor::Red, false, 0.5f, 0);
          }
        }
      }
    }
  }

  CurrentDebugDelay++;
  if(CurrentDebugDelay > ShowDebugDelay) {
    CurrentDebugDelay = 0;
  }

}
