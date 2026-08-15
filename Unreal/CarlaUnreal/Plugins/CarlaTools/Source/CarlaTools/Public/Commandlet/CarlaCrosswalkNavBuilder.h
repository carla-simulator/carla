// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include <util/ue-header-guard-begin.h>
#include "CoreMinimal.h"
#include "WorldPartition/WorldPartitionBuilder.h"
#include <util/ue-header-guard-end.h>

#include "CarlaCrosswalkNavBuilder.generated.h"

/// World Partition builder that reads the map's OpenDRIVE file and generates
/// one ACarlaNavModifierBox external actor per crosswalk polygon, stamping
/// the UNavAreaCarlaCrosswalk nav area. Rerunnable: previously generated
/// boxes are deleted first. Run BEFORE WorldPartitionNavigationDataBuilder so
/// the areas land in the rebuilt navmesh chunks:
///
///   UnrealEditor-Cmd <uproject> <map> -run=WorldPartitionBuilderCommandlet \
///       -Builder=CarlaCrosswalkNavBuilder -SCCProvider=None -nullrhi \
///       -unattended
UCLASS()
class CARLATOOLS_API UCarlaCrosswalkNavBuilder : public UWorldPartitionBuilder
{
  GENERATED_BODY()

public:

  UCarlaCrosswalkNavBuilder(const FObjectInitializer &ObjectInitializer)
    : Super(ObjectInitializer) {}

  virtual bool RequiresCommandletRendering() const override
  {
    return false;
  }

  virtual ELoadingMode GetLoadingMode() const override
  {
    // The boxes are placed from OpenDRIVE coordinates; no cell content needs
    // to be loaded.
    return ELoadingMode::Custom;
  }

protected:

  virtual bool RunInternal(
      UWorld *World,
      const FCellInfo &InCellInfo,
      FPackageSourceControlHelper &PackageHelper) override;
};
