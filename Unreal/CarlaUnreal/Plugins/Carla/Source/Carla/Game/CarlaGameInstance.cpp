// Copyright (c) 2024 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "Carla/Game/CarlaGameInstance.h"
#include "Carla.h"
#include "Carla/Settings/CarlaSettings.h"

UCarlaGameInstance::UCarlaGameInstance() {
  CarlaSettings = CreateDefaultSubobject<UCarlaSettings>(TEXT("CarlaSettings"));
  Recorder = CreateDefaultSubobject<ACarlaRecorder>(TEXT("Recorder"));
  CarlaEngine.SetRecorder(Recorder);

  check(CarlaSettings != nullptr);
  CarlaSettings->LoadSettings();
  CarlaSettings->LogSettings();
}

UCarlaGameInstance::~UCarlaGameInstance() = default;
