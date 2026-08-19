// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "Carla/Game/CarlaGameModeBase.h"

#include <util/ue-header-guard-begin.h>
#include "CoreMinimal.h"
#include <util/ue-header-guard-end.h>

#include "AutowareGameModeBase.generated.h"

/// Game mode for Autoware maps: when the world settings carry an MGRS data
/// asset, the episode georeference is taken from it instead of the map's
/// OpenDRIVE header. Ported from tier4/autoware-support.
UCLASS()
class CARLA_API AAutowareGameModeBase : public ACarlaGameModeBase
{
  GENERATED_BODY()

public:

  AAutowareGameModeBase(const FObjectInitializer& ObjectInitializer);

protected:

  void InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage) override;

  virtual void LoadGeoReference() override;
};
