// Copyright (c) 2025 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Templates/SubclassOf.h"

#include "VehicleAuthoringLibrary.generated.h"

class UAnimBlueprint;
class UBlueprint;
class USkeletalMesh;
class USkeleton;
class UStaticMesh;
class UVehicleWheel;

/// Editor-only authoring helpers used to assemble a CARLA wheeled vehicle from
/// its individual assets (skeletal mesh, animation blueprint, physics asset,
/// wheels and the spawnable vehicle blueprint). These functions are meant to be
/// driven from Python through the PythonScriptPlugin so a full vehicle can be
/// generated programmatically.
///
/// Every function is best-effort and editor-only: each guards its inputs and
/// returns null / false (logging through LogCarlaTools) instead of crashing.
UCLASS()
class CARLATOOLS_API UVehicleAuthoringLibrary : public UBlueprintFunctionLibrary
{
  GENERATED_BODY()

public:

  /// Duplicate @a Template into @a DestPackagePath and retarget the resulting
  /// animation blueprint to @a Skeleton. The whole asset is duplicated (so the
  /// AnimGraph is preserved as-is) and only the skeleton references are
  /// retargeted; the AnimGraph nodes are never rebuilt by hand. The new package
  /// is compiled and saved. Returns the new animation blueprint or null.
  UFUNCTION(BlueprintCallable, CallInEditor, Category = "CarlaVehicle")
  static UAnimBlueprint* CreateVehicleAnimBP(
      USkeleton* Skeleton,
      UAnimBlueprint* Template,
      const FString& DestPackagePath);

  /// Build a DEDICATED physics asset for @a Mesh and configure it as a CARLA
  /// wheeled vehicle. A freshly-duplicated mesh usually still references the
  /// TEMPLATE's shared physics asset (e.g. SM_Mustang1966_PhysicsAsset); editing
  /// that in place would corrupt the donor vehicle, so a per-mesh asset named
  /// "<Mesh>_PhysicsAsset" is created next to the mesh (auto-generating a convex
  /// chassis hull from the skinned geometry) and assigned to @a Mesh.
  ///
  /// The chassis (the single non-wheel body) is left SIMULATED; when
  /// @a CollisionStaticMesh is supplied its collision hull replaces the
  /// auto-generated one (a hand-authored hull is higher quality). Each of the
  /// four canonical wheel bones is given a KINEMATIC sphere of radius
  /// @a WheelRadius (cm): the PxVehicle SDK owns wheel motion via the raycast
  /// WheelSetups, so these bodies are collision-only and must not simulate.
  /// Returns false (and logs) when the mesh or its generation fails.
  UFUNCTION(BlueprintCallable, CallInEditor, Category = "CarlaVehicle")
  static bool SetupVehiclePhysicsAsset(
      USkeletalMesh* Mesh,
      UStaticMesh* CollisionStaticMesh,
      float WheelRadius);

  /// Apply the given defaults to the class-default-object of a UVehicleWheel
  /// blueprint and save it. @a TireConfig is optional and only applied when it
  /// is a UTireConfig. Returns false when @a WheelClass is invalid.
  UFUNCTION(BlueprintCallable, CallInEditor, Category = "CarlaVehicle")
  static bool ConfigureWheel(
      TSubclassOf<UVehicleWheel> WheelClass,
      float Radius,
      float Width,
      float Mass,
      float SteerAngleDeg,
      bool bAffectedByHandbrake,
      UObject* TireConfig);

  /// Create a spawnable CARLA vehicle blueprint named @a Name in folder
  /// @a DestPackagePath by DUPLICATING @a TemplateBlueprint (e.g. BP_Mustang66)
  /// and repointing its inherited components to the new assets. Duplication is
  /// required because the skeletal mesh, anim class and wheel setups live on
  /// inherited *native* components (AWheeledVehicle::Mesh / VehicleMovement):
  /// the template already carries those overrides in its generated-class CDO
  /// (a proven, serialising slot), so overwriting their values + recompiling
  /// persists, whereas establishing brand-new overrides on a fresh blueprint
  /// does not. The CDO Mesh receives @a Mesh + @a Anim's generated class, the
  /// movement component's WheelSetups are filled from @a Wheels / @a WheelBones.
  ///
  /// @a RaycastMesh repoints the CustomCollision component when reachable; that
  /// component is SCS-inherited (not on the CDO), so for the MVP it is left at
  /// the template's value when it cannot be edited (logged). Compiled and saved.
  /// Returns the new blueprint or null.
  UFUNCTION(BlueprintCallable, CallInEditor, Category = "CarlaVehicle")
  static UBlueprint* CreateVehicleBlueprint(
      const FString& Name,
      const FString& DestPackagePath,
      UBlueprint* TemplateBlueprint,
      USkeletalMesh* Mesh,
      UAnimBlueprint* Anim,
      UStaticMesh* RaycastMesh,
      const TArray<TSubclassOf<UVehicleWheel>>& Wheels,
      const TArray<FName>& WheelBones);

  /// Compile @a BP through Kismet and save its package. Returns false on
  /// failure (invalid blueprint or save error).
  UFUNCTION(BlueprintCallable, CallInEditor, Category = "CarlaVehicle")
  static bool CompileAndSaveBlueprint(UBlueprint* BP);

private:

  /// Save the package that owns @a Asset to disk using the conventions shared
  /// by the rest of CarlaTools (RF_Public | RF_Standalone, SAVE_NoError).
  static bool SavePackageForAsset(UObject* Asset);
};
