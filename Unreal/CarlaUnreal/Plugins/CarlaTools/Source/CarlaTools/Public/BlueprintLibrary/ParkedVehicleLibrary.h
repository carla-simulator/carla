// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include <util/ue-header-guard-begin.h>
#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Engine/StaticMesh.h"
#include <util/ue-header-guard-end.h>

#include "ParkedVehicleLibrary.generated.h"

UCLASS(BlueprintType)
class CARLATOOLS_API UParkedVehicleLibrary : public UBlueprintFunctionLibrary
{
  GENERATED_BODY()

public:

  /// Sets Mesh on Vehicle's UStaticMeshComponent named MeshComponentName,
  /// and (if GlassMesh is set) GlassMesh on the one named
  /// GlassComponentName -- neither is necessarily Vehicle's root
  /// component. BP_ParkCars' root is a bare DefaultSceneRoot with the
  /// body and glass meshes living on two separate sibling components
  /// ("StaticMesh", "Glass"), so going through GetRootComponent (as the
  /// old Blueprint graph did) silently fails there.
  ///
  /// Both meshes are set in the one call so that Mesh and GlassMesh are
  /// guaranteed to come from the same picked vehicle definition: picking
  /// them from two separate BlueprintCallable calls fed by the same
  /// "random index into an array" pure node chain doesn't guarantee that
  /// -- pure nodes with a side effect (here, a random stream advancing)
  /// re-evaluate per exec statement that reads them, so a second call
  /// can silently roll a different index than the first and pair up a
  /// body with another vehicle's glass.
  ///
  /// Also recolors the first material slot on Mesh whose name contains
  /// BodyworkSlotSubstring (case-insensitive) with Color, via a
  /// per-instance dynamic material instance (never the shared asset), and
  /// marks Vehicle's root component Static (parked vehicles never move).
  ///
  /// A substring match, not an exact one: the "parked" car meshes each
  /// name their paintable slot differently -- "MI_BodyWork_Mustang",
  /// "Vh_Car_AudiTT_BodyworkMat", "Vh_Car_ToyotaPrius_BodyworkMat" -- no
  /// two vehicles share one literal slot name, only the substring
  /// "Bodywork" in common. If no slot matches, the mesh is still set;
  /// only the recolor step is skipped.
  ///
  /// Returns false, doing nothing, if MeshComponentName doesn't name a
  /// UStaticMeshComponent on Vehicle. GlassComponentName not naming one is
  /// not fatal -- the glass mesh is simply left unset.
  UFUNCTION(BlueprintCallable, Category = "Carla Tools|Vehicles")
  static bool ConfigureParkedVehicleAppearance(
      AActor *Vehicle,
      UStaticMesh *Mesh,
      FLinearColor Color,
      UStaticMesh *GlassMesh = nullptr,
      FName MeshComponentName = TEXT("StaticMesh"),
      FName GlassComponentName = TEXT("Glass"),
      FString BodyworkSlotSubstring = TEXT("Bodywork"),
      FName ColorParameterName = TEXT("Base Color"));
};
