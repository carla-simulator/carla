// Copyright (c) 2025 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "Vehicle/VehicleAuthoringLibrary.h"

#include "CarlaTools.h"

#include "AssetRegistryModule.h"
#include "AssetToolsModule.h"
#include "Editor.h"
#include "EditorAnimUtils.h"
#include "IAssetTools.h"
#include "Animation/AnimBlueprint.h"
#include "Animation/Skeleton.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/Blueprint.h"
#include "Engine/SkeletalMesh.h"
#include "Engine/StaticMesh.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Kismet2/BlueprintEditorUtils.h"
#include "Kismet2/KismetEditorUtilities.h"
#include "Misc/PackageName.h"
#include "Misc/Paths.h"
#include "Modules/ModuleManager.h"
// FKAggregateGeom / FKSphereElem come transitively from BodySetup.h (they are
// the type of UBodySetup::AggGeom), so no direct AggregateGeom.h include is
// needed — and in UE4.26 that header lives in the PhysicsCore module anyway.
#include "PhysicsEngine/BodySetup.h"
// BodySetupEnums.h lives in the PhysicsCore module's public root (UE4.26),
// not under PhysicsEngine/. PhysicsCore is a Build.cs dependency.
#include "BodySetupEnums.h"
// USkeletalBodySetup is declared inside PhysicsAsset.h in UE4.26 (no separate
// SkeletalBodySetup.h header exists).
#include "PhysicsEngine/PhysicsAsset.h"
// FPhysicsAssetUtils::CreateFromSkeletalMesh + FPhysAssetCreateParams live in the
// UnrealEd module (already a private dependency); the header sits at its public
// root, hence the bare filename include.
#include "PhysicsAssetUtils.h"
#include "UObject/Package.h"
#include "UObject/UObjectGlobals.h"

#include "TireConfig.h"
#include "VehicleWheel.h"
#include "WheeledVehicleMovementComponent4W.h"

#include "Carla/Vehicle/CarlaWheeledVehicle.h"

bool UVehicleAuthoringLibrary::SavePackageForAsset(UObject* Asset)
{
  if (!Asset)
  {
    return false;
  }

  UPackage* Package = Asset->GetOutermost();
  if (!Package)
  {
    return false;
  }

  Package->MarkPackageDirty();
  const FString PackageFileName = FPackageName::LongPackageNameToFilename(
      Package->GetName(), FPackageName::GetAssetPackageExtension());

  return UPackage::SavePackage(
      Package,
      Asset,
      EObjectFlags::RF_Public | EObjectFlags::RF_Standalone,
      *PackageFileName,
      GError,
      nullptr,
      true,
      true,
      SAVE_NoError);
}

UAnimBlueprint* UVehicleAuthoringLibrary::CreateVehicleAnimBP(
    USkeleton* Skeleton,
    UAnimBlueprint* Template,
    const FString& DestPackagePath)
{
  if (!Skeleton || !Template)
  {
    UE_LOG(LogCarlaTools, Error, TEXT("CreateVehicleAnimBP: invalid Skeleton or Template."));
    return nullptr;
  }

  const FString PackagePath = FPaths::GetPath(DestPackagePath);
  const FString AssetName = FPaths::GetBaseFilename(DestPackagePath);
  if (PackagePath.IsEmpty() || AssetName.IsEmpty())
  {
    UE_LOG(LogCarlaTools, Error, TEXT("CreateVehicleAnimBP: invalid DestPackagePath '%s'."), *DestPackagePath);
    return nullptr;
  }

  // Duplicate the whole template (the AnimGraph is preserved verbatim) instead
  // of recreating the graph nodes by hand.
  IAssetTools& AssetTools = FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools").Get();
  UObject* Duplicated = AssetTools.DuplicateAsset(AssetName, PackagePath, Template);
  UAnimBlueprint* NewAnimBP = Cast<UAnimBlueprint>(Duplicated);
  if (!NewAnimBP)
  {
    UE_LOG(LogCarlaTools, Error, TEXT("CreateVehicleAnimBP: failed to duplicate template into %s."), *DestPackagePath);
    return nullptr;
  }

  // Retarget the duplicated blueprint onto the requested skeleton.
  USkeleton* OldSkeleton = NewAnimBP->TargetSkeleton;
  if (OldSkeleton != Skeleton)
  {
    NewAnimBP->TargetSkeleton = Skeleton;
    if (OldSkeleton)
    {
      TArray<TWeakObjectPtr<UObject>> AssetsToRetarget;
      AssetsToRetarget.Add(NewAnimBP);
      EditorAnimUtils::RetargetAnimations(
          OldSkeleton,
          Skeleton,
          AssetsToRetarget,
          /*bRetargetReferredAssets*/ false,
          /*NameRule*/ nullptr,
          /*bConvertSpaces*/ false);
    }
    FBlueprintEditorUtils::RefreshAllNodes(NewAnimBP);
  }

  FKismetEditorUtilities::CompileBlueprint(NewAnimBP);

  if (!SavePackageForAsset(NewAnimBP))
  {
    UE_LOG(LogCarlaTools, Warning, TEXT("CreateVehicleAnimBP: failed to save %s."), *DestPackagePath);
  }

  return NewAnimBP;
}

bool UVehicleAuthoringLibrary::SetupVehiclePhysicsAsset(
    USkeletalMesh* Mesh,
    UStaticMesh* CollisionStaticMesh,
    float WheelRadius)
{
  if (!Mesh)
  {
    UE_LOG(LogCarlaTools, Error, TEXT("SetupVehiclePhysicsAsset: invalid Mesh."));
    return false;
  }

  // Canonical CARLA 4W wheel bones (fixed PxVehicleDrive4W roles). The chassis is
  // whatever body is NOT one of these, so the base bone name does not have to be
  // hard-coded (the shipped Mustang skeleton and the USD importer disagree on it:
  // "Vehicle_Base" vs the mesh root).
  static const TArray<FName> WheelBones = {
      FName(TEXT("Wheel_Front_Left")),
      FName(TEXT("Wheel_Front_Right")),
      FName(TEXT("Wheel_Rear_Left")),
      FName(TEXT("Wheel_Rear_Right"))
  };
  const TSet<FName> WheelBoneSet(WheelBones);
  const float SphereRadius = (WheelRadius > 1.0f) ? WheelRadius : 32.0f;

  // Build (or reuse) a DEDICATED physics asset owned by the mesh's own package.
  // A duplicated mesh inherits the donor's shared PhysicsAsset pointer; mutating
  // that would corrupt the donor vehicle, so we never touch it.
  const FString MeshPackageName = Mesh->GetOutermost()->GetName();       // /Game/.../SM_Foo
  const FString PackagePath = FPackageName::GetLongPackagePath(MeshPackageName);
  const FString AssetName = Mesh->GetName() + TEXT("_PhysicsAsset");
  const FString NewPackageName = PackagePath + TEXT("/") + AssetName;
  const FString ObjectPath = NewPackageName + TEXT(".") + AssetName;

  UPhysicsAsset* PhysicsAsset = LoadObject<UPhysicsAsset>(
      nullptr, *ObjectPath, nullptr, LOAD_NoWarn | LOAD_Quiet);
  if (!PhysicsAsset)
  {
    UPackage* Package = CreatePackage(*NewPackageName);
    if (!Package)
    {
      UE_LOG(LogCarlaTools, Error, TEXT("SetupVehiclePhysicsAsset: could not create package '%s'."), *NewPackageName);
      return false;
    }
    PhysicsAsset = NewObject<UPhysicsAsset>(
        Package, *AssetName, RF_Public | RF_Standalone | RF_Transactional);

    // Auto-generate collision from the skinned geometry: a single convex hull per
    // bone, no ragdoll constraints, one body per bone so the wheel bones exist.
    FPhysAssetCreateParams Params;
    Params.MinBoneSize = 1.0f;              // wheels/base are small — do not skip them
    Params.GeomType = EFG_SingleConvexHull; // chassis-style hull
    Params.bBodyForAll = true;              // guarantee a body per bone
    Params.bCreateConstraints = false;      // a wheeled vehicle needs no joints
    Params.bWalkPastSmall = false;
    Params.bDisableCollisionsByDefault = true;

    FText Err;
    if (!FPhysicsAssetUtils::CreateFromSkeletalMesh(
            PhysicsAsset, Mesh, Params, Err, /*bSetToMesh*/ true))
    {
      UE_LOG(LogCarlaTools, Error,
          TEXT("SetupVehiclePhysicsAsset: CreateFromSkeletalMesh failed for '%s': %s"),
          *Mesh->GetName(), *Err.ToString());
      return false;
    }
    FAssetRegistryModule::AssetCreated(PhysicsAsset);
    UE_LOG(LogCarlaTools, Log, TEXT("SetupVehiclePhysicsAsset: created '%s'."), *ObjectPath);
  }

  // Point the mesh at our owned asset (CreateFromSkeletalMesh already does this
  // for the fresh case; make it explicit for the reuse case too).
  if (Mesh->PhysicsAsset != PhysicsAsset)
  {
    Mesh->Modify();
    Mesh->PhysicsAsset = PhysicsAsset;
  }

  PhysicsAsset->Modify();

  // Diagnostic: record the actual body layout (bone / physics type / geometry).
  // Physics-asset bodies are not introspectable from UE4.26 Python, so this log
  // is the authoritative record of what the generator produced.
  UE_LOG(LogCarlaTools, Log, TEXT("SetupVehiclePhysicsAsset: '%s' has %d bodies:"),
      *PhysicsAsset->GetName(), PhysicsAsset->SkeletalBodySetups.Num());
  for (int32 i = 0; i < PhysicsAsset->SkeletalBodySetups.Num(); ++i)
  {
    if (const USkeletalBodySetup* B = PhysicsAsset->SkeletalBodySetups[i])
    {
      UE_LOG(LogCarlaTools, Log,
          TEXT("  [%d] bone=%s type=%d sph=%d box=%d sphyl=%d convex=%d"),
          i, *B->BoneName.ToString(), (int32)B->PhysicsType.GetValue(),
          B->AggGeom.SphereElems.Num(), B->AggGeom.BoxElems.Num(),
          B->AggGeom.SphylElems.Num(), B->AggGeom.ConvexElems.Num());
    }
  }

  // Wheels: kinematic sphere sized to the wheel radius. Kinematic because the
  // PxVehicle SDK drives the wheels via the raycast WheelSetups, not these bodies
  // — a simulated wheel body fights the suspension and freezes the car.
  for (const FName& WheelBone : WheelBones)
  {
    USkeletalBodySetup* WheelBody = nullptr;
    const int32 BodyIdx = PhysicsAsset->FindBodyIndex(WheelBone);
    if (BodyIdx != INDEX_NONE)
    {
      WheelBody = PhysicsAsset->SkeletalBodySetups[BodyIdx];
    }
    else
    {
      WheelBody = NewObject<USkeletalBodySetup>(PhysicsAsset, NAME_None, RF_Transactional);
      WheelBody->BoneName = WheelBone;
      PhysicsAsset->SkeletalBodySetups.Add(WheelBody);
      PhysicsAsset->UpdateBodySetupIndexMap();
    }
    if (!WheelBody)
    {
      UE_LOG(LogCarlaTools, Warning, TEXT("SetupVehiclePhysicsAsset: no body for wheel bone '%s'."), *WheelBone.ToString());
      continue;
    }

    WheelBody->Modify();
    // Must clear the auto-generated convex first, else bCreatedPhysicsMeshes stays
    // set and the new sphere is ignored.
    WheelBody->RemoveSimpleCollision();
    FKSphereElem Sphere;
    Sphere.Radius = SphereRadius;
    Sphere.Center = FVector::ZeroVector;
    WheelBody->AggGeom.SphereElems.Add(Sphere);
    WheelBody->PhysicsType = PhysType_Kinematic;
    WheelBody->InvalidatePhysicsData();
    WheelBody->CreatePhysicsMeshes();
  }

  // Chassis: the single non-wheel body. Copy the artist collision hull when
  // provided (mirrors USDImporterWidget::CopyCollisionToPhysicsAsset), otherwise
  // keep the auto-generated convex. Simulated so it carries the vehicle mass.
  bool bChassisFound = false;
  for (USkeletalBodySetup* Body : PhysicsAsset->SkeletalBodySetups)
  {
    if (!Body || WheelBoneSet.Contains(Body->BoneName))
    {
      continue;
    }
    bChassisFound = true;
    Body->Modify();
    if (CollisionStaticMesh && CollisionStaticMesh->BodySetup)
    {
      Body->RemoveSimpleCollision();
      Body->AggGeom = CollisionStaticMesh->BodySetup->AggGeom;
    }
    Body->PhysicsType = PhysType_Default;
    Body->InvalidatePhysicsData();
    Body->CreatePhysicsMeshes();
  }
  if (!bChassisFound)
  {
    UE_LOG(LogCarlaTools, Warning,
        TEXT("SetupVehiclePhysicsAsset: no non-wheel (chassis) body found in '%s'."),
        *PhysicsAsset->GetName());
  }

  PhysicsAsset->UpdateBodySetupIndexMap();
  PhysicsAsset->UpdateBoundsBodiesArray();

  if (!SavePackageForAsset(PhysicsAsset))
  {
    UE_LOG(LogCarlaTools, Warning, TEXT("SetupVehiclePhysicsAsset: failed to save physics asset '%s'."), *PhysicsAsset->GetName());
  }
  // Persist the mesh's updated PhysicsAsset pointer.
  if (!SavePackageForAsset(Mesh))
  {
    UE_LOG(LogCarlaTools, Warning, TEXT("SetupVehiclePhysicsAsset: failed to save mesh '%s'."), *Mesh->GetName());
  }

  return true;
}

bool UVehicleAuthoringLibrary::ConfigureWheel(
    TSubclassOf<UVehicleWheel> WheelClass,
    float Radius,
    float Width,
    float Mass,
    float SteerAngleDeg,
    bool bAffectedByHandbrake,
    UObject* TireConfig)
{
  if (!*WheelClass)
  {
    UE_LOG(LogCarlaTools, Error, TEXT("ConfigureWheel: invalid WheelClass."));
    return false;
  }

  UVehicleWheel* WheelCDO = WheelClass->GetDefaultObject<UVehicleWheel>();
  if (!WheelCDO)
  {
    UE_LOG(LogCarlaTools, Error, TEXT("ConfigureWheel: could not resolve wheel CDO."));
    return false;
  }

  WheelCDO->ShapeRadius = Radius;
  WheelCDO->ShapeWidth = Width;
  WheelCDO->Mass = Mass;
  WheelCDO->SteerAngle = SteerAngleDeg;
  WheelCDO->bAffectedByHandbrake = bAffectedByHandbrake;
  if (UTireConfig* Tire = Cast<UTireConfig>(TireConfig))
  {
    WheelCDO->TireConfig = Tire;
  }
  WheelCDO->PostEditChange();

  // The wheel usually lives in its own blueprint asset; persist that package.
  if (UBlueprint* WheelBP = Cast<UBlueprint>(WheelClass->ClassGeneratedBy))
  {
    WheelBP->Modify();
    return SavePackageForAsset(WheelBP);
  }

  // Native wheel class: defaults are applied in memory but there is no asset to
  // save. Report success since the CDO was updated.
  UE_LOG(LogCarlaTools, Warning, TEXT("ConfigureWheel: '%s' is a native class; defaults applied in memory only."), *WheelClass->GetName());
  return true;
}

UBlueprint* UVehicleAuthoringLibrary::CreateVehicleBlueprint(
    const FString& Name,
    const FString& DestPackagePath,
    UBlueprint* TemplateBlueprint,
    USkeletalMesh* Mesh,
    UAnimBlueprint* Anim,
    UStaticMesh* RaycastMesh,
    const TArray<TSubclassOf<UVehicleWheel>>& Wheels,
    const TArray<FName>& WheelBones)
{
  if (!Mesh)
  {
    UE_LOG(LogCarlaTools, Error, TEXT("CreateVehicleBlueprint: invalid Mesh."));
    return nullptr;
  }
  if (!TemplateBlueprint)
  {
    UE_LOG(LogCarlaTools, Error, TEXT("CreateVehicleBlueprint: invalid TemplateBlueprint."));
    return nullptr;
  }
  if (Wheels.Num() != WheelBones.Num())
  {
    UE_LOG(LogCarlaTools, Error,
        TEXT("CreateVehicleBlueprint: Wheels (%d) and WheelBones (%d) count mismatch."),
        Wheels.Num(), WheelBones.Num());
    return nullptr;
  }

  // Duplicate the template vehicle blueprint. The skeletal mesh, anim class and
  // wheel setups live on inherited *native* components (AWheeledVehicle::Mesh
  // and VehicleMovement, created with CreateDefaultSubobject). The template
  // already carries those overrides in its generated-class CDO - a recorded slot
  // that is proven to serialise (the shipped BP stores its mesh there) - so we
  // overwrite the values on the already-compiled duplicate CDO and save WITHOUT
  // recompiling (a recompile reverts native-component slots to the parent
  // default). Establishing brand-new overrides on a fresh blueprint does not
  // serialise, which is why duplication is required.
  IAssetTools& AssetTools = FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools").Get();
  UObject* Duplicated = AssetTools.DuplicateAsset(Name, DestPackagePath, TemplateBlueprint);
  UBlueprint* NewBP = Cast<UBlueprint>(Duplicated);
  if (!NewBP || !NewBP->GeneratedClass)
  {
    UE_LOG(LogCarlaTools, Error, TEXT("CreateVehicleBlueprint: failed to duplicate template into %s/%s."), *DestPackagePath, *Name);
    return nullptr;
  }

  ACarlaWheeledVehicle* CDOVehicle = Cast<ACarlaWheeledVehicle>(NewBP->GeneratedClass->GetDefaultObject());
  if (!CDOVehicle)
  {
    UE_LOG(LogCarlaTools, Error, TEXT("CreateVehicleBlueprint: duplicated blueprint CDO is not an ACarlaWheeledVehicle."));
    return nullptr;
  }

  // NOTE: the skeletal mesh is repointed LAST (see below) and the blueprint is
  // saved WITHOUT a recompile. The duplicate already carries the template's
  // recorded native-component override slot for Mesh.SkeletalMesh, so changing
  // the value on the already-compiled CDO and saving persists it; a recompile
  // here reverts the native slot to the parent default (None) and is the reason
  // earlier attempts produced None.

  // Repoint the 4W movement component's wheel setups (also a native CDO
  // sub-object) to the new wheel classes / bones.
  if (UWheeledVehicleMovementComponent4W* MovementComp = Cast<UWheeledVehicleMovementComponent4W>(
          CDOVehicle->GetVehicleMovementComponent()))
  {
    if (Wheels.Num() > 0)
    {
      MovementComp->Modify();
      TArray<FWheelSetup> WheelSetups;
      for (int32 i = 0; i < Wheels.Num(); ++i)
      {
        FWheelSetup Setup;
        Setup.WheelClass = Wheels[i];
        Setup.BoneName = WheelBones[i];
        WheelSetups.Add(Setup);
      }
      MovementComp->WheelSetups = WheelSetups;
    }
  }
  else
  {
    UE_LOG(LogCarlaTools, Warning, TEXT("CreateVehicleBlueprint: 4W movement component not found on CDO."));
  }

  // Repoint the CustomCollision sensor proxy when it is reachable from the CDO.
  // It is an SCS-inherited component (not a CDO sub-object), so for the MVP it
  // is left at the template's value when it cannot be edited here.
  if (RaycastMesh)
  {
    TArray<UStaticMeshComponent*> StaticMeshComps;
    CDOVehicle->GetComponents(StaticMeshComps);
    UStaticMeshComponent* CollisionComp = nullptr;
    for (UStaticMeshComponent* Comp : StaticMeshComps)
    {
      if (Comp->GetName().Contains(TEXT("CustomCollision")))
      {
        CollisionComp = Comp;
        break;
      }
    }
    if (CollisionComp)
    {
      CollisionComp->Modify();
      CollisionComp->SetStaticMesh(RaycastMesh);
    }
    else
    {
      UE_LOG(LogCarlaTools, Warning,
          TEXT("CreateVehicleBlueprint: CustomCollision not reachable from CDO (SCS component); left at template value."));
    }
  }

  // Repoint the inherited native Mesh component LAST, directly on the already-
  // compiled duplicate CDO, and do NOT recompile afterwards. The duplicate
  // inherited BP_Mustang66's recorded Mesh.SkeletalMesh override slot, so
  // overwriting the value on the live CDO and saving persists it; a recompile
  // here reverts the slot to the parent default (None).
  if (USkeletalMeshComponent* CDOMesh = CDOVehicle->GetMesh())
  {
    UE_LOG(LogCarlaTools, Log,
        TEXT("CreateVehicleBlueprint: Mesh arg='%s'; CDO Mesh.SkeletalMesh before='%s'."),
        Mesh ? *Mesh->GetName() : TEXT("<null>"),
        CDOMesh->SkeletalMesh ? *CDOMesh->SkeletalMesh->GetName() : TEXT("<null>"));

    CDOMesh->Modify();
    CDOMesh->SetSkeletalMesh(Mesh);
    if (Anim && Anim->GeneratedClass)
    {
      CDOMesh->SetAnimInstanceClass(Anim->GeneratedClass);
    }
  }
  else
  {
    UE_LOG(LogCarlaTools, Warning, TEXT("CreateVehicleBlueprint: CDO Mesh component not found; SkeletalMesh not repointed."));
  }

  // Decisive diagnostic immediately before save (no recompile in between).
  if (USkeletalMeshComponent* CheckMesh = CDOVehicle->GetMesh())
  {
    UE_LOG(LogCarlaTools, Log,
        TEXT("CreateVehicleBlueprint: CDO Mesh.SkeletalMesh before save='%s' (Mesh arg='%s')."),
        CheckMesh->SkeletalMesh ? *CheckMesh->SkeletalMesh->GetName() : TEXT("<null>"),
        Mesh ? *Mesh->GetName() : TEXT("<null>"));
  }

  // Save the duplicated blueprint. NOTE: the orchestrator sets the skeletal mesh
  // from editor Python (set_editor_property + save_loaded_asset) after this call,
  // so it owns mesh persistence; the C++ SetSkeletalMesh above is a best-effort
  // default.
  if (!SavePackageForAsset(NewBP))
  {
    UE_LOG(LogCarlaTools, Warning, TEXT("CreateVehicleBlueprint: failed to save %s/%s."), *DestPackagePath, *Name);
  }

  return NewBP;
}

bool UVehicleAuthoringLibrary::CompileAndSaveBlueprint(UBlueprint* BP)
{
  if (!BP)
  {
    UE_LOG(LogCarlaTools, Error, TEXT("CompileAndSaveBlueprint: invalid blueprint."));
    return false;
  }

  FKismetEditorUtilities::CompileBlueprint(BP);
  return SavePackageForAsset(BP);
}
