// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "Carla.h"
#include "Carla/Walker/WalkerController.h"
#include "Components/PrimitiveComponent.h"
#include "Components/PoseableMeshComponent.h"
#include "Containers/Map.h"

#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Pawn.h"

AWalkerController::AWalkerController(const FObjectInitializer &ObjectInitializer)
  : Super(ObjectInitializer)
{
  PrimaryActorTick.bCanEverTick = true;
}

void AWalkerController::OnPossess(APawn *InPawn)
{
  Super::OnPossess(InPawn);

  auto *Character = Cast<ACharacter>(InPawn);
  if (Character == nullptr)
  {
    UE_LOG(LogCarla, Error, TEXT("Walker is not a character!"));
    return;
  }

  auto *MovementComponent = Character->GetCharacterMovement();
  if (MovementComponent == nullptr)
  {
    UE_LOG(LogCarla, Error, TEXT("Walker missing character movement component!"));
    return;
  }

  MovementComponent->MaxWalkSpeed = GetMaximumWalkSpeed();
  MovementComponent->JumpZVelocity = 500.0f;
  Character->JumpMaxCount = 2;
}

UPoseableMeshComponent *AddNewBoneComponent(AActor *InActor, FVector inLocation, FRotator inRotator)
{
  UPoseableMeshComponent *NewComp = NewObject<UPoseableMeshComponent>(InActor,
      UPoseableMeshComponent::StaticClass());
  if (NewComp)
  {
    NewComp->RegisterComponent();
    NewComp->SetWorldLocation(inLocation);
    NewComp->SetWorldRotation(inRotator);
    NewComp->AttachToComponent(InActor->GetRootComponent(), FAttachmentTransformRules::KeepRelativeTransform);
  }
  return NewComp;
}

void AWalkerController::SetManualBones(const bool bIsEnabled)
{
  bManualBones = bIsEnabled;
  if (bManualBones)
  {
    auto *Character = GetCharacter();
    TArray<UPoseableMeshComponent *> PoseableMeshes;
    TArray<USkeletalMeshComponent *> SkeletalMeshes;
    Character->GetComponents<UPoseableMeshComponent>(PoseableMeshes, false);
    Character->GetComponents<USkeletalMeshComponent>(SkeletalMeshes, false);
    USkeletalMeshComponent *SkeletalMesh = SkeletalMeshes.IsValidIndex(0) ? SkeletalMeshes[0] : nullptr;
    if (SkeletalMesh)
    {
      UPoseableMeshComponent *PoseableMesh =
          PoseableMeshes.IsValidIndex(0) ? PoseableMeshes[0] : AddNewBoneComponent(Character,
          SkeletalMesh->GetRelativeTransform().GetLocation(),
          SkeletalMesh->GetRelativeTransform().GetRotation().Rotator());
      PoseableMesh->SetSkeletalMesh(SkeletalMesh->SkeletalMesh);
    }
  }
}

void AWalkerController::Tick(float DeltaSeconds)
{
  Super::Tick(DeltaSeconds);

  auto *Character = GetCharacter();
  switch (Control.which())
  {
    case 0: // FWalkerControl
      if (Character != nullptr)
      {
        Character->AddMovementInput(boost::get<FWalkerControl>(Control).Direction,
            boost::get<FWalkerControl>(Control).Speed / GetMaximumWalkSpeed());
        if (boost::get<FWalkerControl>(Control).Jump)
        {
          Character->Jump();
        }
      }
      break;   // End of FWalkerControl case

    case 1: // FWalkerBoneControl
      if (boost::get<FWalkerBoneControl>(Control).BoneTransforms.Num() != 0)
      {
        TArray<UPoseableMeshComponent *> PoseableMeshes;
        Character->GetComponents<UPoseableMeshComponent>(PoseableMeshes, false);
        UPoseableMeshComponent *PoseableMesh = PoseableMeshes.IsValidIndex(0) ? PoseableMeshes[0] : nullptr;
        if (PoseableMesh)
        {
          for (const TPair<FString,
              FTransform> &pair : boost::get<FWalkerBoneControl>(Control).BoneTransforms)
          {
            FName BoneName = FName(*pair.Key);
            PoseableMesh->SetBoneTransformByName(BoneName, pair.Value, EBoneSpaces::Type::ComponentSpace);
          }
          boost::get<FWalkerBoneControl>(Control).BoneTransforms.Empty();
        }
      }
      break;   // End of FWalkerBoneControl case
  }
}
