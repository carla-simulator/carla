// CARLA, Copyright (C) 2017 Computer Vision Center (CVC)

#pragma once

#include "GameFramework/PlayerState.h"
#include "CarlaPlayerState.generated.h"

/**
 *
 */
UCLASS()
class CARLA_API ACarlaPlayerState : public APlayerState
{
  GENERATED_BODY()

public:

  virtual void Reset() override;

  virtual void CopyProperties(APlayerState *PlayerState) override;

  void RegisterCollision(AActor *Actor, FVector NormalImpulse);

  const FVector &GetLocation() const
  {
    return Location;
  }

  const FVector &GetOrientation() const
  {
    return Orientation;
  }

  const FVector &GetAcceleration() const
  {
    return Acceleration;
  }

  float GetForwardSpeed() const
  {
    return ForwardSpeed;
  }

  float GetCollisionIntensityCars() const
  {
    return CollisionIntensityCars;
  }

  float GetCollisionIntensityPedestrians() const
  {
    return CollisionIntensityPedestrians;
  }

  float GetCollisionIntensityOther() const
  {
    return CollisionIntensityOther;
  }

private:

  friend class ACarlaVehicleController;

  FVector Location;

  FVector Orientation;

  FVector Acceleration;

  float ForwardSpeed;

  float CollisionIntensityCars = 0.0f;

  float CollisionIntensityPedestrians = 0.0f;

  float CollisionIntensityOther = 0.0f;
};
