// Copyright (c) 2024 Computer Vision Center (CVC) at the Universitat Autonoma
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

  virtual void PrePhysTick(float DeltaSeconds) override;
  void SetOwner(AActor *NewOwner) override;

  UFUNCTION()
  void OnCollisionEvent(
      AActor *Actor,
      AActor *OtherActor,
      FVector NormalImpulse,
      const FHitResult &Hit);

  UFUNCTION(BlueprintCallable, Category="Collision")
  void OnActorCollisionEvent(
      AActor *Actor,
      AActor *OtherActor,
      FVector NormalImpulse,
      const FHitResult &Hit);

  UFUNCTION()
  void OnComponentCollisionEvent(
      UPrimitiveComponent* HitComp,
      AActor* OtherActor,
      UPrimitiveComponent* OtherComp,
      FVector NormalImpulse,
      const FHitResult& Hit);

private:
  /// Registry that saves all collisions. Used to avoid sending the same collision more than once per frame,
  /// as the collision sensor uses the PhysX substepping tick. Helps with sensor usage and stream overload.
  std::vector<std::tuple<uint64_t, AActor*, AActor*>> CollisionRegistry;
};
