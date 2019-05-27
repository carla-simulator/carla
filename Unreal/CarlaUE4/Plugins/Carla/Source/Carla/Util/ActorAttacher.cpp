// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "Carla.h"
#include "Carla/Util/ActorAttacher.h"

#include "GameFramework/SpringArmComponent.h"

static void UActorAttacher_AttachActorsWithSpringArm(
    AActor *Child,
    AActor *Parent)
{
  auto SpringArm = NewObject<USpringArmComponent>(Parent);

  const auto ChildLocation = Child->GetActorLocation();
  Child->SetActorLocation(FVector::ZeroVector);

  FRotator LookAt = FRotationMatrix::MakeFromX(-ChildLocation).Rotator();
  SpringArm->TargetOffset = FVector(0.0f, 0.0f, 30.0f);
  SpringArm->SetRelativeRotation(LookAt);
  SpringArm->SetupAttachment(Parent->GetRootComponent());
  SpringArm->TargetArmLength = ChildLocation.Size();
  SpringArm->bEnableCameraRotationLag = true;
  SpringArm->CameraRotationLagSpeed = 8.0f;
  SpringArm->bInheritPitch = false;
  SpringArm->bInheritRoll = false;
  SpringArm->bInheritYaw = true;
  SpringArm->bDoCollisionTest = true;

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
    default:
      UE_LOG(LogCarla, Fatal, TEXT("Invalid attachment type"));
  }

  Child->SetOwner(Parent);
}
