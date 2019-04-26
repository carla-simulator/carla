// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "Carla/Actor/ActorDefinition.h"
#include "Carla/Sensor/Sensor.h"

#include "CollisionSensor.generated.h"

class UCarlaEpisode;
class UCarlaGameInstance;

/// A sensor to register collisions.
UCLASS()
class CARLA_API ACollisionSensor : public ASensor
{
  GENERATED_BODY()

public:

  static FActorDefinition GetSensorDefinition();

  ACollisionSensor(const FObjectInitializer& ObjectInitializer);

  void SetOwner(AActor *NewOwner) override;

private:

  UFUNCTION()
  void OnCollisionEvent(
      AActor *Actor,
      AActor *OtherActor,
      FVector NormalImpulse,
      const FHitResult &Hit);
};
