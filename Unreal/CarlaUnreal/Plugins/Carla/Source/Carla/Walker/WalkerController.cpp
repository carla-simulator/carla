// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma
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
#include "Navigation/CrowdFollowingComponent.h"
#include "Navigation/PathFollowingComponent.h"
#include <util/ue-header-guard-end.h>

AWalkerController::AWalkerController(const FObjectInitializer &ObjectInitializer)
  // Swap the stock path following component for the Detour crowd one so
  // navigating walkers avoid each other (same pattern as
  // ADetourCrowdAIController).
  : Super(ObjectInitializer.SetDefaultSubobjectClass<UCrowdFollowingComponent>(TEXT("PathFollowingComponent")))
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

  UCharacterMovementComponent *MovementComponent = CurrentCharacter->GetCharacterMovement();
  if (MovementComponent == nullptr)
  {
    UE_LOG(LogCarla, Error, TEXT("Walker missing character movement component!"));
    return;
  }
  MovementComponent->MaxWalkSpeed = GetMaximumWalkSpeed();
  MovementComponent->JumpZVelocity = 500.0f;
  CurrentCharacter->JumpMaxCount = 2;
}

void AWalkerController::ApplyWalkerControl(const FWalkerControl &InControl)
{
  // Manual control always wins: a client applying a WalkerControl while the
  // server is path-following expects the walker to obey it (this is also the
  // path the legacy client-side navigation drives through).
  if (bNavigationActive)
  {
    StopNavigation();
  }
  Control = InControl;
}

bool AWalkerController::StartNavigation()
{
  if (GetCharacter() == nullptr)
  {
    return false;
  }
  SetNavigationActive(true);
  return true;
}

bool AWalkerController::GoToNavLocation(const FVector &WorldLocation)
{
  if (!StartNavigation())
  {
    return false;
  }
  const EPathFollowingRequestResult::Type Result = MoveToLocation(
      WorldLocation,
      /*AcceptanceRadius=*/50.0f,
      /*bStopOnOverlap=*/true,
      /*bUsePathfinding=*/true,
      /*bProjectDestinationToNavigation=*/true);
  if (Result == EPathFollowingRequestResult::Failed)
  {
    UE_LOG(LogCarla, Warning,
        TEXT("Walker %s: MoveToLocation to (%s) failed (no navmesh under target?)"),
        *GetName(), *WorldLocation.ToCompactString());
    return false;
  }
  return true;
}

bool AWalkerController::SetNavMaxSpeed(float SpeedCmPerSec)
{
  NavMaxSpeed = FMath::Clamp(SpeedCmPerSec, 0.0f, GetMaximumWalkSpeed());
  ACharacter *CurrentCharacter = GetCharacter();
  if (CurrentCharacter == nullptr)
  {
    return false;
  }
  if (bNavigationActive)
  {
    if (auto *Movement = CurrentCharacter->GetCharacterMovement())
    {
      Movement->MaxWalkSpeed = NavMaxSpeed;
    }
  }
  return true;
}

bool AWalkerController::StopNavigation()
{
  if (bNavigationActive)
  {
    StopMovement();
    SetNavigationActive(false);
  }
  return true;
}

void AWalkerController::SetNavigationActive(bool bActive)
{
  bNavigationActive = bActive;
  ACharacter *CurrentCharacter = GetCharacter();
  if (CurrentCharacter == nullptr)
  {
    return;
  }
  if (auto *Movement = CurrentCharacter->GetCharacterMovement())
  {
    // While navigating, MaxWalkSpeed IS the walking speed (path following
    // moves at full input scale). The manual path instead scales its input
    // against GetMaximumWalkSpeed(), so it needs the historical ceiling back.
    Movement->MaxWalkSpeed = bActive ? NavMaxSpeed : GetMaximumWalkSpeed();
  }
  if (bActive)
  {
    // Drop any leftover manual input so it cannot fight the path following.
    Control = FWalkerControl();
  }
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

  // In navigation mode the crowd following component feeds the movement
  // input; injecting the manual control here as well would double-drive
  // the character.
  if (bNavigationActive) return;

  CurrentCharacter->AddMovementInput(Control.Direction,
        Control.Speed / GetMaximumWalkSpeed());

  if (Control.Jump)
  {
    CurrentCharacter->Jump();
  }
}
