// CARLA, Copyright (C) 2017 Computer Vision Center (CVC)

#pragma once

#include "CarlaVehicleController.h"
#include "CarlaServerController.generated.h"

/**
 *
 */
UCLASS()
class CARLA_API ACarlaServerController : public ACarlaVehicleController
{
  GENERATED_BODY()

public:

  virtual void Tick(float DeltaTime) override;

  virtual void OnCollisionEvent(
      AActor* Actor,
      AActor* OtherActor,
      FVector NormalImpulse,
      const FHitResult& Hit) override;
};
