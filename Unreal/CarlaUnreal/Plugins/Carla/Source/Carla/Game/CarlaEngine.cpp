// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "Carla/Game/CarlaEngine.h"
#include "Carla.h"
#include "Carla/ContentPacks/ContentPackManager.h"
#include "Carla/Game/CarlaEpisode.h"
#include "Carla/Game/CarlaGameModeBase.h"
#include "Carla/Game/CarlaStaticDelegates.h"
#include "Carla/Game/CarlaStatics.h"
#include "Carla/Lights/CarlaLightSubsystem.h"
#include "Carla/Recorder/CarlaRecorder.h"
#include "Carla/Settings/CarlaSettings.h"
#include "Carla/Settings/EpisodeSettings.h"
#include "Carla/MapGen/LargeMapManager.h"
#include "Carla/Traffic/TrafficLightManager.h"
#include "Carla/Actor/ActorData.h"
#include "Carla/Sensor/Sensor.h"

#include <util/disable-ue4-macros.h>
#include <carla/Logging.h>
#include <carla/multigpu/primaryCommands.h>
#include <carla/multigpu/commands.h>
#include <carla/multigpu/secondary.h>
#include <carla/multigpu/secondaryCommands.h>
#include <carla/ros2/ROS2.h>
#include <carla/ros2/middleware/Middleware.h>
#include <carla/ros2/middleware/MiddlewareConfig.h>
#include <carla/ros2/middleware/ActiveMiddleware.h>
#include <carla/streaming/EndPoint.h>
#include <carla/streaming/Server.h>
#include <util/enable-ue4-macros.h>

#include <util/ue-header-guard-begin.h>
#include "Engine/Engine.h"
#include "Engine/GameViewportClient.h"
#include "Misc/App.h"
#include "PhysicsEngine/PhysicsSettings.h"
#include "SceneInterface.h"
#include "Engine/Engine.h"
#include "Engine/GameViewportClient.h"
#include <util/ue-header-guard-end.h>

#include <cstdlib>
#include <cstring>
#include <optional>
#include <thread>

// =============================================================================
// -- Static local methods -----------------------------------------------------
// =============================================================================

// init static variables
uint64_t FCarlaEngine::FrameCounter = 0;

static uint32 FCarlaEngine_GetNumberOfThreadsForRPCServer()
{
  return std::max(std::thread::hardware_concurrency(), 4u) - 2u;
}

static TOptional<double> FCarlaEngine_GetFixedDeltaSeconds()
{
  return FApp::IsBenchmarking() ? FApp::GetFixedDeltaTime() : TOptional<double>{};
}

static void FCarlaEngine_SetFixedDeltaSeconds(TOptional<double> FixedDeltaSeconds)
{
  FApp::SetBenchmarking(FixedDeltaSeconds.IsSet());
  FApp::SetFixedDeltaTime(FixedDeltaSeconds.Get(0.0));
}

// =============================================================================
// -- FCarlaEngine -------------------------------------------------------------
// =============================================================================

FCarlaEngine::~FCarlaEngine()
{
  // The command callback captures this engine and outlives it otherwise
  // (Secondary is kept alive by its own commander).
  if (Secondary)
  {
    Secondary->Stop();
  }
  if (bIsRunning)
  {
    #if defined(WITH_ROS2)
    auto ROS2 = carla::ros2::ROS2::GetInstance();
    if (ROS2->IsEnabled())
      ROS2->Shutdown();
    #endif
    FWorldDelegates::OnWorldTickStart.Remove(OnPreTickHandle);
    FWorldDelegates::OnWorldPostActorTick.Remove(OnPostTickHandle);
    FCarlaStaticDelegates::OnEpisodeSettingsChange.Remove(OnEpisodeSettingsChangeHandle);
  }
}

void FCarlaEngine::NotifyInitGame(const UCarlaSettings &Settings)
{
  TRACE_CPUPROFILER_EVENT_SCOPE_STR(__FUNCTION__);
  if (!bIsRunning)
  {
    // Content packs are discovered when the engine subsystem initializes;
    // this is the no-op safety net that guarantees they are mounted before
    // the RPC server answers its first get_available_maps.
    if (UCarlaContentPackManager *ContentPacks = UCarlaContentPackManager::Get())
    {
      ContentPacks->Discover();
    }
    // A previous run's generated OpenDRIVE files must not shadow the cooked
    // OpenDriveMap stub in this one.
    UCarlaEpisode::ClearGeneratedWorldFiles();

    const auto StreamingPort = Settings.StreamingPort;
    const auto SecondaryPort = Settings.SecondaryPort;
    const auto PrimaryIP     = Settings.PrimaryIP;
    const auto PrimaryPort   = Settings.PrimaryPort;

    // rpclib's asio acceptor throws (std::system_error, "address already
    // in use") when the RPC port is taken -- typically another CARLA
    // instance still running. Unhandled it aborts the whole editor with
    // SIGABRT and a core dump; fail with a readable message instead.
    try
    {
      auto BroadcastStream = Server.Start(Settings.RPCPort, StreamingPort, SecondaryPort);
      Server.AsyncRun(FCarlaEngine_GetNumberOfThreadsForRPCServer());
      WorldObserver.SetStream(BroadcastStream);
    }
    catch (const std::exception &e)
    {
      UE_LOG(LogCarla, Error,
          TEXT("CARLA server failed to start on RPC port %d: %s. "
               "Another CARLA server is probably running on this port -- "
               "close it or launch with a different -carla-rpc-port. "
               "Shutting down."),
          Settings.RPCPort, UTF8_TO_TCHAR(e.what()));
      // Immediate exit: a deferred RequestExit lets the engine keep
      // initializing this half-built game instance and it segfaults in
      // teardown before ever reaching the main loop. The UE log mirror
      // may not flush before _exit, so also print straight to stderr.
      fprintf(stderr,
          "ERROR: CARLA server failed to start on RPC port %d: %s. "
          "Another CARLA server is probably running on this port -- "
          "close it or launch with a different -carla-rpc-port.\n",
          static_cast<int>(Settings.RPCPort), e.what());
      fflush(stderr);
      GLog->Flush();
      FPlatformMisc::RequestExit(true);
      return;
    }

    OnPreTickHandle = FWorldDelegates::OnWorldTickStart.AddRaw(
        this,
        &FCarlaEngine::OnPreTick);
    OnPostTickHandle = FWorldDelegates::OnWorldPostActorTick.AddRaw(
        this,
        &FCarlaEngine::OnPostTick);
    OnEpisodeSettingsChangeHandle = FCarlaStaticDelegates::OnEpisodeSettingsChange.AddRaw(
        this,
        &FCarlaEngine::OnEpisodeSettingsChanged);

    bIsRunning = true;

    // check to convert this as secondary server
    if (!PrimaryIP.empty())
    {
      // we are secondary server, connecting to primary server
      bIsPrimaryServer = false;

      // Clients subscribe with the primary's stream ids only (see GET_TOKEN).
      Server.GetStreamingServer().SetSessionsUseStreamAliases(true);

      // ROS commands address the sensor by the primary's stream id.
      auto FindAliasedStream = [this](const carla::Buffer &Data)
          -> std::optional<carla::streaming::detail::stream_id_type>
      {
        carla::streaming::detail::stream_id_type PrimaryStreamId = 0u;
        if (Data.size() < sizeof(PrimaryStreamId))
        {
          return std::nullopt;
        }
        std::memcpy(&PrimaryStreamId, Data.data(), sizeof(PrimaryStreamId));
        return Server.GetStreamingServer().FindStreamAlias(PrimaryStreamId);
      };

      // define the commands executor (when a command comes from the primary server)
      auto CommandExecutor = [=, this](carla::multigpu::MultiGPUCommand Id, carla::Buffer Data) {
        struct CarlaStreamBuffer : public std::streambuf
        {
            CarlaStreamBuffer(char *buf, std::size_t size) { setg(buf, buf, buf + size); }
        };
        switch (Id) {
          case carla::multigpu::MultiGPUCommand::SEND_FRAME:
          {
            {
              TRACE_CPUPROFILER_EVENT_SCOPE_STR("MultiGPUCommand::SEND_FRAME");
              std::scoped_lock<std::mutex> Lock(FrameToProcessMutex);
              if (CurrentEpisode)
              {
                // convert frame data from buffer to istream
                CarlaStreamBuffer TempStream(reinterpret_cast<char *>(Data.data()), Data.size());
                std::istream InStream(&TempStream);
                CurrentEpisode->GetFrameData().Read(InStream);
                {
                  TRACE_CPUPROFILER_EVENT_SCOPE_STR("FramesToProcess.emplace_back");
                  FramesToProcess.emplace_back(CurrentEpisode->GetFrameData());
                }
                // SEND_FRAME has no acknowledgement or backpressure, so a growing
                // backlog means this secondary is falling behind; surface it.
                // Logged only on state change, with separate warn/recovery
                // thresholds (hysteresis) so a backlog oscillating near one
                // value can't flip the log every tick.
                static constexpr int32 BacklogWarningThreshold = 5;
                static constexpr int32 BacklogRecoveryThreshold = 2;
                const int32 BacklogSize = static_cast<int32>(FramesToProcess.size());
                if (BacklogSize > BacklogWarningThreshold && !bFramesToProcessBacklogged)
                {
                  UE_LOG(LogCarla, Warning,
                      TEXT("Secondary server is falling behind the primary: %d frames queued"),
                      BacklogSize);
                  bFramesToProcessBacklogged = true;
                }
                else if (BacklogSize <= BacklogRecoveryThreshold && bFramesToProcessBacklogged)
                {
                  UE_LOG(LogCarla, Log, TEXT("Secondary server has caught up with the primary"));
                  bFramesToProcessBacklogged = false;
                }
              }
            }
            // forces a tick
            Server.Tick();
            break;
          }
          case carla::multigpu::MultiGPUCommand::LOAD_MAP:
          {
            const carla::multigpu::LoadMapRequest Request = carla::multigpu::ParseLoadMapPayload(
                std::string_view(reinterpret_cast<const char *>(Data.data()), Data.size()));
            {
              std::scoped_lock<std::mutex> Lock(FrameToProcessMutex);
              PendingLoadMap = UTF8_TO_TCHAR(Request.map.c_str());
              PendingLoadMapId = Request.load_id;
              PendingLoadEpoch = SensorStreams.BeginEpisodeChange(Server.GetStreamingServer());
              bLoadMapPending = true;
            }
            break;
          }
          case carla::multigpu::MultiGPUCommand::GET_TOKEN:
          {
            // Resolved by actor id: local stream ids follow this secondary's own creation order.
            carla::multigpu::GetTokenRequest Request{};
            std::optional<carla::streaming::detail::token_type> Token;
            if (Data.size() >= sizeof(Request))
            {
              std::memcpy(&Request, Data.data(), sizeof(Request));
              Token = SensorStreams.Resolve(Server.GetStreamingServer(), Request.actor_id, Request.stream_id);
            }
            if (!Token)
            {
              carla::log_warning("multigpu: cannot provide a token for sensor actor ", Request.actor_id,
                  " (", Data.size(), "-byte request); replying not ready");
              carla::Buffer Reply(
                  reinterpret_cast<const unsigned char *>(carla::multigpu::kTokenNotReadyMarker.data()),
                  carla::multigpu::kTokenNotReadyMarker.size());
              Secondary->Write(std::move(Reply));
              break;
            }
            carla::Buffer Reply(reinterpret_cast<const unsigned char *>(&*Token), sizeof(*Token));
            carla::log_info("responding with a token for port ", Token->get_port());
            Secondary->Write(std::move(Reply));
            break;
          }
          case carla::multigpu::MultiGPUCommand::YOU_ALIVE:
          {
            std::string msg("Yes, I'm alive");
            carla::Buffer buf((unsigned char *) msg.c_str(), (size_t) msg.size());
            carla::log_info("responding is alive command");
            Secondary->Write(std::move(buf));
            break;
          }
          case carla::multigpu::MultiGPUCommand::ENABLE_ROS:
          {
            const auto sensor_id = FindAliasedStream(Data);
            if (sensor_id)
            {
              Server.GetStreamingServer().EnableForROS(*sensor_id);
            }
            const bool res = sensor_id.has_value();
            carla::Buffer buf(reinterpret_cast<const unsigned char *>(&res), sizeof(res));
            carla::log_info("responding ENABLE_ROS with: ", res);
            Secondary->Write(std::move(buf));
            break;
          }
          case carla::multigpu::MultiGPUCommand::DISABLE_ROS:
          {
            const auto sensor_id = FindAliasedStream(Data);
            if (sensor_id)
            {
              Server.GetStreamingServer().DisableForROS(*sensor_id);
            }
            const bool res = sensor_id.has_value();
            carla::Buffer buf(reinterpret_cast<const unsigned char *>(&res), sizeof(res));
            carla::log_info("responding DISABLE_ROS with: ", res);
            Secondary->Write(std::move(buf));
            break;
          }
          case carla::multigpu::MultiGPUCommand::IS_ENABLED_ROS:
          {
            const auto sensor_id = FindAliasedStream(Data);
            const bool res = sensor_id && Server.GetStreamingServer().IsEnabledForROS(*sensor_id);
            carla::Buffer buf(reinterpret_cast<const unsigned char *>(&res), sizeof(res));
            carla::log_info("responding IS_ENABLED_ROS with: ", res);
            Secondary->Write(std::move(buf));
            break;
          }
        }
      };

      Secondary = std::make_shared<carla::multigpu::Secondary>(PrimaryIP, PrimaryPort, CommandExecutor);
      Secondary->Connect();
      // set this server in synchronous mode
      bSynchronousMode = true;
      if (!FApp::CanEverRender())
      {
        UE_LOG(LogCarla, Warning,
            TEXT("Multi-GPU secondary started without rendering (-nullrhi): camera sensors routed here will produce no data"));
      }
    }
    else
    {
      // we are primary server, starting server
      bIsPrimaryServer = true;
      SecondaryServer = Server.GetSecondaryServer();
      SecondaryServer->SetNewConnectionCallback([this]()
      {
        this->bNewConnection = true;
        UE_LOG(LogCarla, Log, TEXT("New secondary connection detected"));
      });
      if (!FApp::CanEverRender())
      {
        UE_LOG(LogCarla, Log,
            TEXT("Rendering disabled (-nullrhi): running as a non-rendering, authority-only primary. Camera sensors need a connected rendering secondary server to produce data."));
      }
    }
  }

  // create ROS2 manager
  #if defined(WITH_ROS2)
  if (Settings.ROS2)
  {
    UE_LOG(LogCarla, Log, TEXT("ROS2: Creating ROS2 Instance..."));
    auto ROS2 = carla::ros2::ROS2::GetInstance();
    const std::string Rmw = TCHAR_TO_UTF8(*Settings.RmwName);
    const auto Parsed = carla::ros2::MiddlewareFromString(Rmw);
    if (!Parsed.valid)
    {
      UE_LOG(LogCarla, Error,
          TEXT("ROS2: unrecognized --rmw value '%s'. Available: %s. ROS2 is DISABLED for this session."),
          *Settings.RmwName,
          UTF8_TO_TCHAR(carla::ros2::GetAvailableMiddleware().c_str()));
    }
    else
    {
      int32 DomainId = Settings.ROS2DomainId;
      if (DomainId != carla::ros2::kUnsetDomainId && !carla::ros2::IsValidDomainId(DomainId))
      {
        UE_LOG(LogCarla, Error,
            TEXT("ROS2: --ros-domain-id=%d is out of range [%d, %d]; using the default domain."),
            DomainId, carla::ros2::kMinDomainId, carla::ros2::kMaxDomainId);
        DomainId = carla::ros2::kUnsetDomainId;
      }
      if (!ROS2->Enable(true, Parsed.middleware, DomainId))
      {
        UE_LOG(LogCarla, Error,
            TEXT("ROS2: --rmw='%s' is not compiled into this binary. Available: %s. ROS2 is DISABLED for this session."),
            *Settings.RmwName,
            UTF8_TO_TCHAR(carla::ros2::GetAvailableMiddleware().c_str()));
      }
      else
      {
        UE_LOG(LogCarla, Log, TEXT("ROS2: enabled with middleware '%s'."), *Settings.RmwName);
        if (Settings.RmwName == TEXT("fastdds"))
        {
          // Mirrors the transport default applied in FastDDSSharedParticipant:
          // UDPv4-only unless FASTDDS_BUILTIN_TRANSPORTS is set (Fast-DDS
          // shared memory silently drops data across container/uid/version
          // boundaries, so it is opt-in).
          const char *TransportsEnv = std::getenv("FASTDDS_BUILTIN_TRANSPORTS");
          if (TransportsEnv != nullptr)
          {
            UE_LOG(LogCarla, Log,
                TEXT("ROS2: Fast-DDS builtin transports set from FASTDDS_BUILTIN_TRANSPORTS='%s'."),
                UTF8_TO_TCHAR(TransportsEnv));
          }
          else
          {
            UE_LOG(LogCarla, Log,
                TEXT("ROS2: Fast-DDS transport: UDPv4 only (default; set FASTDDS_BUILTIN_TRANSPORTS=DEFAULT to re-enable shared memory)."));
          }
        }
        // Apply the configured default topic visibility before any sensor stream is
        // created. Gated on Settings.ROS2 so non-ROS2 runs never force every stream
        // active (which would make every sensor produce data each tick).
        Server.GetStreamingServer().SetROS2TopicVisibilityDefaultEnabled(Settings.ROS2TopicVisibility);
      }
    }
  } else {
    UE_LOG(LogCarla, Log, TEXT("ROS2: ROS2 enabled..."));
  }
  #else
    UE_LOG(LogCarla, Log, TEXT("ROS2: ROS2 extension not build..."));
  #endif

  bMapChanged = true;
}

void FCarlaEngine::NotifyBeginEpisode(UCarlaEpisode &Episode)
{
  TRACE_CPUPROFILER_EVENT_SCOPE_STR(__FUNCTION__);
  Episode.EpisodeSettings.FixedDeltaSeconds = FCarlaEngine_GetFixedDeltaSeconds();
  {
    std::scoped_lock<std::mutex> Lock(FrameToProcessMutex);
    CurrentEpisode = &Episode;
  }

  // Reset map settings
  UWorld* World = CurrentEpisode->GetWorld();
  ALargeMapManager* LargeMapManager = UCarlaStatics::GetLargeMapManager(World);
  if (LargeMapManager)
  {
    CurrentSettings.TileStreamingDistance = LargeMapManager->GetLayerStreamingDistance();
    CurrentSettings.ActorActiveDistance = LargeMapManager->GetActorStreamingDistance();
  }

  // The settings survive load_world(reset_settings=False) but the traffic
  // light manager is new with every map, and a map can enable sign snapping
  // on its own. Enable it on whichever side lacks it so that they agree.
  ACarlaGameModeBase* GameMode = UCarlaStatics::GetGameMode(World);
  if (ATrafficLightManager* TrafficLightManager =
          GameMode ? GameMode->GetTrafficLightManager() : nullptr)
  {
    if (CurrentSettings.bAdjustSignsHeightToGround)
    {
      TrafficLightManager->SetAdjustSignsHeightToGround(true);
    }
    else
    {
      CurrentSettings.bAdjustSignsHeightToGround =
          TrafficLightManager->GetAdjustSignsHeightToGround();
    }
  }

  if (!bIsPrimaryServer)
  {
    // set this secondary server with no-rendering mode
    CurrentSettings.bNoRenderingMode = true;
  }

  CurrentEpisode->ApplySettings(CurrentSettings);

  ResetFrameCounter(GFrameNumber);

  // make connection between Episode and Recorder
  if (Recorder)
  {
    Recorder->SetEpisode(&Episode);
    Episode.SetRecorder(Recorder);
    Recorder->GetReplayer()->CheckPlayAfterMapLoaded();
  }

  Server.NotifyBeginEpisode(Episode);

  Episode.bIsPrimaryServer = bIsPrimaryServer;

  if (!bIsPrimaryServer && Secondary)
  {
    SensorStreams.OpenEpisode(LoadingEpoch);
    bWarnedSensorBindRefused = false;

    // Re-arms the primary's full-resync flag (see Router::HandleResponse),
    // since connect-time arming predates this secondary's own level load.
    // Write(Buffer), not Write(std::string): the latter has no completion
    // handler keeping the message alive and can dangle.
    const std::string Ready = carla::multigpu::MakeEpisodeReadyMessage(LoadingLoadMapId);
    carla::Buffer Marker(reinterpret_cast<const unsigned char *>(Ready.data()), Ready.size());
    Secondary->Write(std::move(Marker));
  }
}

void FCarlaEngine::NotifyEndEpisode()
{
  SensorStreams.CloseEpisode();
  Server.NotifyEndEpisode();
  std::scoped_lock<std::mutex> Lock(FrameToProcessMutex);
  CurrentEpisode = nullptr;
  FramesToProcess.clear();
  bFramesToProcessBacklogged = false;
}

void FCarlaEngine::BindReplayedSensor(uint32_t PrimaryActorId, uint32_t LocalActorId, bool bCreated)
{
  FCarlaActor *CarlaActor = CurrentEpisode->FindCarlaActor(LocalActorId);
  if ((CarlaActor == nullptr) || (CarlaActor->GetActorType() != FCarlaActor::ActorType::Sensor))
  {
    return;
  }

  if (!SensorStreams.IsEpisodeOpen())
  {
    if (!bWarnedSensorBindRefused)
    {
      UE_LOG(LogCarla, Warning,
          TEXT("Multi-GPU: ignoring replayed sensor %u (primary actor %u): the episode of the latest map load is not open here (did the level load fail?); its clients get no data until it is"),
          LocalActorId, PrimaryActorId);
      bWarnedSensorBindRefused = true;
    }
    return;
  }

  ASensor *Sensor = Cast<ASensor>(CarlaActor->GetActor());
  const bool bSensorHasStream = (Sensor != nullptr) && Sensor->IsStreamReady();
  std::optional<carla::streaming::detail::stream_id_type> OwnStreamId;
  if (bSensorHasStream)
  {
    OwnStreamId = carla::streaming::detail::token_type(Sensor->GetToken()).get_stream_id();
  }
  else if (FActorSensorData *DormantData = CarlaActor->GetActorData<FActorSensorData>())
  {
    if (DormantData->Stream.IsStreamReady())
    {
      OwnStreamId = carla::streaming::detail::token_type(DormantData->Stream.GetToken()).get_stream_id();
    }
  }
  if (!OwnStreamId)
  {
    UE_LOG(LogCarla, Warning,
        TEXT("Multi-GPU: replayed sensor %u (primary actor %u) has no stream; its clients get no data"),
        LocalActorId, PrimaryActorId);
    return;
  }

  // Only a sensor created by this replay step has never ticked; a live
  // sensor's stream must not be replaced (its capture reads it concurrently).
  const bool bCanAdopt = bCreated && bSensorHasStream;
  auto ReservedStream = SensorStreams.Bind(Server.GetStreamingServer(), PrimaryActorId, *OwnStreamId, bCanAdopt);
  if (!ReservedStream)
  {
    return;
  }
  Sensor->SetDataStream(FDataStream(std::move(*ReservedStream)));
  Server.GetStreamingServer().CloseStream(*OwnStreamId);
}

void FCarlaEngine::WaitForSecondaryEpisodes()
{
  if (!SecondaryServer || !SecondaryServer->IsAnySecondaryLoading())
  {
    return;
  }
  TRACE_CPUPROFILER_EVENT_SCOPE_STR(__FUNCTION__);
  static constexpr double TimeoutSeconds = 30.0;
  const double Start = FPlatformTime::Seconds();
  const double Deadline = Start + TimeoutSeconds;
  UE_LOG(LogCarla, Log, TEXT("Multi-GPU: waiting for secondary servers to load the new episode"));
  while (SecondaryServer->IsAnySecondaryLoading())
  {
    if (FPlatformTime::Seconds() >= Deadline)
    {
      UE_LOG(LogCarla, Warning,
          TEXT("Multi-GPU: secondary servers did not load the new episode within %.0f s (waited %.0f ms); ticking without waiting"),
          TimeoutSeconds, (FPlatformTime::Seconds() - Start) * 1000.0);
      SecondaryServer->StopWaitingForSecondaryLoads();
      return;
    }
    Server.RunSome(1u);
  }
  UE_LOG(LogCarla, Log, TEXT("Multi-GPU: all secondary servers loaded the new episode after %.0f ms"),
      (FPlatformTime::Seconds() - Start) * 1000.0);
}

void FCarlaEngine::OnPreTick(UWorld *, ELevelTick TickType, float DeltaSeconds)
{
  TRACE_CPUPROFILER_EVENT_SCOPE_STR(__FUNCTION__);
  if (TickType == ELevelTick::LEVELTICK_All)
  {

    if (bIsPrimaryServer)
    {
      if (CurrentEpisode && !bSynchronousMode && SecondaryServer->HasClientsConnected())
      {
        // set synchronous mode
        CurrentSettings.bSynchronousMode = true;
        CurrentSettings.FixedDeltaSeconds = 1 / 20.0f;
        OnEpisodeSettingsChanged(CurrentSettings);
        CurrentEpisode->ApplySettings(CurrentSettings);
      }

      // process RPC commands
      do
      {
        Server.RunSome(1u);
      }
      while (bSynchronousMode && !Server.TickCueReceived());

      // Frames sent while a secondary is still loading are dropped there.
      if (bSynchronousMode)
      {
        WaitForSecondaryEpisodes();
      }
    }
    else
    {
      // process frame data
      do
      {
        Server.RunSome(1u);
      }
      while (!FramesToProcess.size() && !(bLoadMapPending && CurrentEpisode));

      if (bLoadMapPending && CurrentEpisode)
      {
        FString MapToLoad;
        {
          std::scoped_lock<std::mutex> Lock(FrameToProcessMutex);
          MapToLoad = std::move(PendingLoadMap);
          PendingLoadMap.Reset();
          LoadingEpoch = PendingLoadEpoch;
          LoadingLoadMapId = PendingLoadMapId;
          bLoadMapPending = false;
        }
        UGameplayStatics::OpenLevel(CurrentEpisode->GetWorld(), *MapToLoad, true);
      }
    }

    // update frame counter
    UpdateFrameCounter();

    if (CurrentEpisode)
    {
      CurrentEpisode->TickTimers(DeltaSeconds);

      if (!bIsPrimaryServer)
      {
        if (FramesToProcess.size())
        {
          TRACE_CPUPROFILER_EVENT_SCOPE_STR("FramesToProcess.PlayFrameData");
          std::scoped_lock<std::mutex> Lock(FrameToProcessMutex);
          FramesToProcess.front().PlayFrameData(
              CurrentEpisode,
              MappedId,
              [this](uint32_t PrimaryActorId, uint32_t LocalActorId, bool bCreated)
              {
                BindReplayedSensor(PrimaryActorId, LocalActorId, bCreated);
              },
              [this](uint32_t PrimaryActorId)
              {
                SensorStreams.Unbind(Server.GetStreamingServer(), PrimaryActorId);
              });
          FramesToProcess.erase(FramesToProcess.begin()); // remove first element
        }
      }
    }
  }
}


void FCarlaEngine::OnPostTick(UWorld *World, ELevelTick TickType, float DeltaSeconds)
{
  TRACE_CPUPROFILER_EVENT_SCOPE_STR(__FUNCTION__);
  // With -RenderOffScreen nobody can see the main viewport, but its world
  // render still costs a full scene pass (Lumen, shadows, clouds) every frame
  // next to the sensors' own captures. Keep it disabled in that mode; sensors
  // are independent scene captures and are unaffected.
  static const bool bRenderOffScreen =
      FParse::Param(FCommandLine::Get(), TEXT("RenderOffScreen"));
  if (bRenderOffScreen && GEngine && GEngine->GameViewport &&
      !GEngine->GameViewport->bDisableWorldRendering)
  {
    GEngine->GameViewport->bDisableWorldRendering = true;
  }
  // FScene::SceneFrameNumber normally advances once per main-viewport world
  // render (FRendererModule::BeginRenderingViewFamilies). With the viewport's
  // world render disabled it freezes, every scene capture gets
  // ViewFamily.FrameNumber == 0, and all frame-keyed renderer caches (global
  // distance field, virtual shadow maps, Lumen temporal state) treat every
  // capture as "never rendered" and rebuild from scratch. Advance it manually
  // so sensors keep the normal per-frame cache behavior.
  if (World && World->Scene && GEngine && GEngine->GameViewport &&
      GEngine->GameViewport->bDisableWorldRendering)
  {
    World->Scene->IncrementFrameNumber();
  }
  // tick the recorder/replayer system
  if (GetCurrentEpisode())
  {
    if (bIsPrimaryServer)
    {
      if (SecondaryServer->HasClientsConnected()) {
        const bool bWasNewConnection = bNewConnection.exchange(false);
        GetCurrentEpisode()->GetFrameData().GetFrameData(GetCurrentEpisode(), true, bWasNewConnection);
        std::ostringstream OutStream;
        GetCurrentEpisode()->GetFrameData().Write(OutStream);

        // send frame data to secondary
        std::string Tmp(OutStream.str());
        SecondaryServer->GetCommander().SendFrameData(carla::Buffer(std::move((unsigned char *) Tmp.c_str()), (size_t) Tmp.size()));

        GetCurrentEpisode()->GetFrameData().Clear();
      }
    }

    auto* EpisodeRecorder = GetCurrentEpisode()->GetRecorder();
    if (EpisodeRecorder)
    {
      EpisodeRecorder->Ticking(DeltaSeconds);
    }
  }

  if ((TickType == ELevelTick::LEVELTICK_All) && (CurrentEpisode != nullptr))
  {
    // Look for lightsubsystem
    bool LightUpdatePending = false;
    if (World)
    {
      UCarlaLightSubsystem* CarlaLightSubsystem = World->GetSubsystem<UCarlaLightSubsystem>();
      if (CarlaLightSubsystem)
      {
        LightUpdatePending = CarlaLightSubsystem->IsUpdatePending();
      }
    }

    // send the worldsnapshot
    WorldObserver.BroadcastTick(*CurrentEpisode, DeltaSeconds, bMapChanged, LightUpdatePending);
    CurrentEpisode->GetSensorManager().PostPhysTick(World, TickType, DeltaSeconds);
    ResetSimulationState();
  }
}

void FCarlaEngine::OnEpisodeSettingsChanged(const FEpisodeSettings &Settings)
{
  CurrentSettings = FEpisodeSettings(Settings);

  bSynchronousMode = Settings.bSynchronousMode;

#if WITH_EDITOR
  if (GEngine && GEngine->GameViewport)
  {
    // -RenderOffScreen keeps the invisible main viewport's world render off
    // regardless of the episode's no-rendering setting (see OnPostTick).
    GEngine->GameViewport->bDisableWorldRendering = Settings.bNoRenderingMode ||
        FParse::Param(FCommandLine::Get(), TEXT("RenderOffScreen"));
  }
#endif
  FCarlaEngine_SetFixedDeltaSeconds(Settings.FixedDeltaSeconds);

  // Setting parameters for physics substepping
  UPhysicsSettings* PhysSett = UPhysicsSettings::Get();
  PhysSett->bSubstepping = Settings.bSubstepping;
  PhysSett->MaxSubstepDeltaTime = Settings.MaxSubstepDeltaTime;
  PhysSett->MaxSubsteps = Settings.MaxSubsteps;

  UWorld* World = CurrentEpisode->GetWorld();
  ALargeMapManager* LargeMapManager = UCarlaStatics::GetLargeMapManager(World);
  if (LargeMapManager)
  {
    LargeMapManager->SetLayerStreamingDistance(Settings.TileStreamingDistance);
    LargeMapManager->SetActorStreamingDistance(Settings.ActorActiveDistance);
  }
}

void FCarlaEngine::ResetSimulationState()
{
  bMapChanged = false;
}
