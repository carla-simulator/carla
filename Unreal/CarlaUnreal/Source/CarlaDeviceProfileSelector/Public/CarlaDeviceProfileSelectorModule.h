// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

// Carla per-quality tier module. Loaded at LoadingPhase=PostConfigInit per
// CarlaUnreal.uproject. Reads `-quality-level=<Tier>` from the command line,
// resolves it case-sensitively against `Low / Medium / High / Epic`, falls
// back to `Epic` when absent, selects the matching DefaultScalability.ini
// bucket for every scalability group, and lowers the few CVars that
// DefaultEngine.ini pins above scalability priority at
// ECVF_SetByDeviceProfile. A FCoreDelegates::OnPostEngineInit
// callback re-applies the tier scalability levels after
// UGameUserSettings::ApplyNonResolutionSettings, so persisted
// GameUserSettings.ini state cannot shadow the tier-selected buckets.
class FCarlaDeviceProfileSelectorModule : public IModuleInterface {
public:

  virtual void StartupModule() override;
  virtual void ShutdownModule() override;
};
