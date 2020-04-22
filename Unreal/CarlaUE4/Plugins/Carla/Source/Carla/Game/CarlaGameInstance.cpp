// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "Carla.h"
#include "Carla/Game/CarlaGameInstance.h"

#include "Carla/Settings/CarlaSettings.h"

UCarlaGameInstance::UCarlaGameInstance() {
  CarlaSettings = CreateDefaultSubobject<UCarlaSettings>(TEXT("CarlaSettings"));
  check(CarlaSettings != nullptr);
  CarlaSettings->LoadSettings();
  CarlaSettings->LogSettings();
}

UCarlaGameInstance::~UCarlaGameInstance() = default;

void UCarlaGameInstance::SetMapToLoad(const FString MapName)
{
  MapToLoad = MapName;
  bShouldLoadLevel = true;
}

void UCarlaGameInstance::CheckAndLoadMap(UWorld *world, UCarlaEpisode &Episode)
{
  if(bShouldLoadLevel)
  {
    Episode.LoadNewEpisode(MapToLoad);
    bShouldLoadLevel = false;
  }
}
