// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "Carla/Autoware/Data/MgrsDataAsset.h"

#include <util/ue-header-guard-begin.h>
#include "CoreMinimal.h"
#include "GameFramework/WorldSettings.h"
#include <util/ue-header-guard-end.h>

#include "AutowareWorldSettings.generated.h"

/// World settings holding the per-map MGRS data asset reference used by the
/// Autoware flow (AutowareGameModeBase georeference + AutowareGnssSensor MGRS
/// offset). Ported from tier4/autoware-support.
UCLASS()
class CARLA_API AAutowareWorldSettings : public AWorldSettings
{
  GENERATED_BODY()

public:

  UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "MGRS", meta = (DisplayName = "Mgrs Data Asset"))
  TSoftObjectPtr<UMgrsDataAsset> MgrsDataAssetSoftPtr;
};
