// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "Carla.h"
#include "MockGameController.h"

#include "Game/DataRouter.h"

MockGameController::MockGameController(
    FDataRouter &InDataRouter,
    const FMockGameControllerSettings &InSettings)
  : ICarlaGameControllerBase(InDataRouter),
    Settings(InSettings) {}

void MockGameController::Initialize(UCarlaSettings &CarlaSettings)
{
#if WITH_EDITOR
  if (Settings.bOverrideCarlaSettings) {
    CarlaSettings.NumberOfVehicles = Settings.NumberOfVehicles;
    CarlaSettings.NumberOfPedestrians = Settings.NumberOfPedestrians;
    CarlaSettings.WeatherId = Settings.WeatherId;
  }
#endif // WITH_EDITOR

  if (Settings.bChangeWeatherOnBeginPlay && (CarlaSettings.WeatherDescriptions.Num() > 0)) {
    static uint32 StaticIndex = 0u;
    CarlaSettings.WeatherId = StaticIndex % CarlaSettings.WeatherDescriptions.Num();
    ++StaticIndex;
  }
}

APlayerStart *MockGameController::ChoosePlayerStart(
    const TArray<APlayerStart *> &AvailableStartSpots)
{
  check(AvailableStartSpots.Num() > 0);
  const uint32 Index =
      (Settings.bRandomPlayerStart ?
          FMath::RandRange(0, AvailableStartSpots.Num() - 1) :
          Settings.PlayerStartIndex % AvailableStartSpots.Num());
  UE_LOG(LogCarla, Log, TEXT("Spawning player at player start %d/%d"), Index, AvailableStartSpots.Num());
  return AvailableStartSpots[Index];
}

void MockGameController::RegisterPlayer(AController &NewPlayer)
{
  ACarlaVehicleController *VehicleController = Cast<ACarlaVehicleController>(&NewPlayer);
  if (VehicleController != nullptr) {
    VehicleController->EnableUserInput(true);
  } else {
    UE_LOG(LogCarla, Warning, TEXT("Player is not a ACarlaVehicleController"));
  }
}

void MockGameController::BeginPlay()
{

}

void MockGameController::Tick(float /*DeltaSeconds*/)
{

}
