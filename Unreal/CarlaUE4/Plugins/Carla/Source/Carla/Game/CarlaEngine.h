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
#include "Carla/Game/FrameData.h"

#include "Misc/CoreDelegates.h"

#include <compiler/disable-ue4-macros.h>
#include <carla/multigpu/router.h>
#include <carla/multigpu/primaryCommands.h>
#include <carla/multigpu/secondary.h>
#include <carla/multigpu/secondaryCommands.h>
#include <carla/ros2/ROS2.h>
#include <compiler/enable-ue4-macros.h>

#include <mutex>

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

  FCarlaServer &GetServer()
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
    #if defined(WITH_ROS2)
    auto ROS2 = carla::ros2::ROS2::GetInstance();
    if (ROS2->IsEnabled())
      ROS2->SetFrame(FCarlaEngine::FrameCounter);
    #endif
    return FCarlaEngine::FrameCounter;
  }

  static void ResetFrameCounter(uint64_t Value = 0)
  {
    FCarlaEngine::FrameCounter = Value;
    #if defined(WITH_ROS2)
    auto ROS2 = carla::ros2::ROS2::GetInstance();
    if (ROS2->IsEnabled())
      ROS2->SetFrame(FCarlaEngine::FrameCounter);
    #endif
  }

  std::shared_ptr<carla::multigpu::Router> GetSecondaryServer()
  {
    return SecondaryServer;
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

  bool bIsPrimaryServer = true;
  bool bNewConnection = false;

  std::unordered_map<uint32_t, uint32_t> MappedId;

  std::shared_ptr<carla::multigpu::Router>    SecondaryServer;
  std::shared_ptr<carla::multigpu::Secondary> Secondary;

  std::vector<FFrameData> FramesToProcess;
  std::mutex FrameToProcessMutex;
};

// Note: this has a circular dependency with FCarlaEngine; it must be included late.
#include "Sensor/AsyncDataStreamImpl.h"
