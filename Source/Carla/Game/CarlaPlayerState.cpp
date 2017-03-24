// CARLA, Copyright (C) 2017 Computer Vision Center (CVC)

#include "Carla.h"
#include "CarlaPlayerState.h"

void ACarlaPlayerState::Reset()
{
  Super::Reset();
  // Reset incremental values.
  CollisionIntensityCars = 0.0f;
  CollisionIntensityPedestrians = 0.0f;
  CollisionIntensityOther = 0.0f;
  // Invalidate images.
  for (auto &image : Images) {
    image = Image(); // Reset.
  }
}

void ACarlaPlayerState::CopyProperties(APlayerState *PlayerState)
{
  Super::CopyProperties(PlayerState);
  if ((PlayerState != nullptr) && (this != PlayerState))
  {
    ACarlaPlayerState *Other = Cast<ACarlaPlayerState>(PlayerState);
    if (Other != nullptr)
    {
      TimeStamp = Other->TimeStamp;
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

void ACarlaPlayerState::RegisterCollision(AActor */*Actor*/, FVector NormalImpulse)
{
  CollisionIntensityOther += NormalImpulse.Size();
}

void ACarlaPlayerState::UpdateTimeStamp()
{
  TimeStamp = FMath::RoundHalfToZero(1000.0 * FPlatformTime::Seconds());
}
