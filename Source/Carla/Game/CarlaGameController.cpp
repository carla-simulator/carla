// CARLA, Copyright (C) 2017 Computer Vision Center (CVC)

#include "Carla.h"
#include "CarlaGameController.h"

// #include "GameFramework/PlayerStart.h"

// #include "CarlaPlayerState.h"
#include "CarlaVehicleController.h"
// #include "SceneCaptureCamera.h"

#include "Settings/CarlaSettings.h"
#include "CarlaServer.h"

using Errc = CarlaServer::ErrorCode;

static constexpr bool BLOCKING = true;
static constexpr bool NON_BLOCKING = false;

CarlaGameController::CarlaGameController() :
  Server(nullptr),
  Player(nullptr) {}

CarlaGameController::~CarlaGameController() {}

void CarlaGameController::Initialize(UCarlaSettings &InCarlaSettings)
{
  CarlaSettings = &InCarlaSettings;

  bool bServerStartedSuccessfully = false;

  if (Server == nullptr) {
    Server = MakeUnique<CarlaServer>(CarlaSettings->WorldPort, CarlaSettings->ServerTimeOut);
    if (Errc::Success == Server->Connect()) {
      if (Errc::Success == Server->ReadNewEpisode(*CarlaSettings, BLOCKING)) {
        bServerStartedSuccessfully = true;
      }
    }
  }

  if (!bServerStartedSuccessfully) {
    UE_LOG(LogCarlaServer, Warning, TEXT("Failed to initialize, server needs restart"));
    Server = nullptr;
  }
}

APlayerStart *CarlaGameController::ChoosePlayerStart(
    const TArray<APlayerStart *> &AvailableStartSpots)
{
  // Send scene description.
  if (Server != nullptr) {
    if (Errc::Success != Server->SendSceneDescription(AvailableStartSpots, BLOCKING)) {
      UE_LOG(LogCarlaServer, Warning, TEXT("Failed to send scene description, server needs restart"));
      Server = nullptr;
    }
  }

  // Read episode start.
  uint32 StartIndex = 0u; // default.
  if (Server != nullptr) {
    if (Errc::Success != Server->ReadEpisodeStart(StartIndex, BLOCKING)) {
      UE_LOG(LogCarlaServer, Warning, TEXT("Failed to read episode start, server needs restart"));
      Server = nullptr;
    }
  }

  return AvailableStartSpots[StartIndex];
}

void CarlaGameController::RegisterPlayer(AController &NewPlayer)
{
  Player = Cast<ACarlaVehicleController>(&NewPlayer);
  check(Player != nullptr);
}

void CarlaGameController::BeginPlay()
{
  check(Player != nullptr);
  GameState = Cast<ACarlaGameState>(Player->GetWorld()->GetGameState());
  check(GameState != nullptr);
  if (Server != nullptr) {
    if (Errc::Success != Server->SendEpisodeReady(BLOCKING)) {
      UE_LOG(LogCarlaServer, Warning, TEXT("Failed to read episode start, server needs restart"));
      Server = nullptr;
    }
  }
}

void CarlaGameController::Tick(float DeltaSeconds)
{
  check(Player != nullptr);
  check(CarlaSettings != nullptr);

  if (Server == nullptr) {
    UE_LOG(LogCarlaServer, Warning, TEXT("Client disconnected, server needs restart"));
    RestartLevel();
  }

  // Check if the client requested a new episode.
  if (Server != nullptr) {
    auto ec = Server->ReadNewEpisode(*CarlaSettings, NON_BLOCKING);
    switch (ec) {
      case Errc::Success:
        RestartLevel();
        break;
      case Errc::Error:
        Server = nullptr;
        break;
    }
  }

  // Send measurements.
  if (Server != nullptr) {
    check(GameState != nullptr);
    if (Errc::Error == Server->SendMeasurements(
            *GameState,
            Player->GetPlayerState(),
            CarlaSettings->bSendNonPlayerAgentsInfo)) {
      Server = nullptr;
    }
  }

  // Read control, block if the settings say so.
  if (Server != nullptr) {
    const bool bShouldBlock = CarlaSettings->bSynchronousMode;
    if (Errc::Error == Server->ReadControl(*Player, bShouldBlock)) {
      Server = nullptr;
    }
  }
}

void CarlaGameController::RestartLevel()
{
  UE_LOG(LogCarlaServer, Log, TEXT("Restarting the level..."));
  Player->RestartLevel();
}
