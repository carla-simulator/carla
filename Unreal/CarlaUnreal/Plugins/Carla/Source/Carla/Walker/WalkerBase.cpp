// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "WalkerBase.h"
#include "Carla.h"
#include "Carla/Game/CarlaStatics.h"
#include "Carla/Game/Tagger.h"
#include "Carla/Vehicle/CarlaWheeledVehicle.h"
#include "Carla/Walker/WalkerController.h"

#include <util/ue-header-guard-begin.h>
#include "Animation/AnimSequence.h"
#include "Components/CapsuleComponent.h"
#include "Engine/SkeletalMesh.h"
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

void AWalkerBase::TagWheelchair()
{
  // Re-tag every component of the walker so a wheelchair mesh attached by
  // the blueprint after spawn gets its semantic-segmentation stencil (a
  // freshly attached component has none). Unlike ue4-dev this needs no
  // episode lookup: the UE5 tagger derives the instance id from the actor
  // itself. The wheelchair mesh lives under Carla/Static/Pedestrian, which
  // ATagger::GetLabelByPath resolves to the Pedestrian label.
  ATagger::TagActor(*this, true);
}

void AWalkerBase::AttachWheelchair()
{
  if (WheelchairComponent != nullptr)
  {
    return;
  }
  USkeletalMeshComponent *WalkerMesh = GetMesh();
  if (WalkerMesh == nullptr)
  {
    UE_LOG(LogCarla, Warning,
        TEXT("Wheelchair: walker %s has no mesh, spawning as normal pedestrian"),
        *GetName());
    return;
  }

  // Soft references throughout: a build without the wheelchair content must
  // still run, degrading to a normal pedestrian.
  auto *ChairMesh = Cast<USkeletalMesh>(FSoftObjectPath(
      TEXT("/Game/Carla/Static/Pedestrian/Prop_Wheelchair/Meshes_/SM_Wheelchair.SM_Wheelchair"))
          .TryLoad());
  auto *SittingAnim = Cast<UAnimSequence>(FSoftObjectPath(
      TEXT("/Game/Carla/Static/Pedestrian/Animations/WheelchairUser/WheelchairSitting_Moving2.WheelchairSitting_Moving2"))
          .TryLoad());
  if (ChairMesh == nullptr || SittingAnim == nullptr)
  {
    UE_LOG(LogCarla, Warning,
        TEXT("Wheelchair: content not available (mesh=%d anim=%d), walker %s stays a normal pedestrian"),
        ChairMesh != nullptr, SittingAnim != nullptr, *GetName());
    return;
  }

  WheelchairComponent = NewObject<USkeletalMeshComponent>(this, TEXT("WheelchairMesh"));
  WheelchairComponent->SetSkeletalMesh(ChairMesh);
  // Same collision convention as the walker body: query-only Pawn geometry.
  // Sensors (lidar, semantic raycasts) see the chair, but it can never
  // physically push the character or catch on the ground.
  WheelchairComponent->SetCollisionProfileName(TEXT("CharacterMesh"));
  WheelchairComponent->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
  WheelchairComponent->SetCollisionObjectType(ECC_Pawn);
  WheelchairComponent->RegisterComponent();
  WheelchairComponent->AttachToComponent(
      WalkerMesh, FAttachmentTransformRules::SnapToTargetNotIncludingScale);
  // The UE5 G2 walker meshes are authored with a different mesh-space yaw
  // than the UE4 pedestrians the chair asset was aligned to; +90 puts the
  // chair's front (casters, footplates) under the rider's feet. Calibrated
  // empirically with a 0/90/180/270 sweep and side-view captures.
  WheelchairComponent->SetRelativeRotation(FRotator(0.0f, 90.0f, 0.0f));

  // Wheelchair animation: the migrated UE4 AnimBlueprint if it loads and
  // compiled (it blends idle/rolling by speed), else a looping rolling
  // animation.
  const TCHAR *ChairAnimMode = TEXT("none");
  auto *ChairAnimClass = Cast<UClass>(FSoftObjectPath(
      TEXT("/Game/Carla/Static/Pedestrian/Animations/Wheelchair/ABP_WheelChair_MK.ABP_WheelChair_MK_C"))
          .TryLoad());
  if (ChairAnimClass != nullptr && !ChairAnimClass->HasAnyClassFlags(CLASS_NewerVersionExists))
  {
    WheelchairComponent->SetAnimInstanceClass(ChairAnimClass);
    ChairAnimMode = TEXT("ABP_WheelChair_MK");
  }
  else if (auto *RollingAnim = Cast<UAnimSequence>(FSoftObjectPath(
      TEXT("/Game/Carla/Static/Pedestrian/Animations/Wheelchair/Wheelchair_Rolling.Wheelchair_Rolling"))
          .TryLoad()))
  {
    WheelchairComponent->PlayAnimation(RollingAnim, /*bLooping=*/true);
    ChairAnimMode = TEXT("Wheelchair_Rolling loop");
  }

  // The rider: the UE4 wheelchair-user AnimBlueprints do not compile in UE5
  // (they reference GEN2 animations that were not migrated), so play the
  // sitting animation directly. The anim targets the old GEN2 skeleton
  // (SK_Pedestrian_Generan), registered as compatible with the UE5 walker
  // skeleton (Skel_Pedestrian_G2) during content migration.
  WalkerMesh->PlayAnimation(SittingAnim, /*bLooping=*/true);

  TagWheelchair();

  UE_LOG(LogCarla, Display,
      TEXT("Wheelchair attached to walker %s (chair anim: %s, rider anim: WheelchairSitting_Moving2 loop)"),
      *GetName(), ChairAnimMode);
}

void AWalkerBase::BeginPlay()
{
  Super::BeginPlay();
  // A walker is a kinematic character: every interaction it needs is
  // query-based (movement sweeps, sensor traces, the vehicle proximity
  // poll below). Rigid-body collision on any of its primitives makes it
  // an infinite-mass wall to simulating vehicles - one chassis contact
  // in the frame before the kill fires is enough to bounce or launch the
  // car, and several migrated walker blueprints kept QueryAndPhysics
  // primitives. Clamp them all to query-only; Kill() re-enables physics
  // explicitly when it ragdolls the mesh.
  {
    TArray<UPrimitiveComponent *> Primitives;
    GetComponents<UPrimitiveComponent>(Primitives, false);
    for (UPrimitiveComponent *Primitive : Primitives)
    {
      const ECollisionEnabled::Type Enabled = Primitive->GetCollisionEnabled();
      if (Enabled == ECollisionEnabled::QueryAndPhysics)
      {
        Primitive->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
      }
      else if (Enabled == ECollisionEnabled::PhysicsOnly)
      {
        Primitive->SetCollisionEnabled(ECollisionEnabled::NoCollision);
      }
      // Several migrated walker meshes carry a WorldDynamic object type
      // instead of Pawn. Vehicle suspension traces skip Pawn objects but
      // must ride dynamic world geometry, so a WorldDynamic body part is
      // an invisible ramp under a wheel (observed: the sphere-cast wheel
      // climbing CharacterMesh0 and yawing the car through a hit). Every
      // walker primitive is a body part: normalize to Pawn.
      if (Primitive->GetCollisionObjectType() == ECC_WorldDynamic)
      {
        Primitive->SetCollisionObjectType(ECC_Pawn);
      }
    }
  }
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
  const float ContactRadius = Capsule->GetScaledCapsuleRadius() + VehicleSensorMargin;
  const float HalfHeight = Capsule->GetScaledCapsuleHalfHeight();

  // The effective poll interval can't be shorter than a frame (timers
  // fire at most once per tick), and a fast vehicle crosses the whole
  // contact shell between polls: at 107 km/h and 25 fps that is ~120 cm
  // per frame against an 85 cm shell, so a pure proximity test tunnels
  // straight through (observed: no kill, and the suspension raycasts
  // launched the car off the corpse-to-be). The query is therefore sized
  // for the fastest supported closing speed, and each found vehicle is
  // tested against its predicted path over the next interval instead of
  // its current position only.
  const float Dt = FMath::Max(VehicleSensorPeriod, GetWorld()->GetDeltaSeconds());
  const float QueryRadius =
      ContactRadius + VehicleBodyAllowance + VehicleSensorSpeedBound * Dt * 1.25f;

  TArray<FOverlapResult> Overlaps;
  FCollisionObjectQueryParams ObjectParams;
  ObjectParams.AddObjectTypesToQuery(ECC_Vehicle);
  FCollisionQueryParams Params(SCENE_QUERY_STAT(WalkerVehicleSensor), false, this);
  if (!GetWorld()->OverlapMultiByObjectType(
      Overlaps,
      GetActorLocation(),
      FQuat::Identity,
      ObjectParams,
      FCollisionShape::MakeCapsule(QueryRadius, HalfHeight),
      Params))
  {
    return;
  }

  const FVector WalkerLoc = GetActorLocation();
  for (const FOverlapResult &Overlap : Overlaps)
  {
    ACarlaWheeledVehicle *Vehicle = Cast<ACarlaWheeledVehicle>(Overlap.GetActor());
    UPrimitiveComponent *VehicleBody = Overlap.GetComponent();
    if (Vehicle == nullptr || VehicleBody == nullptr)
    {
      continue;
    }
    const FVector Velocity = Vehicle->GetVelocity();
    const float Speed = Velocity.Size();
    if (Speed < VehicleKillSpeed)
    {
      continue;
    }
    // Direct contact: distance to the vehicle's actual collision
    // geometry, same semantics the low-speed validation locked in.
    FVector OnBody;
    const float DistToBody =
        VehicleBody->GetDistanceToCollision(WalkerLoc, OnBody);
    bool bKill = DistToBody >= 0.0f && DistToBody <= ContactRadius;
    if (!bKill)
    {
      // Tunneling guard: closest approach of the vehicle center's
      // predicted path over the next poll interval. Direction matters -
      // a fast car in the next lane passes wide and must not kill.
      const FVector PathEnd = FVector(Vehicle->GetActorLocation()) + Velocity * Dt * 1.25f;
      const FVector Closest = FMath::ClosestPointOnSegment(
          WalkerLoc, Vehicle->GetActorLocation(), PathEnd);
      bKill = FVector::Dist2D(Closest, WalkerLoc) <=
                  ContactRadius + VehicleBodyAllowance &&
              FMath::Abs(Closest.Z - WalkerLoc.Z) <= HalfHeight + 200.0f;
    }
    if (bKill)
    {
      Kill(Velocity, Vehicle);
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
