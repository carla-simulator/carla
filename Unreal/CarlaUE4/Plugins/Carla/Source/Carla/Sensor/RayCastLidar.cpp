// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "Carla.h"
#include "Carla/Sensor/RayCastLidar.h"
#include "Carla/Actor/ActorBlueprintFunctionLibrary.h"

#include <compiler/disable-ue4-macros.h>
#include "carla/geom/Math.h"
#include <compiler/enable-ue4-macros.h>

#include "DrawDebugHelpers.h"
#include "Engine/CollisionProfile.h"
#include "Runtime/Engine/Classes/Kismet/KismetMathLibrary.h"

FActorDefinition ARayCastLidar::GetSensorDefinition()
{
  return UActorBlueprintFunctionLibrary::MakeLidarDefinition(TEXT("ray_cast"));
}

ARayCastLidar::ARayCastLidar(const FObjectInitializer& ObjectInitializer)
  : Super(ObjectInitializer)
{
  PrimaryActorTick.bCanEverTick = true;
}

void ARayCastLidar::Set(const FActorDescription &ActorDescription)
{
  Super::Set(ActorDescription);
  FLidarDescription LidarDescription;
  UActorBlueprintFunctionLibrary::SetLidar(ActorDescription, LidarDescription);
  Set(LidarDescription);
}

void ARayCastLidar::Set(const FLidarDescription &LidarDescription)
{
  Description = LidarDescription;
  LidarMeasurement = FLidarMeasurement(Description.Channels);
  CreateLasers();
}

void ARayCastLidar::CreateLasers()
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

void ARayCastLidar::Tick(const float DeltaTime)
{
  Super::Tick(DeltaTime);

  ReadPoints(DeltaTime);

  auto DataStream = GetDataStream(*this);
  DataStream.Send(*this, LidarMeasurement, DataStream.PopBufferFromPool());
}

void ARayCastLidar::ReadPoints(const float DeltaTime)
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

  LidarMeasurement.Reset(ChannelCount * PointsToScanWithOneLaser);

  for (auto Channel = 0u; Channel < ChannelCount; ++Channel)
  {
    for (auto i = 0u; i < PointsToScanWithOneLaser; ++i)
    {
      FVector Point;
      const float Angle = CurrentHorizontalAngle + AngleDistanceOfLaserMeasure * i;
      if (ShootLaser(Channel, Angle, Point))
      {
        LidarMeasurement.WritePoint(Channel, Point);
      }
    }
  }

  const float HorizontalAngle = carla::geom::Math::ToRadians(
      std::fmod(CurrentHorizontalAngle + AngleDistanceOfTick, 360.0f));
  LidarMeasurement.SetHorizontalAngle(HorizontalAngle);
}

bool ARayCastLidar::ShootLaser(const uint32 Channel, const float HorizontalAngle, FVector &XYZ) const
{
  const float VerticalAngle = LaserAngles[Channel];

  FCollisionQueryParams TraceParams = FCollisionQueryParams(FName(TEXT("Laser_Trace")), true, this);
  TraceParams.bTraceComplex = true;
  TraceParams.bReturnPhysicalMaterial = false;

  FHitResult HitInfo(ForceInit);

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
    ECC_MAX,
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

    XYZ = LidarBodyLoc - HitInfo.ImpactPoint;
    XYZ = UKismetMathLibrary::RotateAngleAxis(
      XYZ,
      - LidarBodyRot.Yaw + 90,
      FVector(0, 0, 1)
    );

    return true;
  } else {
    return false;
  }
}
