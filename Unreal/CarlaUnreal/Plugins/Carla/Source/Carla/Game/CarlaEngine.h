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
#include "Engine/World.h"
#include <util/ue-header-guard-end.h>

#include <atomic>
#include <mutex>
#include <optional>

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

  /// False on a Multi-GPU secondary. Known from NotifyInitGame on, before any
  /// actor of the first map begins play.
  bool IsPrimaryServer() const
  {
    return bIsPrimaryServer;
  }

private:

  void OnPreTick(UWorld *World, ELevelTick TickType, float DeltaSeconds);

  void OnPostTick(UWorld *World, ELevelTick TickType, float DeltaSeconds);

  void OnEpisodeSettingsChanged(const FEpisodeSettings &Settings);

  /// Applies FEpisodeSettings::bDeterministicCollisions to the world about to
  /// be created.
  void OnPreWorldInitialization(UWorld *World, const UWorld::InitializationValues IVS);

  void ResetSimulationState();

  /// Secondary only: binds a sensor replayed from the primary to the stream
  /// its clients were given (see carla::multigpu::SensorStreamRegistry).
  void BindReplayedSensor(uint32_t PrimaryActorId, uint32_t LocalActorId, bool bCreated);

  /// Primary only: keeps processing RPC until every secondary sent LOAD_MAP
  /// reports its episode ready (bounded by a timeout).
  void WaitForSecondaryEpisodes();

  /// Secondary only (FCoreDelegates::OnBeginFrame, before the engine computes
  /// this frame's delta): waits for the primary's next frame and makes its
  /// delta this engine frame's time step.
  void OnBeginFrame();

  /// Secondary only: plays the primary's frame on the frame OnBeginFrame
  /// prepared, with the primary's clocks and settings.
  void PlayPrimaryFrame(UWorld &World, float DeltaSeconds, FFrameData &Frame);

  void MirrorPrimarySettings(const carla::multigpu::FrameTime &Time);

  /// Secondary only: tells the primary this episode is ready, which also
  /// makes it send a full resync.
  void SendEpisodeReady();

  /// Sends EPISODE_READY once the world's generated content has finished spawning.
  void SendEpisodeReadyWhenContentReady();

  /// Secondary only: stops mirroring the primary for good (it is gone, or
  /// this server cannot load its map). Never call it holding FrameToProcessMutex.
  void DetachFromPrimary(const char *Reason);

  /// Secondary only: puts back any mirror that moved since its frame was
  /// played (a local mover the mirror does not stop), right before capture.
  void RestorePlayedPoses();

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

  FDelegateHandle OnBeginFrameHandle;

  FDelegateHandle OnTravelFailureHandle;

  // Primary, game thread: the world's clocks recorded in OnPreTick of the
  // current frame, before UWorld::Tick accumulates them; sent in OnPostTick.
  struct FPendingFrameTime
  {
    float DeltaSeconds = 0.0f;
    double TimeSeconds = 0.0;
    double UnpausedTimeSeconds = 0.0;
    double RealTimeSeconds = 0.0;
    double AudioTimeSeconds = 0.0;
    float TimeDilation = 1.0f;
  };
  std::optional<FPendingFrameTime> PendingFrameTime;

  // Secondary, game thread: the primary frame OnBeginFrame prepared this
  // engine frame for, the last one played, and whether this engine frame
  // played one (only then does it capture and broadcast).
  std::optional<uint64_t> PreparedFrame;
  std::optional<uint64_t> LastPlayedFrame;
  bool bFramePlayedThisTick = false;
  bool bWasConnectedToPrimary = false;
  bool bWarnedUntimedFrame = false;
  bool bWarnedUnpairedFrame = false;
  bool bWarnedNoSensorTicks = false;
  uint64_t DeltaMismatches = 0u;
  double LastOwnClockFrameSeconds = 0.0;
  double LastResyncRequestSeconds = 0.0;

  // Secondary: set from the game thread or the multi-GPU command thread when
  // this server misses primary state; OnBeginFrame then asks for a resync.
  std::atomic<bool> bResyncRequested{false};

  // Secondary: frames queued in FramesToProcess, readable without the lock.
  std::atomic<size_t> QueuedFrames{0u};

  // Secondary, game thread: set when loading the primary's map failed.
  bool bTravelFailed = false;

  FDelegateHandle OnPreWorldInitializationHandle;

  /// Value of p.Chaos.AccelerationStructureUseDynamicTree before deterministic
  /// collisions changed it; unset while they are off.
  TOptional<int32> DynamicTreeBeforeDeterministicCollisions;

  bool bIsPrimaryServer = true;

  // Set from the router's io-context thread (ConnectSession/HandleResponse
  // callbacks); consumed and cleared from the game thread in OnPostTick.
  std::atomic<bool> bNewConnection{false};

  std::unordered_map<uint32_t, uint32_t> MappedId;

  // Secondary, game thread: what playing the primary's frames keeps between
  // them (mirrors spawned, attachments pending, poses just set).
  FMirrorState MirrorState;

  // Secondary, game thread: mirrors something moved between playing a frame
  // and its capture, in this episode, and the classes already reported.
  uint64_t RestoredPoses = 0u;
  TSet<FName> RestoredPoseClasses;

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

  // Frames from the primary that could not be parsed completely in this
  // episode, and whether this episode already reported frames written by
  // another build; guarded by FrameToProcessMutex.
  uint64_t DroppedFrames = 0u;
  bool bWarnedFrameFormat = false;

  // Game thread only: BindReplayedSensor warns once per opened episode.
  bool bWarnedSensorBindRefused = false;

  FString PendingLoadMap;
  // What came with it: a world the primary generated (guarded like it).
  carla::multigpu::LoadMapExtras PendingLoadMapExtras;
  std::atomic<bool> bLoadMapPending{false};

  // Secondary, game thread: EPISODE_READY waits for generated content (OpenDRIVE street furniture).
  bool bEpisodeReadyPending = false;
  double EpisodeReadyPendingSince = 0.0;

  // Secondary, game thread: the map of the load started last.
  FString LoadingMap;

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
