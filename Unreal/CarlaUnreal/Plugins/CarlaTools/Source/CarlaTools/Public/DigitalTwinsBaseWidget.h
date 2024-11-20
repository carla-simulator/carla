// Copyright (c) 2024 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include <util/ue-header-guard-begin.h>
#include "CoreMinimal.h"
#include "EditorUtilityWidget.h"
#include "CarlaTools.h"
#include <util/ue-header-guard-end.h>

#include "DigitalTwinsBaseWidget.generated.h"

class UOpenDriveToMap;

UCLASS(BlueprintType)
class CARLATOOLS_API UDigitalTwinsBaseWidget : public UEditorUtilityWidget
{
  GENERATED_BODY()
public:

  UFUNCTION(BlueprintCallable)
  UOpenDriveToMap* InitializeOpenDriveToMap(TSubclassOf<UOpenDriveToMap> BaseClass);

  UFUNCTION(BlueprintPure)
  UOpenDriveToMap* GetOpenDriveToMap();

  UFUNCTION(BlueprintCallable)
  void SetOpenDriveToMap(UOpenDriveToMap* ToSet);

  UFUNCTION(BlueprintCallable)
  void DestroyOpenDriveToMap();
};
