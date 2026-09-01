// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB).
// This work is licensed under the terms of the MIT license. For a copy, see <https://opensource.org/licenses/MIT>.
//
// Compatibility shim for carla-digitaltwins' UBlueprintUtilFunctions (redirected here via
// CoreRedirects) so its building-generator blueprints load against stock CARLA.

#pragma once

#include <util/ue-header-guard-begin.h>
#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include <util/ue-header-guard-end.h>

#include "BlueprintUtilFunctions.generated.h"

UCLASS(BlueprintType)
class CARLATOOLS_API UBlueprintUtilFunctions : public UBlueprintFunctionLibrary
{
  GENERATED_BODY()

public:

  UFUNCTION(BlueprintPure)
  static UObject* CopyAssetToPlugin(UObject* SourceObject, FString PluginName);

  UFUNCTION(BlueprintCallable)
  static FString GetProjectName();
};
