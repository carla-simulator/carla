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

  /// @todo ?
  // virtual void Reset();
  // virtual void CopyProperties(APlayerState *PlayerState);

  void RegisterCollision(AActor *Actor, FVector NormalImpulse) {}

  FVector Location;

  FVector Orientation;

  float ForwardSpeed;
};
