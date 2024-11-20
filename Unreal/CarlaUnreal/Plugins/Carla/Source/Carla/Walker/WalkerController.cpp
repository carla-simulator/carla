// Copyright (c) 2024 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "Carla/Walker/WalkerController.h"
#include "Carla.h"
#include "Carla/Walker/WalkerAnim.h"

#include <util/ue-header-guard-begin.h>
#include "Components/PoseableMeshComponent.h"
#include "Components/PrimitiveComponent.h"
#include "Containers/Map.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Pawn.h"
#include <util/ue-header-guard-end.h>

AWalkerController::AWalkerController(const FObjectInitializer &ObjectInitializer)
  : Super(ObjectInitializer)
{
  PrimaryActorTick.bCanEverTick = true;
}

void AWalkerController::OnPossess(APawn *InPawn)
{
  Super::OnPossess(InPawn);

  ACharacter *CurrentCharacter = Cast<ACharacter>(InPawn);
  if (CurrentCharacter == nullptr)
  {
    UE_LOG(LogCarla, Error, TEXT("Walker is not a character!"));
    return;
  }

  UMovementComponent *MovementComponent = CurrentCharacter->GetCharacterMovement();
  if (MovementComponent == nullptr)
  {
    UE_LOG(LogCarla, Error, TEXT("Walker missing character movement component!"));
    return;
  }
#if 0 //@ Carla UE5
 // MovementComponent->MaxWalkSpeed = GetMaximumWalkSpeed();
 // MovementComponent->JumpZVelocity = 500.0f;
#endif
  CurrentCharacter->JumpMaxCount = 2;
}

void AWalkerController::ApplyWalkerControl(const FWalkerControl &InControl)
{
  Control = InControl;
}

void AWalkerController::GetBonesTransform(FWalkerBoneControlOut &WalkerBones)
{
  ACharacter *CurrentCharacter = GetCharacter();
  if (!CurrentCharacter) return;

  TArray<USkeletalMeshComponent *> SkeletalMeshes;
  CurrentCharacter->GetComponents<USkeletalMeshComponent>(SkeletalMeshes, false);
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
  ACharacter* CurrentCharacter = GetCharacter();
  if (!CurrentCharacter) return;

  TArray<USkeletalMeshComponent *> SkeletalMeshes;
  CurrentCharacter->GetComponents<USkeletalMeshComponent>(SkeletalMeshes, false);
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
  ACharacter* CurrentCharacter = GetCharacter();
  if (!CurrentCharacter) return;

  TArray<USkeletalMeshComponent *> SkeletalMeshes;
  CurrentCharacter->GetComponents<USkeletalMeshComponent>(SkeletalMeshes, false);
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
  ACharacter* CurrentCharacter = GetCharacter();
  if (!CurrentCharacter) return;

  TArray<USkeletalMeshComponent *> SkeletalMeshes;
  CurrentCharacter->GetComponents<USkeletalMeshComponent>(SkeletalMeshes, false);
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

  ACharacter* CurrentCharacter = GetCharacter();
  if (!CurrentCharacter) return;

  CurrentCharacter->AddMovementInput(Control.Direction,
        Control.Speed / GetMaximumWalkSpeed());
  if (Control.Jump)
  {
    CurrentCharacter->Jump();
  }
}
