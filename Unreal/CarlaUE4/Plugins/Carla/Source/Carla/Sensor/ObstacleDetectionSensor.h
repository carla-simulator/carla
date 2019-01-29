// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "Carla/Sensor/Sensor.h"

#include "ObstacleDetectionSensor.generated.h"

class UCarlaEpisode;
class UCarlaGameInstance;

/// A sensor to register collisions.
UCLASS()
class CARLA_API AObstacleDetectionSensor : public ASensor
{
  GENERATED_BODY()

  float Distance;

  float HitRadius;

  float HeightVar;

  bool bOnlyDynamics = false;

  bool bDebugLineTrace = false;

public:

  static FActorDefinition GetSensorDefinition();

  AObstacleDetectionSensor(const FObjectInitializer &ObjectInitializer);

  void SetOwner(AActor *NewOwner) override;

  void Set(const FActorDescription &Description) override;

  void BeginPlay() override;

  void Tick(float DeltaSeconds) override;

private:

  UFUNCTION()
  void OnObstacleDetectionEvent(
      AActor *Actor,
      AActor *OtherActor,
      float Distance,
      const FHitResult &Hit);

  UPROPERTY()
  const UCarlaEpisode *Episode = nullptr;

  UPROPERTY()
  const UCarlaGameInstance *GameInstance = nullptr;

};
