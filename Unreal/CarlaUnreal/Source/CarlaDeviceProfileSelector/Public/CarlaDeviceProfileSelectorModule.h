// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"
#include "IDeviceProfileSelectorModule.h"

// CARLA DeviceProfileSelectorModule. Loaded by UDeviceProfileManager during
// engine init when [DeviceProfileManager] DeviceProfileSelectorModule= names
// this module in DefaultEngine.ini. Translates the CARLA-specific
// `-quality-level=<Tier>` command-line flag into a CarlaQuality_<Tier>
// DeviceProfile selection so per-tier CVars apply at engine init (no runtime
// CVar burst, no Xid 109 risk).
class FCarlaDeviceProfileSelectorModule : public IDeviceProfileSelectorModule
{
public:
  virtual void StartupModule() override;
  virtual void ShutdownModule() override;

  // IDeviceProfileSelectorModule
  virtual const FString GetRuntimeDeviceProfileName() override;
};
