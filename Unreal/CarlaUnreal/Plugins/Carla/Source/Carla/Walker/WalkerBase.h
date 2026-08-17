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

  /// A vehicle touching the walker while moving at least this fast
  /// (cm/s) kills it - ue4-dev parity, where any moving-vehicle contact
  /// was lethal. The threshold only exempts effectively stationary
  /// vehicles (a walker bumping into a parked or stopped car survives).
  /// It must stay low: an alive walker's capsule is query-only, so a
  /// sub-threshold *moving* contact has no physical response at all and
  /// the car visibly interpenetrates the standing pedestrian.
  UPROPERTY(Category="Walker Base", BlueprintReadWrite, EditAnywhere)
  float VehicleKillSpeed = 50.0f;

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

  /// Lateral clearance (cm) added on top of the capsule radius and
  /// sensor margin when testing a fast vehicle's predicted path. Stands
  /// in for the vehicle body's half width, since the prediction sweeps
  /// the actor center, not the collision geometry.
  UPROPERTY(Category="Walker Base", BlueprintReadWrite, EditAnywhere)
  float VehicleBodyAllowance = 90.0f;

  /// Fastest closing speed (cm/s) the proximity query is sized for
  /// (default 300 km/h). Only affects the overlap query radius; the
  /// per-vehicle path prediction uses the vehicle's actual velocity.
  UPROPERTY(Category="Walker Base", BlueprintReadWrite, EditAnywhere)
  float VehicleSensorSpeedBound = 8333.0f;

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
