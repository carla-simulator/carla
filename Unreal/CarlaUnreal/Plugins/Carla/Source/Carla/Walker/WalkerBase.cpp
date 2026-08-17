// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "WalkerBase.h"
#include "Carla.h"
#include "Carla/Game/CarlaStatics.h"
#include "Carla/Vehicle/CarlaWheeledVehicle.h"
#include "Carla/Walker/WalkerController.h"

#include <util/ue-header-guard-begin.h>
#include "Animation/AnimSequence.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "PhysicsEngine/PhysicsAsset.h"
#include "Engine/OverlapResult.h"
#include "Engine/World.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "TimerManager.h"
#include <util/ue-header-guard-end.h>

AWalkerBase::AWalkerBase(const FObjectInitializer &ObjectInitializer)
  : Super(ObjectInitializer)
{
  PrimaryActorTick.bCanEverTick = true;
}

void AWalkerBase::BeginPlay()
{
  Super::BeginPlay();
  // Primary detection: the physical hit itself. The same contact also
  // feeds the collision sensor, so clients keep receiving the collision
  // event exactly like they always have; the capsule is un-walled right
  // after that first contact frame, before the solver can launch the
  // vehicle.
  OnActorHit.AddDynamic(this, &AWalkerBase::OnBodyHit);
  // Fallback: a cheap proximity poll on a timer (the walker blueprints
  // disable actor tick for performance, so Tick() is not an option).
  GetWorldTimerManager().SetTimer(
      VehicleSensorTimer, this, &AWalkerBase::CheckVehicleImpact,
      VehicleSensorPeriod, /*bLoop=*/true);
}

void AWalkerBase::OnBodyHit(AActor *, AActor *OtherActor,
    FVector, const FHitResult &)
{
  if (bAlive)
  {
    TryKillFromVehicle(OtherActor);
  }
}

void AWalkerBase::TryKillFromVehicle(AActor *OtherActor)
{
  ACarlaWheeledVehicle *Vehicle = Cast<ACarlaWheeledVehicle>(OtherActor);
  if (Vehicle == nullptr)
  {
    return;
  }
  // Threshold on the vehicle's own speed: a relative-speed test gets
  // polluted by the contact itself (a nudged walker picks up opposite
  // velocity, inflating the relative measure past the threshold during a
  // parking-speed push). A walker walking into a stationary vehicle
  // stays alive either way.
  const FVector ImpactVelocity = Vehicle->GetVelocity();
  if (ImpactVelocity.Size() >= VehicleKillSpeed)
  {
    Kill(ImpactVelocity, Vehicle);
  }
}

void AWalkerBase::CheckVehicleImpact()
{
  if (!bAlive)
  {
    GetWorldTimerManager().ClearTimer(VehicleSensorTimer);
    return;
  }
  // Proximity query instead of component overlap events: it does not
  // depend on any other component's GenerateOverlapEvents flag (the
  // blueprint-era death triggers lost their event bindings in the UE5
  // migration), and the margin fires it before the first physical
  // contact between the vehicle and the kinematic capsule.
  const UCapsuleComponent *Capsule = GetCapsuleComponent();
  if (Capsule == nullptr)
  {
    return;
  }
  const float Radius = Capsule->GetScaledCapsuleRadius() + VehicleSensorMargin;
  const float HalfHeight = Capsule->GetScaledCapsuleHalfHeight();

  TArray<FOverlapResult> Overlaps;
  FCollisionObjectQueryParams ObjectParams;
  ObjectParams.AddObjectTypesToQuery(ECC_Vehicle);
  FCollisionQueryParams Params(SCENE_QUERY_STAT(WalkerVehicleSensor), false, this);
  if (!GetWorld()->OverlapMultiByObjectType(
      Overlaps,
      GetActorLocation(),
      FQuat::Identity,
      ObjectParams,
      FCollisionShape::MakeCapsule(Radius, HalfHeight),
      Params))
  {
    return;
  }

  for (const FOverlapResult &Overlap : Overlaps)
  {
    TryKillFromVehicle(Overlap.GetActor());
    if (!bAlive)
    {
      return;
    }
  }
}

void AWalkerBase::Kill(FVector ImpactVelocity, AActor *Killer)
{
  if (!bAlive)
  {
    return;
  }
  bAlive = false;

  // Stop the brain: abort navigation and drop any pending manual input.
  // Further client controls are rejected upstream (FWalkerActor returns
  // WalkerDead once bAlive is false).
  if (auto *WalkerController = Cast<AWalkerController>(GetController()))
  {
    WalkerController->StopNavigation();
    WalkerController->ApplyWalkerControl(FWalkerControl());
  }
  if (auto *Movement = GetCharacterMovement())
  {
    Movement->StopMovementImmediately();
    Movement->DisableMovement();
  }

  // The capsule is query-only, but the vehicle wheel suspension traces
  // are queries too - a blocking response is exactly what made cars ride
  // up the capsule and spin. Dead walkers must be invisible to vehicle
  // physics while staying visible to everything else (lidar, camera
  // ground truth, the crowd).
  if (auto *Capsule = GetCapsuleComponent())
  {
    Capsule->SetCollisionResponseToChannel(ECC_Vehicle, ECR_Ignore);
  }

  // The kill replaces the physical contact, so the contact's client
  // contract must be honored here: collision sensors on the vehicle
  // (scenario_runner infraction scoring) expect a collision event with
  // the walker as the other actor.
  if (Killer != nullptr)
  {
    FHitResult FakeHit(ForceInit);
    FakeHit.HitObjectHandle = FActorInstanceHandle(this);
    FakeHit.ImpactPoint = GetActorLocation();
    FakeHit.ImpactNormal = (-ImpactVelocity).GetSafeNormal();
    Killer->OnActorHit.Broadcast(Killer, this,
        ImpactVelocity * 100.0f /*pseudo impulse, ~walker mass*/, FakeHit);
  }

  USkeletalMeshComponent *BodyMesh = GetMesh();
  bool bDeterministic = true;
  if (const UCarlaEpisode *Episode = UCarlaStatics::GetCurrentEpisode(GetWorld()))
  {
    bDeterministic = Episode->GetSettings().bDeterministicRagdolls;
  }

  if (BodyMesh != nullptr && !bDeterministic)
  {
    // Physics ragdoll carried by the impact. The vehicle-channel ignore
    // must land between the profile switch (which resets per-channel
    // overrides) and SetSimulatePhysics (which instantiates the per-bone
    // bodies from the component's current collision state) - otherwise
    // the wheels ride the simulating bodies and launch the car.
    BodyMesh->SetCollisionProfileName(TEXT("Ragdoll"));
    BodyMesh->SetCollisionResponseToChannel(ECC_Vehicle, ECR_Ignore);
    BodyMesh->SetSimulatePhysics(true);
    BodyMesh->SetAllPhysicsLinearVelocity(ImpactVelocity, false);
  }
  else if (BodyMesh != nullptr)
  {
    // Deterministic death: directional animation (the recorder can
    // replay it bit-for-bit, unlike a physics ragdoll). Pick the
    // animation from the impact direction in the walker's frame.
    const FVector LocalDir =
        GetActorTransform().InverseTransformVectorNoScale(
            ImpactVelocity.GetSafeNormal2D());
    const TCHAR *AnimName;
    if (FMath::Abs(LocalDir.X) >= FMath::Abs(LocalDir.Y))
    {
      // Pushed forward -> falls on its front; pushed backward -> back.
      AnimName = LocalDir.X >= 0.0f
          ? TEXT("/Game/Carla/Animations/Death_Anim/Anim_Death_F_01.Anim_Death_F_01")
          : TEXT("/Game/Carla/Animations/Death_Anim/Anim_Death_B_01.Anim_Death_B_01");
    }
    else
    {
      AnimName = LocalDir.Y >= 0.0f
          ? TEXT("/Game/Carla/Animations/Death_Anim/Anim_Death_R_01.Anim_Death_R_01")
          : TEXT("/Game/Carla/Animations/Death_Anim/Anim_Death_L_01.Anim_Death_L_01");
    }
    if (auto *DeathAnim = Cast<UAnimSequence>(
        StaticLoadObject(UAnimSequence::StaticClass(), nullptr, AnimName)))
    {
      BodyMesh->PlayAnimation(DeathAnim, /*bLooping=*/false);
    }
    else
    {
      UE_LOG(LogCarla, Warning,
          TEXT("Walker %s: death animation '%s' not found, using ragdoll"),
          *GetName(), AnimName);
      BodyMesh->SetCollisionProfileName(TEXT("Ragdoll"));
      BodyMesh->SetCollisionResponseToChannel(ECC_Vehicle, ECR_Ignore);
      BodyMesh->SetSimulatePhysics(true);
      BodyMesh->SetAllPhysicsLinearVelocity(ImpactVelocity, false);
    }
  }

  // The corpse mesh must be invisible to vehicle physics in the
  // animation path too (wheel suspension raycasts are queries and the
  // mesh response to the vehicle channel is Block by default).
  if (BodyMesh != nullptr && !BodyMesh->IsSimulatingPhysics())
  {
    BodyMesh->SetCollisionResponseToChannel(ECC_Vehicle, ECR_Ignore);
  }

  UE_LOG(LogCarla, Log, TEXT("Walker %s killed by vehicle impact (%.0f cm/s)"),
      *GetName(), ImpactVelocity.Size());
  StartDeathLifeSpan();
}
