// CARLA, Copyright (C) 2017 Computer Vision Center (CVC)

#include "Carla.h"
#include "CarlaPlayerState.h"

void ACarlaPlayerState::Reset()
{
  Super::Reset();
  CollisionIntensityCars = 0.0f;
  CollisionIntensityPedestrians = 0.0f;
  CollisionIntensityOther = 0.0f;
}

void ACarlaPlayerState::CopyProperties(APlayerState *PlayerState)
{
  Super::CopyProperties(PlayerState);
  if ((PlayerState != nullptr) && (this != PlayerState))
  {
    ACarlaPlayerState *Other = Cast<ACarlaPlayerState>(PlayerState);
    if (Other != nullptr)
    {
      Location = Other->Location;
      Orientation = Other->Orientation;
      Acceleration = Other->Acceleration;
      ForwardSpeed = Other->ForwardSpeed;
      CollisionIntensityCars = Other->CollisionIntensityCars;
      CollisionIntensityPedestrians = Other->CollisionIntensityPedestrians;
      CollisionIntensityOther = Other->CollisionIntensityOther;
    }
  }
}

void ACarlaPlayerState::RegisterCollision(AActor */*Actor*/, FVector NormalImpulse)
{
  CollisionIntensityOther += NormalImpulse.Size();
}
