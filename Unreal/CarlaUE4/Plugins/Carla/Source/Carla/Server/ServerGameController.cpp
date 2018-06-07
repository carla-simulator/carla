// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "Carla.h"
#include "ServerGameController.h"

#include "Game/DataRouter.h"
#include "Server/CarlaServer.h"
#include "Server/ServerSensorDataSink.h"
#include "Settings/CarlaSettings.h"
#include "Private/RenderTargetTemp.h"
#include "Regex.h"
#include "FileManager.h"
#include "Kismet/GameplayStatics.h"
using Errc = FCarlaServer::ErrorCode;

static constexpr bool BLOCKING = true;
static constexpr bool NON_BLOCKING = false;

FServerGameController::FServerGameController(FDataRouter &InDataRouter)
  : ICarlaGameControllerBase(InDataRouter),
    DataSink(MakeShared<FServerSensorDataSink>()),
    Server(nullptr) {
  DataRouter.SetSensorDataSink(DataSink);
}

FServerGameController::~FServerGameController() {}

void FServerGameController::Initialize(UCarlaSettings &InCarlaSettings)
{
  CarlaSettings = &InCarlaSettings;

  // Initialize server if missing.
  if (!Server.IsValid()) {
    Server = MakeShared<FCarlaServer>(CarlaSettings->WorldPort, CarlaSettings->ServerTimeOut);
    DataSink->SetServer(Server);
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
    TArray<USensorDescription *> Sensors;
    CarlaSettings->SensorDescriptions.GenerateValueArray(Sensors);
    const auto &MapName = CarlaSettings->MapName;
    if (Errc::Success != Server->SendSceneDescription(MapName, AvailableStartSpots, Sensors, BLOCKING)) {
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
    switch (ec) 
    {
      case Errc::Success:
        CarlaSettings->LoadSettingsFromString(IniFile);
        if (IsTheSameLevel())
        {
          RestartLevel();
        } else if (MapExists(CarlaSettings->MapName)) 
        {  
          ChangeLevel(CarlaSettings->GetWorld(), CarlaSettings->MapName);
        } else
        {
          UE_LOG(LogCarla, Error, TEXT("The map %s could not be found in the Maps directory, check it out!"), *CarlaSettings->MapName);
        }
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
      DataRouter.ApplyVehicleControl(Control);
    } // Here we ignore the error too.
  }
}

bool FServerGameController::MapExists(const FString& mapname,bool refreshmapfiles)
{
  static TArray<FString> MapFiles;
  if (refreshmapfiles || MapFiles.Num() == 0)
  {
    IFileManager::Get().FindFilesRecursive(MapFiles, *FPaths::Combine(*FPaths::ProjectContentDir(), TEXT("Maps")), TEXT("*.umap"), true, false, false);
  }
  const FRegexPattern MapPattern(*FString::Printf(TEXT("%s\\.umap"), *mapname));
  return (MapFiles.ContainsByPredicate([&](const FString& _mapname) {return FRegexMatcher(MapPattern, _mapname).FindNext(); }));
}

bool FServerGameController::IsTheSameLevel()
{
  UWorld* world = CarlaSettings->GetWorld();
  check(world);
  const FString CurrentMapName = world->GetMapName().Mid(world->StreamingLevelsPrefix.Len());
  const FString LoadMapName = CarlaSettings->MapName.Mid(world->StreamingLevelsPrefix.Len());
  if (CarlaSettings->MapName.IsEmpty() || (!LoadMapName.IsEmpty() && CurrentMapName != LoadMapName))
  {
      CarlaSettings->MapName = LoadMapName;
  }
  return (CurrentMapName.Equals(CarlaSettings->MapName));
}

void FServerGameController::ChangeLevel(UWorld* world,const FString& mapname)
{
  UGameplayStatics::OpenLevel(world, FName(*mapname), true, FString(L"?initCARLA=true"));
}

void FServerGameController::RestartLevel()
{
  UE_LOG(LogCarlaServer, Log, TEXT("Restarting the level..."));
  DataRouter.RestartLevel();
}
