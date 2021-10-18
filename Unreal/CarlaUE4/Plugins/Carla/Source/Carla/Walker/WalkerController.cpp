// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "Carla.h"
#include "Carla/Walker/WalkerController.h"

#include "Components/PoseableMeshComponent.h"
#include "Components/PrimitiveComponent.h"
#include "Containers/Map.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Pawn.h"

#include <boost/variant/apply_visitor.hpp>

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

void AWalkerController::ApplyWalkerControl(const FWalkerControl &InControl)
{
  Control = InControl;
  if (bManualBones)
  {
    SetManualBones(false);
  }
}

void AWalkerController::ApplyWalkerControl(const FWalkerBoneControlIn &InBoneControl)
{
  Control = InBoneControl;
  if (!bManualBones)
  {
    SetManualBones(true);
  }
}

void AWalkerController::SetManualBones(const bool bIsEnabled)
{
  bManualBones = bIsEnabled;

  auto *Character = GetCharacter();
  TArray<UPoseableMeshComponent *> PoseableMeshes;
  TArray<USkeletalMeshComponent *> SkeletalMeshes;
  Character->GetComponents<UPoseableMeshComponent>(PoseableMeshes, false);
  Character->GetComponents<USkeletalMeshComponent>(SkeletalMeshes, false);
  USkeletalMeshComponent *SkeletalMesh = SkeletalMeshes.IsValidIndex(0) ? SkeletalMeshes[0] : nullptr;
  if (SkeletalMesh)
  {
    if (bManualBones)
    {
      UPoseableMeshComponent *PoseableMesh =
          PoseableMeshes.IsValidIndex(0) ? PoseableMeshes[0] : AddNewBoneComponent(Character,
          SkeletalMesh->GetRelativeTransform().GetLocation(),
          SkeletalMesh->GetRelativeTransform().GetRotation().Rotator());
      PoseableMesh->SetSkeletalMesh(SkeletalMesh->SkeletalMesh);
      PoseableMesh->SetVisibility(true);
      SkeletalMesh->SetVisibility(false);
    }
    else
    {
      UPoseableMeshComponent *PoseableMesh = PoseableMeshes.IsValidIndex(0) ? PoseableMeshes[0] : nullptr;
      if (PoseableMesh)
        PoseableMesh->SetVisibility(false);
      SkeletalMesh->SetVisibility(true);
    }
  }
}

void AWalkerController::GetBonesTransform(FWalkerBoneControlOut &WalkerBones)
{
  auto *Character = GetCharacter();
  TArray<UPoseableMeshComponent *> PoseableMeshes;
  TArray<USkeletalMeshComponent *> SkeletalMeshes;
  Character->GetComponents<UPoseableMeshComponent>(PoseableMeshes, false);
  Character->GetComponents<USkeletalMeshComponent>(SkeletalMeshes, false);
  USkeletalMeshComponent *SkeletalMesh = SkeletalMeshes.IsValidIndex(0) ? SkeletalMeshes[0] : nullptr;
  if (!SkeletalMesh) return;

  UPoseableMeshComponent *PoseableMesh =
        PoseableMeshes.IsValidIndex(0) ? PoseableMeshes[0] : AddNewBoneComponent(Character,
        SkeletalMesh->GetRelativeTransform().GetLocation(),
        SkeletalMesh->GetRelativeTransform().GetRotation().Rotator());
  PoseableMesh->SetSkeletalMesh(SkeletalMesh->SkeletalMesh);
  PoseableMesh->SetVisibility(false);
  PoseableMesh->CopyPoseFromSkeletalComponent(SkeletalMesh);
    
  FPoseSnapshot Snap;
  SkeletalMesh->SnapshotPose(Snap);
  for (int i=0; i<Snap.BoneNames.Num(); ++i)
  {
    FWalkerBoneControlOutData Transforms;
    Transforms.World = PoseableMesh->GetBoneTransformByName(Snap.BoneNames[i], EBoneSpaces::WorldSpace);
    Transforms.Component = PoseableMesh->GetBoneTransformByName(Snap.BoneNames[i], EBoneSpaces::ComponentSpace);
    Transforms.Relative = Snap.LocalTransforms[i];
    WalkerBones.BoneTransforms.Add(Snap.BoneNames[i].ToString(), Transforms);
    // FVector Loc = SkeletalMesh->GetBoneLocation(Snap.BoneNames[i], EBoneSpaces::WorldSpace);        
    // Trans = Snap.LocalTransforms[i];
    // Trans.SetLocation(Loc);
    // FTransform TransRel = Snap.LocalTransforms[i];
    // if (i == 0)
    // {
    //   UE_LOG(LogCarla, Log, TEXT("World (%f, %f, %f), Relative (%f, %f, %f), Component (%f, %f, %f)"), Trans.GetLocation().X, Trans.GetLocation().Y, Trans.GetLocation().Z, TransRel.GetLocation().X, TransRel.GetLocation().Y, TransRel.GetLocation().Z, TransComp.GetLocation().X, TransComp.GetLocation().Y, TransComp.GetLocation().Z);
    //   UE_LOG(LogCarla, Log, TEXT("World (%f, %f, %f), Relative (%f, %f, %f), Component (%f, %f, %f)"), Trans.GetLocation().X, Trans.GetLocation().Y, Trans.GetLocation().Z, TransRel.GetLocation().X, TransRel.GetLocation().Y, TransRel.GetLocation().Z, TransComp.GetLocation().X, TransComp.GetLocation().Y, TransComp.GetLocation().Z);
    // }
  }
}

void AWalkerController::ControlTickVisitor::operator()(const FWalkerControl &WalkerControl)
{
  auto *Character = Controller->GetCharacter();
  if (Character != nullptr)
  {
    Character->AddMovementInput(WalkerControl.Direction,
        WalkerControl.Speed / Controller->GetMaximumWalkSpeed());
    if (WalkerControl.Jump)
    {
      Character->Jump();
    }
  }
}

void AWalkerController::ControlTickVisitor::operator()(FWalkerBoneControlIn &WalkerBoneControlIn)
{
  auto *Character = Controller->GetCharacter();
  if (Character == nullptr)
  {
    return;
  }
  TArray<UPoseableMeshComponent *> PoseableMeshes;
  Character->GetComponents<UPoseableMeshComponent>(PoseableMeshes, false);
  UPoseableMeshComponent *PoseableMesh = PoseableMeshes.IsValidIndex(0) ? PoseableMeshes[0] : nullptr;
  if (PoseableMesh)
  {
    for (const TPair<FString, FTransform> &pair : WalkerBoneControlIn.BoneTransforms)
    {
      FName BoneName = FName(*pair.Key);
      PoseableMesh->SetBoneTransformByName(BoneName, pair.Value, EBoneSpaces::Type::ComponentSpace);
    }
    WalkerBoneControlIn.BoneTransforms.Empty();
  }
}

void AWalkerController::Tick(float DeltaSeconds)
{
  TRACE_CPUPROFILER_EVENT_SCOPE(AWalkerController::Tick);
  Super::Tick(DeltaSeconds);
  AWalkerController::ControlTickVisitor ControlTickVisitor(this);
  boost::apply_visitor(ControlTickVisitor, Control);
}
