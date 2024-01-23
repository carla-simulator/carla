// Copyright (c) 2024 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "Carla/Util/ActorAttacher.h"
#include "Carla.h"
#include "GameFramework/SpringArmComponent.h"

static void UActorAttacher_AttachActorsWithSpringArm(
    AActor *Child,
    AActor *Parent)
{
  auto SpringArm = NewObject<USpringArmComponent>(Parent);

  // Child location negated to compensate for the spring arm rotation (rotated
  // from the "other end" of the arm).
  const auto ChildLocation = -Child->GetActorLocation();
  Child->SetActorLocation(FVector::ZeroVector);

  // Adding Z-offset to avoid colliding against the ground on bumpy terrain.
  SpringArm->TargetOffset = FVector(0.0f, 0.0f, 30.0f);
  SpringArm->bDoCollisionTest = true;

  FRotator LookAt = FRotationMatrix::MakeFromX(ChildLocation).Rotator();
  SpringArm->SetRelativeRotation(LookAt);
  SpringArm->SetupAttachment(Parent->GetRootComponent());

  SpringArm->TargetArmLength = ChildLocation.Size();
  SpringArm->bEnableCameraRotationLag = true;
  SpringArm->CameraRotationLagSpeed = 8.0f;

  SpringArm->bInheritPitch = false;
  SpringArm->bInheritRoll = false;
  SpringArm->bInheritYaw = true;

  SpringArm->AttachToComponent(
      Parent->GetRootComponent(),
      FAttachmentTransformRules::KeepRelativeTransform);
  SpringArm->RegisterComponent();

  auto ChildComp = NewObject<UChildActorComponent>(Parent);
  ChildComp->SetupAttachment(
      SpringArm,
      USpringArmComponent::SocketName);
  Child->AttachToComponent(
      ChildComp,
      FAttachmentTransformRules::KeepRelativeTransform);
  ChildComp->RegisterComponent();
}

static void UActorAttacher_AttachActorsWithSpringArmGhost(
    AActor *Child,
    AActor *Parent)
{
  auto SpringArm = NewObject<USpringArmComponent>(Parent);

  // Child location negated to compensate for the spring arm rotation (rotated
  // from the "other end" of the arm).
  const auto ChildLocation = -Child->GetActorLocation();
  Child->SetActorLocation(FVector::ZeroVector);

  // Adding Z-offset to avoid colliding against the ground on bumpy terrain.
  SpringArm->TargetOffset = FVector(0.0f, 0.0f, 0.0f);
  SpringArm->bDoCollisionTest = false;

  FRotator LookAt = FRotationMatrix::MakeFromX(ChildLocation).Rotator();
  SpringArm->SetRelativeRotation(LookAt);
  SpringArm->SetupAttachment(Parent->GetRootComponent());

  SpringArm->TargetArmLength = ChildLocation.Size();
  SpringArm->bEnableCameraRotationLag = true;
  SpringArm->CameraRotationLagSpeed = 8.0f;

  SpringArm->bInheritPitch = false;
  SpringArm->bInheritRoll = false;
  SpringArm->bInheritYaw = true;

  SpringArm->AttachToComponent(
      Parent->GetRootComponent(),
      FAttachmentTransformRules::KeepRelativeTransform);
  SpringArm->RegisterComponent();

  auto ChildComp = NewObject<UChildActorComponent>(Parent);
  ChildComp->SetupAttachment(
      SpringArm,
      USpringArmComponent::SocketName);
  Child->AttachToComponent(
      ChildComp,
      FAttachmentTransformRules::KeepRelativeTransform);
  ChildComp->RegisterComponent();
}

void UActorAttacher::AttachActors(
    AActor *Child,
    AActor *Parent,
    const EAttachmentType AttachmentType)
{
  check(Child != nullptr);
  check(Parent != nullptr);

  switch (AttachmentType)
  {
    case EAttachmentType::Rigid:
      Child->AttachToActor(Parent, FAttachmentTransformRules::KeepRelativeTransform);
      break;
    case EAttachmentType::SpringArm:
      UActorAttacher_AttachActorsWithSpringArm(Child, Parent);
      break;
    case EAttachmentType::SpringArmGhost:
      UActorAttacher_AttachActorsWithSpringArmGhost(Child, Parent);
      break;
    default:
      UE_LOG(LogCarla, Fatal, TEXT("Invalid attachment type"));
  }

  Child->SetOwner(Parent);
}
