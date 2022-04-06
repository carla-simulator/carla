// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "Carla.h"
#include "Carla/Walker/WalkerController.h"
#include "Carla/Walker/WalkerAnim.h"

#include "Components/PoseableMeshComponent.h"
#include "Components/PrimitiveComponent.h"
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

void AWalkerController::ApplyWalkerControl(const FWalkerControl &InControl)
{
  Control = InControl;
}

void AWalkerController::GetBonesTransform(FWalkerBoneControlOut &WalkerBones)
{
  auto *Character = GetCharacter();
  if (!Character) return;

  TArray<USkeletalMeshComponent *> SkeletalMeshes;
  Character->GetComponents<USkeletalMeshComponent>(SkeletalMeshes, false);
  USkeletalMeshComponent *SkeletalMesh = SkeletalMeshes.IsValidIndex(0) ? SkeletalMeshes[0] : nullptr;
  if (!SkeletalMesh) return;

  // get the walker animation class
  auto *AnimInst = SkeletalMesh->GetAnimInstance();
  if (!AnimInst) return;
  UWalkerAnim *WalkerAnim = Cast<UWalkerAnim>(AnimInst);
  if (!WalkerAnim) return;
  
  // get current pose
  FPoseSnapshot TempSnapshot;
  SkeletalMesh->SnapshotPose(TempSnapshot);
  
  // copy pose
  // WalkerAnim->Snap = TempSnapshot;

  for (int i=0; i<TempSnapshot.BoneNames.Num(); ++i)
  {
    FWalkerBoneControlOutData Transforms;
    Transforms.World = SkeletalMesh->GetSocketTransform(TempSnapshot.BoneNames[i], ERelativeTransformSpace::RTS_World);
    Transforms.Component = SkeletalMesh->GetSocketTransform(TempSnapshot.BoneNames[i], ERelativeTransformSpace::RTS_Actor);
    Transforms.Relative = SkeletalMesh->GetSocketTransform(TempSnapshot.BoneNames[i], ERelativeTransformSpace::RTS_ParentBoneSpace);
    // Transforms.Relative = TempSnapshot.LocalTransforms[i];

    WalkerBones.BoneTransforms.Add(TempSnapshot.BoneNames[i].ToString(), Transforms);
  }
}

void AWalkerController::SetBonesTransform(const FWalkerBoneControlIn &WalkerBones)
{
  auto *Character = GetCharacter();
  if (!Character) return;

  TArray<USkeletalMeshComponent *> SkeletalMeshes;
  Character->GetComponents<USkeletalMeshComponent>(SkeletalMeshes, false);
  USkeletalMeshComponent *SkeletalMesh = SkeletalMeshes.IsValidIndex(0) ? SkeletalMeshes[0] : nullptr;
  if (!SkeletalMesh) return;

  // get the walker animation class
  auto *AnimInst = SkeletalMesh->GetAnimInstance();
  if (!AnimInst) return;
  UWalkerAnim *WalkerAnim = Cast<UWalkerAnim>(AnimInst);
  if (!WalkerAnim) return;
  
  // if pose is empty, then get a first version
  if (WalkerAnim->Snap.BoneNames.Num() == 0)
  {
    // get current pose
    SkeletalMesh->SnapshotPose(WalkerAnim->Snap);
  }

  TMap<FName, FTransform> BonesMap;
  for (const TPair<FString, FTransform> &pair : WalkerBones.BoneTransforms)
  {
    FName BoneName = FName(*pair.Key);
    BonesMap.Add(BoneName, pair.Value);
  }

  // assign common bones
  for (int i=0; i<WalkerAnim->Snap.BoneNames.Num(); ++i)
  {
    FTransform *Trans = BonesMap.Find(WalkerAnim->Snap.BoneNames[i]);
    if (Trans)
    {
      WalkerAnim->Snap.LocalTransforms[i] = *Trans;
    }
  }
}

void AWalkerController::BlendPose(float Blend)
{
  auto *Character = GetCharacter();
  if (!Character) return;

  TArray<USkeletalMeshComponent *> SkeletalMeshes;
  Character->GetComponents<USkeletalMeshComponent>(SkeletalMeshes, false);
  USkeletalMeshComponent *SkeletalMesh = SkeletalMeshes.IsValidIndex(0) ? SkeletalMeshes[0] : nullptr;
  if (!SkeletalMesh) return;

  // get the walker animation class
  auto *AnimInst = SkeletalMesh->GetAnimInstance();
  if (!AnimInst) return;
  UWalkerAnim *WalkerAnim = Cast<UWalkerAnim>(AnimInst);
  if (!WalkerAnim) return;
  
  // set current pose
  WalkerAnim->Blend = Blend;
}

void AWalkerController::GetPoseFromAnimation()
{
  auto *Character = GetCharacter();
  if (!Character) return;

  TArray<USkeletalMeshComponent *> SkeletalMeshes;
  Character->GetComponents<USkeletalMeshComponent>(SkeletalMeshes, false);
  USkeletalMeshComponent *SkeletalMesh = SkeletalMeshes.IsValidIndex(0) ? SkeletalMeshes[0] : nullptr;
  if (!SkeletalMesh) return;

  // get the walker animation class
  auto *AnimInst = SkeletalMesh->GetAnimInstance();
  if (!AnimInst) return;
  UWalkerAnim *WalkerAnim = Cast<UWalkerAnim>(AnimInst);
  if (!WalkerAnim) return;
  
  // get current pose
  SkeletalMesh->SnapshotPose(WalkerAnim->Snap);
}

void AWalkerController::Tick(float DeltaSeconds)
{
  TRACE_CPUPROFILER_EVENT_SCOPE(AWalkerController::Tick);
  Super::Tick(DeltaSeconds);

  auto *Character = GetCharacter();
  if (!Character) return;

  Character->AddMovementInput(Control.Direction,
        Control.Speed / GetMaximumWalkSpeed());
  if (Control.Jump)
  {
    Character->Jump();
  }
}
