// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include <util/ue-header-guard-begin.h>
#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "BlueprintLibrary/DecalSpawnLibrary.h"
#include <util/ue-header-guard-end.h>

#include "DecalLibraryAsset.generated.h"

/// Editable content asset holding the pool of decals (and their spawn
/// ranges/weights) that UDecalSpawnLibrary::ReplaceActorsWithRandomDecals
/// picks from. Create one instance per style/map in the Content Browser and
/// reference it from a WD_SpawnActors variable of this type, instead of
/// keeping the array as loose Blueprint struct data.
UCLASS(BlueprintType)
class CARLATOOLS_API UDecalLibraryAsset : public UPrimaryDataAsset
{
  GENERATED_BODY()

public:

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Carla Tools|Decals")
  TArray<FDecalSpawnParams> DecalLibrary;
};
