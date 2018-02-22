// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "Carla.h"
#include "CarlaGameController.h"

#include "Server/CarlaServer.h"
#include "Settings/CarlaSettings.h"
#include "Vehicle/CarlaVehicleController.h"

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

  // Initialize server if missing.
  if (Server == nullptr) {
    Server = MakeUnique<CarlaServer>(CarlaSettings->WorldPort, CarlaSettings->ServerTimeOut);
    if ((Errc::Success != Server->Connect()) ||
        (Errc::Success != Server->ReadNewEpisode(*CarlaSettings, BLOCKING))) {
      UE_LOG(LogCarlaServer, Warning, TEXT("Failed to initialize, server needs restart"));
      Server = nullptr;
    }
  }
}

APlayerStart *CarlaGameController::ChoosePlayerStart(
    const TArray<APlayerStart *> &AvailableStartSpots)
{
  check(AvailableStartSpots.Num() > 0);
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

  if (static_cast<int64>(StartIndex) >= AvailableStartSpots.Num()) {
    UE_LOG(LogCarlaServer, Warning, TEXT("Client requested an invalid player start, using default one instead."));
    StartIndex = 0u;
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
    return;
  }

  // Check if the client requested a new episode.
  {
    auto ec = Server->ReadNewEpisode(*CarlaSettings, NON_BLOCKING);
    switch (ec) {
      case Errc::Success:
        RestartLevel();
        return;
      case Errc::Error:
        Server = nullptr;
        return;
      default:
        break; // fallthrough...
    }
  }

  // Send measurements.
  {
    check(GameState != nullptr);
    if (Errc::Error == Server->SendMeasurements(
            *GameState,
            Player->GetPlayerState(),
            CarlaSettings->bSendNonPlayerAgentsInfo)) {
      Server = nullptr;
      return;
    }
  }

  // Read control, block if the settings say so.
  {
    const bool bShouldBlock = CarlaSettings->bSynchronousMode;
    if (Errc::Error == Server->ReadControl(*Player, bShouldBlock)) {
      Server = nullptr;
      return;
    }
  }
}

void CarlaGameController::RestartLevel()
{
  UE_LOG(LogCarlaServer, Log, TEXT("Restarting the level..."));
  Player->RestartLevel();
}
