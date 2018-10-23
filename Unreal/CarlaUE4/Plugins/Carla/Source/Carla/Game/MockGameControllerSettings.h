// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "MockGameControllerSettings.generated.h"

USTRUCT(BlueprintType)
struct CARLA_API FMockGameControllerSettings
{
  GENERATED_USTRUCT_BODY()

  /** If true, weather will be changed every time we start the level.
    *
    * Has precedence over options in "Override CARLA Settings".
    */
  UPROPERTY(EditAnywhere, Category = "Mock CARLA Controller")
  bool bChangeWeatherOnBeginPlay = false;

  /** If true, a random player start position will be chosen every time we start the level. */
  UPROPERTY(EditAnywhere, Category = "Mock CARLA Controller")
  bool bRandomPlayerStart = false;

  /** Index of the player start position. */
  UPROPERTY(EditAnywhere, Category = "Mock CARLA Controller", meta = (EditCondition = "!bRandomPlayerStart", ClampMin = 0))
  int32 PlayerStartIndex = 0;

#if WITH_EDITORONLY_DATA

  /** Override available settings in CARLA Settings (Editor only). */
  UPROPERTY(EditAnywhere, Category = "Mock CARLA Controller", meta = (DisplayName = "Override CARLA Settings"))
  bool bOverrideCarlaSettings = false;

  /** Number of NPC vehicles to be spawned into the level. */
  UPROPERTY(EditAnywhere, Category = "Mock CARLA Controller", meta = (EditCondition = "bOverrideCarlaSettings", ClampMin = 0))
  int32 NumberOfVehicles = 5;

  /** Number of NPC pedestrians to be spawned into the level. */
  UPROPERTY(EditAnywhere, Category = "Mock CARLA Controller", meta = (EditCondition = "bOverrideCarlaSettings", ClampMin = 0))
  int32 NumberOfPedestrians = 15;

  /** Index of the weather setting to use. If negative, weather won't be changed. */
  UPROPERTY(EditAnywhere, Category = "Mock CARLA Controller", meta = (EditCondition = "bOverrideCarlaSettings"))
  int32 WeatherId = -1;

#endif // WITH_EDITORONLY_DATA
};
