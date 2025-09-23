// Copyright (c) 2023 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

// Engine headers
#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "BlueprintUtilFunctions.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogDigitalTwinsToolBlueprintUtil, Log, All);

UCLASS(BlueprintType)
class CARLATOOLS_API UBlueprintUtilFunctions : public UBlueprintFunctionLibrary
{
  GENERATED_BODY()

public:

  UFUNCTION(BlueprintPure)
  static UObject* CopyAssetToPlugin(UObject* SourceObject, FString PluginName);

  UFUNCTION(BlueprintCallable)
  static void UnloadRegionWorldPartition(
      UWorld* World,
      const FBox& RegionBounds,
      const FString& RegionName);

};
