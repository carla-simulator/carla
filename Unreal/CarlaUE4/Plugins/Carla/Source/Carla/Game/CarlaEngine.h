// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "Carla/Sensor/WorldObserver.h"
#include "Carla/Server/TheNewCarlaServer.h"
#include "Carla/Util/NonCopyable.h"

#include "Misc/CoreDelegates.h"

class UCarlaSettings;

class FCarlaEngine : private NonCopyable
{
public:

  ~FCarlaEngine();

  void NotifyInitGame(const UCarlaSettings &Settings);

  void NotifyBeginEpisode(UCarlaEpisode &Episode);

  void NotifyEndEpisode();

  const FTheNewCarlaServer &GetServer() const
  {
    return Server;
  }

  UCarlaEpisode *GetCurrentEpisode()
  {
    return CurrentEpisode;
  }

private:

  void OnPreTick(ELevelTick TickType, float DeltaSeconds);

  void OnPostTick(UWorld *World, ELevelTick TickType, float DeltaSeconds);

  void OnPostTickSync(UWorld *World, ELevelTick TickType, float DeltaSeconds);

  bool bIsRunning = false;

  FTheNewCarlaServer Server;

  FWorldObserver WorldObserver;

  UCarlaEpisode *CurrentEpisode = nullptr;

  FDelegateHandle OnPreTickHandle;

  FDelegateHandle OnPostTickHandle;
};
