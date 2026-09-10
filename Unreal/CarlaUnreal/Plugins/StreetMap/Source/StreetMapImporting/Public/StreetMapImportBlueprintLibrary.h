// Copyright (c) 2023 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.
#pragma once

#include "CoreMinimal.h"
#include "EditorUtilityWidget.h"
#include "StreetMapImportBlueprintLibrary.generated.h"

/**
 * Import factory object for OpenStreetMap assets
 */
UCLASS()
class UStreetMapImportBlueprintLibrary : public UObject
{
	GENERATED_BODY()
public:

  UFUNCTION(BlueprintCallable, Category="EditorUtilityWidget")
  static UStreetMap* ImportStreetMap(FString Path, FString DestinationAssetPath, FVector2D OriginLatLon);
};

