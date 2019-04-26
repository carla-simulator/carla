// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "Carla.h"
#include "ServerGameController.h"

#include "Game/DataRouter.h"
#include "Server/CarlaServer.h"
#include "Settings/CarlaSettings.h"
#include "Private/RenderTargetTemp.h"

using Errc = FCarlaServer::ErrorCode;

static constexpr bool BLOCKING = true;
static constexpr bool NON_BLOCKING = false;

FServerGameController::FServerGameController(FDataRouter &InDataRouter)
  : ICarlaGameControllerBase(InDataRouter),
    Server(nullptr) {}

FServerGameController::~FServerGameController() {}

void FServerGameController::Initialize(UCarlaSettings &InCarlaSettings)
{
  CarlaSettings = &InCarlaSettings;

  // Initialize server if missing.
  if (!Server.IsValid()) {
    Server = MakeShared<FCarlaServer>(CarlaSettings->WorldPort, CarlaSettings->ServerTimeOut);
    FString IniFile;
    if ((Errc::Success == Server->Connect()) &&
        (Errc::Success == Server->ReadNewEpisode(IniFile, BLOCKING))) {
      CarlaSettings->LoadSettingsFromString(IniFile);
    } else {
      UE_LOG(LogCarlaServer, Warning, TEXT("Failed to initialize, server needs restart"));
      Server = nullptr;
    }
  }
}

APlayerStart *FServerGameController::ChoosePlayerStart(
    const TArray<APlayerStart *> &AvailableStartSpots)
{
  check(AvailableStartSpots.Num() > 0);
  // Send scene description.
  if (Server.IsValid()) {
    const auto &MapName = CarlaSettings->MapName;
    if (Errc::Success != Server->SendSceneDescription(MapName, AvailableStartSpots, BLOCKING)) {
      UE_LOG(LogCarlaServer, Warning, TEXT("Failed to send scene description, server needs restart"));
      Server = nullptr;
    }
  }

  // Read episode start.
  uint32 StartIndex = 0u; // default.
  if (Server.IsValid()) {
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

void FServerGameController::RegisterPlayer(AController &NewPlayer)
{

}

void FServerGameController::BeginPlay()
{
  if (Server.IsValid()) {
    if (Errc::Success != Server->SendEpisodeReady(BLOCKING)) {
      UE_LOG(LogCarlaServer, Warning, TEXT("Failed to read episode start, server needs restart"));
      Server = nullptr;
    }
  }
}

void FServerGameController::Tick(float DeltaSeconds)
{
  check(CarlaSettings != nullptr);

  if (!Server.IsValid()) {
    UE_LOG(LogCarlaServer, Warning, TEXT("Client disconnected, server needs restart"));
    RestartLevel();
    return;
  }

  // Check if the client requested a new episode.
  {
    FString IniFile;
    auto ec = Server->ReadNewEpisode(IniFile, NON_BLOCKING);
    switch (ec) {
      case Errc::Success:
        CarlaSettings->LoadSettingsFromString(IniFile);
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
    if (CarlaSettings->bSynchronousMode)
    {
      FlushRenderingCommands();
    }

    if (Errc::Error == Server->SendMeasurements(
            DataRouter.GetPlayerState(),
            DataRouter.GetAgents(),
            CarlaSettings->bSendNonPlayerAgentsInfo))
    {
      // The error here must be ignored, otherwise we can create a race
      // condition between the different ports.
      return;
    }
  }

  // Read control, block if the settings say so.
  {
    const bool bShouldBlock = CarlaSettings->bSynchronousMode;
    FVehicleControl Control;
    if (Errc::Error != Server->ReadControl(Control, bShouldBlock))
    {
      DataRouter.ApplyVehicleControl(Control, EVehicleInputPriority::Client);
    } // Here we ignore the error too.
  }
}

void FServerGameController::RestartLevel()
{
  UE_LOG(LogCarlaServer, Log, TEXT("Restarting the level..."));
  DataRouter.RestartLevel();
}
