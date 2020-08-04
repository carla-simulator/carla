// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "Carla/Game/CarlaEpisode.h"
#include "Carla/Game/CarlaGameInstance.h"
#include "Carla/Game/CarlaGameModeBase.h"
#include "Carla/Settings/CarlaSettings.h"

#include "Kismet/GameplayStatics.h"
#include "Kismet/BlueprintFunctionLibrary.h"

#include "CarlaStatics.generated.h"

// =============================================================================
// -- UCarlaStatics declaration ------------------------------------------------
// =============================================================================

UCLASS()
class CARLA_API UCarlaStatics : public UBlueprintFunctionLibrary
{
  GENERATED_BODY()

public:

  UFUNCTION(BlueprintPure, Category="CARLA", meta=(WorldContext="WorldContextObject"))
  static ACarlaGameModeBase *GetGameMode(const UObject *WorldContextObject);

  UFUNCTION(BlueprintPure, Category="CARLA", meta=(WorldContext="WorldContextObject"))
  static UCarlaGameInstance *GetGameInstance(const UObject *WorldContextObject);

  UFUNCTION(BlueprintPure, Category="CARLA", meta=(WorldContext="WorldContextObject"))
  static UCarlaEpisode *GetCurrentEpisode(const UObject *WorldContextObject);

  UFUNCTION(BlueprintPure, Category="CARLA", meta=(WorldContext="WorldContextObject"))
  static UCarlaSettings *GetCarlaSettings(const UObject *WorldContextObject);

  UFUNCTION(BlueprintPure, Category="CARLA")
  static TArray<FString> GetAllMapNames();

  UFUNCTION(BlueprintPure, Category="CARLA", meta=(WorldContext="WorldContextObject"))
  static ACarlaRecorder* GetRecorder(const UObject *WorldContextObject);

  static CarlaReplayer* GetReplayer(const UObject *WorldContextObject);
};

// =============================================================================
// -- UCarlaStatics implementation ---------------------------------------------
// =============================================================================

inline ACarlaGameModeBase *UCarlaStatics::GetGameMode(const UObject *WorldContext)
{
  return Cast<ACarlaGameModeBase>(UGameplayStatics::GetGameMode(WorldContext));
}

inline UCarlaGameInstance *UCarlaStatics::GetGameInstance(const UObject *WorldContext)
{
  return Cast<UCarlaGameInstance>(UGameplayStatics::GetGameInstance(WorldContext));
}

inline UCarlaEpisode *UCarlaStatics::GetCurrentEpisode(const UObject *WorldContext)
{
  auto GameInstance = GetGameInstance(WorldContext);
  return GameInstance != nullptr ? GameInstance->GetCarlaEpisode() : nullptr;
}

inline UCarlaSettings *UCarlaStatics::GetCarlaSettings(const UObject *WorldContext)
{
  auto GameInstance = GetGameInstance(WorldContext);
  return GameInstance != nullptr ? GameInstance->GetCARLASettings() : nullptr;
}

inline ACarlaRecorder* UCarlaStatics::GetRecorder(const UObject *WorldContextObject)
{
  auto* Episode = UCarlaStatics::GetCurrentEpisode(WorldContextObject);
  if (Episode)
  {
    return Episode->GetRecorder();
  }
  return nullptr;
}

inline CarlaReplayer* UCarlaStatics::GetReplayer(const UObject *WorldContextObject)
{
  auto* Episode = UCarlaStatics::GetCurrentEpisode(WorldContextObject);
  if (Episode)
  {
    return Episode->GetReplayer();
  }
  return nullptr;
}
