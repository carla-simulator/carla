// CARLA, Copyright (C) 2017 Computer Vision Center (CVC)

#include "Carla.h"
#include "CarlaPlayerState.h"

void ACarlaPlayerState::Reset()
{
  Super::Reset();
  // Reset incremental values.
  GameTimeStamp = 0.0f;
  CollisionIntensityCars = 0.0f;
  CollisionIntensityPedestrians = 0.0f;
  CollisionIntensityOther = 0.0f;
  Images.Empty();
}

void ACarlaPlayerState::CopyProperties(APlayerState *PlayerState)
{
  Super::CopyProperties(PlayerState);
  if ((PlayerState != nullptr) && (this != PlayerState))
  {
    ACarlaPlayerState *Other = Cast<ACarlaPlayerState>(PlayerState);
    if (Other != nullptr)
    {
      PlatformTimeStamp = Other->PlatformTimeStamp;
      GameTimeStamp = Other->GameTimeStamp;
      Location = Other->Location;
      Orientation = Other->Orientation;
      Acceleration = Other->Acceleration;
      ForwardSpeed = Other->ForwardSpeed;
      CollisionIntensityCars = Other->CollisionIntensityCars;
      CollisionIntensityPedestrians = Other->CollisionIntensityPedestrians;
      CollisionIntensityOther = Other->CollisionIntensityOther;
      OtherLaneIntersectionFactor = Other->OtherLaneIntersectionFactor;
      OffRoadIntersectionFactor = Other->OffRoadIntersectionFactor;
      Images = Other->Images;
      UE_LOG(LogCarla, Log, TEXT("Copied properties of ACarlaPlayerState"));
    }
  }
}

void ACarlaPlayerState::RegisterCollision(AActor * /*Actor*/, FVector NormalImpulse)
{
  CollisionIntensityOther += NormalImpulse.Size();
}

static int32 RoundToMilliseconds(float Seconds)
{
  return FMath::RoundHalfToZero(1000.0 * Seconds);
}

void ACarlaPlayerState::UpdateTimeStamp(float DeltaSeconds)
{
  FramesPerSecond = 1.0f / DeltaSeconds;
  PlatformTimeStamp = RoundToMilliseconds(FPlatformTime::Seconds());
  GameTimeStamp += RoundToMilliseconds(DeltaSeconds);
}
