// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include <util/ue-header-guard-begin.h>
#include "GameFramework/Character.h"
#include <util/ue-header-guard-end.h>

#include "WalkerBase.generated.h"

class ACarlaWheeledVehicle;

UCLASS()
class CARLA_API AWalkerBase : public ACharacter
{

  GENERATED_BODY()

public:

  AWalkerBase(const FObjectInitializer &ObjectInitializer);

  UPROPERTY(Category="Walker Base", BlueprintReadWrite, EditAnywhere)
  bool bAlive = true;

  UPROPERTY(Category="Walker Base", BlueprintReadWrite, EditAnywhere)
  float LifeSpanAfterDeath = 10.0f;

  /// A vehicle touching the walker with at least this relative speed
  /// (cm/s) kills it. Below the threshold the contact is treated as a
  /// harmless brush (parking-speed nudges must not kill).
  UPROPERTY(Category="Walker Base", BlueprintReadWrite, EditAnywhere)
  float VehicleKillSpeed = 210.0f;

  /// How far beyond the capsule radius the vehicle proximity check
  /// reaches (cm). The margin makes the kill trigger before the first
  /// physical contact: the capsule is a kinematic body, so to a
  /// simulating vehicle it is an infinite-mass wall - the impulse of a
  /// real contact is what used to launch and spin cars. The
  /// capsule is query-only, so vehicles never physically contact it -
  /// what launched cars was the wheel suspension raycasts riding the
  /// blocking capsule like a rigid bollard. The kill therefore fires
  /// from this proximity poll just before the wheels reach the capsule,
  /// and the collision event is delivered to the vehicle's sensors
  /// explicitly from Kill().
  UPROPERTY(Category="Walker Base", BlueprintReadWrite, EditAnywhere)
  float VehicleSensorMargin = 50.0f;

  /// Vehicle proximity poll period in seconds. A timer rather than
  /// Tick(): the walker blueprints keep actor tick disabled for
  /// performance, and the timer must not depend on that.
  UPROPERTY(Category="Walker Base", BlueprintReadWrite, EditAnywhere)
  float VehicleSensorPeriod = 0.02f;

  UFUNCTION(BlueprintCallable)
  void StartDeathLifeSpan()
  {
    SetLifeSpan(LifeSpanAfterDeath);
  }

  /// Kill the walker as if run over: stop navigation and movement, stop
  /// blocking vehicles, ragdoll (or play the directional death animation
  /// when the episode requests deterministic ragdolls) and start the
  /// after-death life span. Idempotent.
  UFUNCTION(BlueprintCallable)
  void Kill(FVector ImpactVelocity, AActor *Killer = nullptr);

  virtual void BeginPlay() override;

private:

  UFUNCTION()
  void OnBodyHit(AActor *SelfActor, AActor *OtherActor,
      FVector NormalImpulse, const FHitResult &Hit);

  void CheckVehicleImpact();

  void TryKillFromVehicle(AActor *OtherActor);

  FTimerHandle VehicleSensorTimer;
};
