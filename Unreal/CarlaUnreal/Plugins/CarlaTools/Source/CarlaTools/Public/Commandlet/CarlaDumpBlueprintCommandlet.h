// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include <util/ue-header-guard-begin.h>
#include "Commandlets/Commandlet.h"
#include <util/ue-header-guard-end.h>

#include "CarlaDumpBlueprintCommandlet.generated.h"

/// Dumps every graph of a Blueprint asset (nodes, pins, default values and
/// links) to the log so the logic can be inspected without opening the editor.
///
/// Usage:
///   UnrealEditor-Cmd <project> -run=CarlaDumpBlueprint <ObjectPath> -unattended -nullrhi
///   e.g. -run=CarlaDumpBlueprint /Game/Carla/Blueprints/Weather/BP_CarlaWeather
UCLASS()
class CARLATOOLS_API UCarlaDumpBlueprintCommandlet : public UCommandlet
{
  GENERATED_BODY()

public:

  virtual int32 Main(const FString& Params) override;
};
