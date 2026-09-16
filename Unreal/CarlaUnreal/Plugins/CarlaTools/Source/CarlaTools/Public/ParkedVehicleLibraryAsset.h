// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include <util/ue-header-guard-begin.h>
#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Carla/Actor/ActorDefinition.h"
#include <util/ue-header-guard-end.h>

#include "ParkedVehicleLibraryAsset.generated.h"

/// Editable content asset holding the pool of parked-vehicle definitions
/// (mesh + available colors per entry) that WD_SpawnActors' ParkedVehicles
/// function picks from. Create one instance per style/map in the Content
/// Browser and reference it from a WD_SpawnActors variable of this type,
/// instead of keeping the array as a Class Defaults property on the widget
/// Blueprint itself.
///
/// This isn't just for consistency with UDecalLibraryAsset: editing a
/// TArray<FColor> nested inside a TArray<FVehicleActorDefinition> directly
/// on a Blueprint's Class Defaults has crashed the editor (reproducibly,
/// losing unsaved entries both times) -- Blueprint property edits can
/// trigger a recompile mid-edit, and if that reallocates the CDO's array
/// while the color picker still holds a pointer into the old one, the
/// picker writes to freed memory. A plain UDataAsset's properties don't
/// go through Blueprint recompilation on edit, which removes that trigger.
UCLASS(BlueprintType)
class CARLATOOLS_API UParkedVehicleLibraryAsset : public UPrimaryDataAsset
{
  GENERATED_BODY()

public:

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Carla Tools|Vehicles")
  TArray<FVehicleActorDefinition> ParkedVehicles;
};
