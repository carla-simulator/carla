// CARLA, Copyright (C) 2017 Computer Vision Center (CVC)

#include "Carla.h"
#include "MockGameController.h"

MockGameController::MockGameController(const FMockGameControllerSettings &InSettings) :
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

#if WITH_EDITOR
  if (Settings.bForceEnableSemanticSegmentation) {
    CarlaSettings.bSemanticSegmentationEnabled = true;
  }
#endif // WITH_EDITOR
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
