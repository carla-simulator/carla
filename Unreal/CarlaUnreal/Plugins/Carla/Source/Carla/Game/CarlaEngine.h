// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma
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

#include <util/disable-ue4-macros.h>
#include <carla/multigpu/router.h>
#include <carla/multigpu/primaryCommands.h>
#include <carla/multigpu/secondary.h>
#include <carla/multigpu/secondaryCommands.h>
#include <carla/multigpu/sensorStreamRegistry.h>
#if WITH_ROS2
    #include <carla/ros2/ROS2.h>
#endif
#include <util/enable-ue4-macros.h>

#include <util/ue-header-guard-begin.h>
#include "Misc/CoreDelegates.h"
#include <util/ue-header-guard-end.h>

#include <atomic>
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

  /// Secondary only: binds a sensor replayed from the primary to the stream
  /// its clients were given (see carla::multigpu::SensorStreamRegistry).
  void BindReplayedSensor(uint32_t PrimaryActorId, uint32_t LocalActorId, bool bCreated);

  /// Primary only: keeps processing RPC until every secondary sent LOAD_MAP
  /// reports its episode ready (bounded by a timeout).
  void WaitForSecondaryEpisodes();

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

  // Set from the router's io-context thread (ConnectSession/HandleResponse
  // callbacks); consumed and cleared from the game thread in OnPostTick.
  std::atomic<bool> bNewConnection{false};

  std::unordered_map<uint32_t, uint32_t> MappedId;

  // Secondary only. Resolved and reset from the multi-GPU command thread
  // (GET_TOKEN, LOAD_MAP), bound from the game thread; internally synchronized.
  carla::multigpu::SensorStreamRegistry SensorStreams;

  std::shared_ptr<carla::multigpu::Router>    SecondaryServer;
  std::shared_ptr<carla::multigpu::Secondary> Secondary;

  std::vector<FFrameData> FramesToProcess;
  std::mutex FrameToProcessMutex;

  // Tracks whether this secondary is currently considered behind the
  // primary's SEND_FRAME cadence, so the backlog warning logs on state
  // change only rather than once per queued frame.
  bool bFramesToProcessBacklogged = false;

  // Game thread only: BindReplayedSensor warns once per opened episode.
  bool bWarnedSensorBindRefused = false;

  FString PendingLoadMap;
  std::atomic<bool> bLoadMapPending{false};

  // SensorStreams epoch of PendingLoadMap (guarded by FrameToProcessMutex) and
  // of the map load the game thread started last (game thread only).
  carla::multigpu::SensorStreamRegistry::epoch_type PendingLoadEpoch = 0u;
  carla::multigpu::SensorStreamRegistry::epoch_type LoadingEpoch = 0u;

  // Primary's id of PendingLoadMap and of the load started last, echoed in
  // the episode-ready message; same guards as the epochs above.
  carla::multigpu::load_map_id_type PendingLoadMapId = 0u;
  carla::multigpu::load_map_id_type LoadingLoadMapId = 0u;
};

// Note: this has a circular dependency with FCarlaEngine; it must be included late.
#include "Sensor/AsyncDataStreamImpl.h"
