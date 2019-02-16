// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "Carla.h"
#include "Carla/Game/CarlaEngine.h"

#include "Carla/Game/CarlaEpisode.h"
#include "Carla/Settings/CarlaSettings.h"

#include <thread>

static uint32 GetNumberOfThreadsForRPCServer()
{
  return std::max(std::thread::hardware_concurrency(), 4u) - 2u;
}

FCarlaEngine::~FCarlaEngine()
{
  if (bIsRunning)
  {
    FWorldDelegates::OnWorldTickStart.Remove(OnPreTickHandle);
    FWorldDelegates::OnWorldPostActorTick.Remove(OnPostTickHandle);
  }
}

void FCarlaEngine::NotifyInitGame(const UCarlaSettings &Settings)
{
  if (!bIsRunning)
  {
    auto BroadcastStream = Server.Start(Settings.WorldPort);
    Server.AsyncRun(GetNumberOfThreadsForRPCServer());

    WorldObserver.SetStream(BroadcastStream);

    OnPreTickHandle = FWorldDelegates::OnWorldTickStart.AddRaw(this, &FCarlaEngine::OnPreTick);
    OnPostTickHandle = FWorldDelegates::OnWorldPostActorTick.AddRaw(this, &FCarlaEngine::OnPostTick);

    bIsRunning = true;
  }
}

void FCarlaEngine::NotifyBeginEpisode(UCarlaEpisode &Episode)
{
  CurrentEpisode = &Episode;
  Server.NotifyBeginEpisode(Episode);
}

void FCarlaEngine::NotifyEndEpisode()
{
  Server.NotifyEndEpisode();
  CurrentEpisode = nullptr;
}

void FCarlaEngine::OnPreTick(ELevelTick TickType, float DeltaSeconds)
{
  if ((TickType == ELevelTick::LEVELTICK_All) && (CurrentEpisode != nullptr))
  {
    CurrentEpisode->TickTimers(DeltaSeconds);
    WorldObserver.BroadcastTick(*CurrentEpisode);
  }
}

void FCarlaEngine::OnPostTick(UWorld *, ELevelTick, float)
{
  Server.RunSome(10u);
}
