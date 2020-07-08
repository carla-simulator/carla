// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include <PxScene.h>
#include <cmath>
#include "Carla.h"
#include "Carla/Sensor/RayCastRawLidar.h"
#include "Carla/Actor/ActorBlueprintFunctionLibrary.h"
#include "carla/geom/Math.h"

#include <compiler/disable-ue4-macros.h>
#include "carla/geom/Math.h"
#include <compiler/enable-ue4-macros.h>

#include "DrawDebugHelpers.h"
#include "Engine/CollisionProfile.h"
#include "Runtime/Engine/Classes/Kismet/KismetMathLibrary.h"
#include "Runtime/Core/Public/Async/ParallelFor.h"

FActorDefinition ARayCastRawLidar::GetSensorDefinition()
{
  return UActorBlueprintFunctionLibrary::MakeLidarDefinition(TEXT("ray_cast_raw"));
}

ARayCastRawLidar::ARayCastRawLidar(const FObjectInitializer& ObjectInitializer)
  : Super(ObjectInitializer)
{
  PrimaryActorTick.bCanEverTick = true;

  RandomEngine = CreateDefaultSubobject<URandomEngine>(TEXT("RandomEngine"));
}

void ARayCastRawLidar::Set(const FActorDescription &ActorDescription)
{
  Super::Set(ActorDescription);
  FLidarDescription LidarDescription;
  UActorBlueprintFunctionLibrary::SetLidar(ActorDescription, LidarDescription);
  Set(LidarDescription);
}

void ARayCastRawLidar::Set(const FLidarDescription &LidarDescription)
{
  Description = LidarDescription;
  LidarMeasurement = FLidarMeasurement(Description.Channels);
  CreateLasers();
}

void ARayCastRawLidar::CreateLasers()
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

  // Compute drop off model parameters
  DropOffBeta = 1.0f - Description.DropOffAtZeroIntensity;
  DropOffAlpha = Description.DropOffAtZeroIntensity / Description.DropOffIntensityLimit;
  DropOffGenActive = Description.DropOffGenRate > std::numeric_limits<float>::epsilon();
}

void ARayCastRawLidar::Tick(const float DeltaTime)
{
  Super::Tick(DeltaTime);

  ReadPoints(DeltaTime);

  auto DataStream = GetDataStream(*this);
  DataStream.Send(*this, LidarMeasurement, DataStream.PopBufferFromPool());
}

void ARayCastRawLidar::ReadPoints(const float DeltaTime)
{
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
      LidarMeasurement.GetHorizontalAngle());
  const float AngleDistanceOfTick = Description.RotationFrequency * 360.0f * DeltaTime;
  const float AngleDistanceOfLaserMeasure = AngleDistanceOfTick / PointsToScanWithOneLaser;

  LidarMeasurement.Reset(ChannelCount, PointsToScanWithOneLaser);


  GetWorld()->GetPhysicsScene()->GetPxScene()->lockRead();
  ParallelFor(ChannelCount, [&](int32 idxChannel) {

    FCriticalSection Mutex;
    ParallelFor(PointsToScanWithOneLaser, [&](int32 idxPtsOneLaser) {
      FVector Point;
      float Intensity;
      const float Angle = CurrentHorizontalAngle + AngleDistanceOfLaserMeasure * idxPtsOneLaser;
      if (ShootLaser(idxChannel, Angle, Point, Intensity)) {
        Mutex.Lock();
        LidarMeasurement.WritePointAsync(idxChannel, {Point, Intensity});
        Mutex.Unlock();
      }
    });
  });
  GetWorld()->GetPhysicsScene()->GetPxScene()->unlockRead();

  LidarMeasurement.SaveDetections();


  const float HorizontalAngle = carla::geom::Math::ToRadians(
      std::fmod(CurrentHorizontalAngle + AngleDistanceOfTick, 360.0f));
  LidarMeasurement.SetHorizontalAngle(HorizontalAngle);
}

float ARayCastRawLidar::ComputeIntensity(const FVector &LidarBodyLoc, const FHitResult& HitInfo) const
{
  return 0.0;

  const FVector HitPoint = HitInfo.ImpactPoint - LidarBodyLoc;
  const float Distance = 0.01f * HitPoint.Size();

  const float AttenAtm = Description.AtmospAttenRate;
  const float AbsAtm = exp(-AttenAtm * Distance);

  const FActorRegistry &Registry = GetEpisode().GetActorRegistry();

  uint8 label = 69;

//  AActor* actor = HitInfo.Actor.Get();
//  if (actor != nullptr) {
//    FActorView view = Registry.Find(actor);
//
//    if(view.IsValid()){
//      const FActorInfo* ActorInfo = view.GetActorInfo();
//
//      if(ActorInfo != nullptr) {
//        //TSet<ECityObjectLabel> labels = ActorInfo->SemanticTags;
//        //if(labels.Num() == 1)
//        //    label = static_cast<uint8>(*labels.CreateConstIterator());
//      }
//      else {
//        UE_LOG(LogCarla, Warning, TEXT("Info not valid!!!!"));
//      }
//    }
//    else {
//      UE_LOG(LogCarla, Warning, TEXT("View not valid %p!!!!"), view.GetActor());
//    }
//
//  }
//  else {
//    UE_LOG(LogCarla, Warning, TEXT("Actor not found!!!!"));
//  }

  const float IntRec = AbsAtm;

  return IntRec;
}

bool ARayCastRawLidar::ShootLaser(const uint32 Channel, const float HorizontalAngle, FVector &XYZ, float &Intensity) const
{

  if(DropOffGenActive && RandomEngine->GetUniformFloat() < Description.DropOffGenRate)
    return false;

  const float VerticalAngle = LaserAngles[Channel];

  FCollisionQueryParams TraceParams = FCollisionQueryParams(FName(TEXT("Laser_Trace")), true, this);
  TraceParams.bTraceComplex = true;
  TraceParams.bReturnPhysicalMaterial = false;

  FHitResult HitInfo(ForceInit);

  FTransform actorTransf = GetTransform();
  FVector LidarBodyLoc = GetActorLocation();
  FRotator LidarBodyRot = GetActorRotation();
  FRotator LaserRot (VerticalAngle, HorizontalAngle, 0);  // float InPitch, float InYaw, float InRoll
  FRotator ResultRot = UKismetMathLibrary::ComposeRotators(
    LaserRot,
    LidarBodyRot
  );
  const auto Range = Description.Range;
  FVector EndTrace = Range * UKismetMathLibrary::GetForwardVector(ResultRot) + LidarBodyLoc;

  GetWorld()->LineTraceSingleByChannel(
    HitInfo,
    LidarBodyLoc,
    EndTrace,
    ECC_GameTraceChannel2,
    TraceParams,
    FCollisionResponseParams::DefaultResponseParam
  );


  if (HitInfo.bBlockingHit)
  {
    if (Description.ShowDebugPoints)
    {
      DrawDebugPoint(
        GetWorld(),
        HitInfo.ImpactPoint,
        10,  //size
        FColor(255,0,255),
        false,  //persistent (never goes away)
        0.1  //point leaves a trail on moving object
      );
    }

    const FVector hp = HitInfo.ImpactPoint;
    XYZ = actorTransf.Inverse().TransformPosition(hp);

    Intensity = ComputeIntensity(LidarBodyLoc, HitInfo);



    if(Intensity > Description.DropOffIntensityLimit)
      return true;
    else
      return RandomEngine->GetUniformFloat() < DropOffAlpha * Intensity + DropOffBeta;

  } else {
    return false;
  }
}
