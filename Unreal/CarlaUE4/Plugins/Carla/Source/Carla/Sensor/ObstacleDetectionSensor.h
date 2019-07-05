// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "Carla/Sensor/Sensor.h"
#include "Carla/Actor/ActorDefinition.h"
#include "Carla/Actor/ActorDescription.h"
#include "ObstacleDetectionSensor.generated.h"

class UCarlaEpisode;

/// A sensor to register collisions.
UCLASS()
class CARLA_API AObstacleDetectionSensor : public ASensor
{
  GENERATED_BODY()

public:

  static FActorDefinition GetSensorDefinition();

  AObstacleDetectionSensor(const FObjectInitializer &ObjectInitializer);

  void Set(const FActorDescription &Description) override;

  void Tick(float DeltaSeconds) override;

private:

  UFUNCTION()
  void OnObstacleDetectionEvent(
      AActor *Actor,
      AActor *OtherActor,
      float Distance,
      const FHitResult &Hit);

private:

  float Distance;

  float HitRadius;

  bool bOnlyDynamics = false;

  bool bDebugLineTrace = false;
};
