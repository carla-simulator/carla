// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include <util/ue-header-guard-begin.h>
#include "Commandlets/Commandlet.h"
#include <util/ue-header-guard-end.h>

#include "DisableNaniteOnVegetationMeshesCommandlet.generated.h"

UCLASS()
class CARLA_API UDisableNaniteOnVegetationMeshesCommandlet
  : public UCommandlet
{
  GENERATED_BODY()

public:

  UDisableNaniteOnVegetationMeshesCommandlet();

#if WITH_EDITORONLY_DATA
  virtual int32 Main(const FString &Params) override;
#endif
};
