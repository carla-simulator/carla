// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "Carla/Recorder/CarlaRecorder.h"
#include "Carla/Sensor/WorldObserver.h"
#include "Carla/Server/CarlaServer.h"
#include "Carla/Settings/EpisodeSettings.h"
#include "Carla/Util/NonCopyable.h"

#include "Misc/CoreDelegates.h"

class UCarlaSettings;
struct FEpisodeSettings;

class FCarlaEngine : private NonCopyable
{
public:

  static uint64_t FrameCounter;

  ~FCarlaEngine();

  void NotifyInitGame(const UCarlaSettings &Settings);

  void NotifyBeginEpisode(UCarlaEpisode &Episode);

  void NotifyEndEpisode();

  const FCarlaServer &GetServer() const
  {
    return Server;
  }

  UCarlaEpisode *GetCurrentEpisode()
  {
    return CurrentEpisode;
  }

  void SetRecorder(ACarlaRecorder *InRecorder)
  {
    Recorder = InRecorder;
  }

  static uint64_t GetFrameCounter()
  {
    return FCarlaEngine::FrameCounter;
  }

  static uint64_t UpdateFrameCounter()
  {
    FCarlaEngine::FrameCounter += 1;
    return FCarlaEngine::FrameCounter;
  }

  static void ResetFrameCounter(uint64_t Value = 0)
  {
    FCarlaEngine::FrameCounter = Value;
  }

private:

  void OnPreTick(UWorld *World, ELevelTick TickType, float DeltaSeconds);

  void OnPostTick(UWorld *World, ELevelTick TickType, float DeltaSeconds);

  void OnEpisodeSettingsChanged(const FEpisodeSettings &Settings);

  void ResetSimulationState();

  bool bIsRunning = false;

  bool bSynchronousMode = false;

  bool bMapChanged = false;

  FCarlaServer Server;

  FWorldObserver WorldObserver;

  UCarlaEpisode *CurrentEpisode = nullptr;

  FEpisodeSettings CurrentSettings;

  ACarlaRecorder *Recorder = nullptr;

  FDelegateHandle OnPreTickHandle;

  FDelegateHandle OnPostTickHandle;

  FDelegateHandle OnEpisodeSettingsChangeHandle;
};
