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
#include "Carla/OpenDrive/OpenDrive.h"
#include "Carla/OpenDrive/OpenDriveGenerator.h"
#include "Carla/Traffic/TrafficLightManager.h"
#include "Carla/Actor/ActorData.h"
#include "Carla/Sensor/Sensor.h"

#include <util/disable-ue4-macros.h>
#include <carla/Logging.h>
#include <carla/MsgPack.h>
#include <carla/multigpu/primaryCommands.h>
#include <carla/multigpu/commands.h>
#include <carla/multigpu/frameGate.h>
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
#include "EngineUtils.h"
#include "GameFramework/WorldSettings.h"
#include "HAL/FileManager.h"
#include "HAL/IConsoleManager.h"
#include "HAL/PlatformProcess.h"
#include "HAL/PlatformTime.h"
#include "Misc/App.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
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

#if !UE_BUILD_SHIPPING
static TAutoConsoleVariable<int32> CVarCarlaMultiGpuVerifyFrames(
    TEXT("carla.MultiGpu.VerifyFrames"),
    1,
    TEXT("Multi-GPU primary: read every frame sent to the secondaries back and log an error when it ")
    TEXT("does not parse completely with this build (1 = on, 0 = off)."),
    ECVF_Default);
#endif // !UE_BUILD_SHIPPING

static TOptional<double> FCarlaEngine_GetFixedDeltaSeconds()
{
  return FApp::IsBenchmarking() ? FApp::GetFixedDeltaTime() : TOptional<double>{};
}

static void FCarlaEngine_SetFixedDeltaSeconds(TOptional<double> FixedDeltaSeconds)
{
  FApp::SetBenchmarking(FixedDeltaSeconds.IsSet());
  FApp::SetFixedDeltaTime(FixedDeltaSeconds.Get(0.0));
}

/// Whether a travel to another map is scheduled for @a World: load_world()
/// only schedules it, and the engine performs it at the start of the next
/// engine frame, before ticking any world.
static bool FCarlaEngine_IsWorldTravelPending(const UWorld *World)
{
  const FWorldContext *Context = (GEngine && World) ? GEngine->GetWorldContextFromWorld(World) : nullptr;
  return (Context != nullptr) && !Context->TravelURL.IsEmpty();
}

static bool FCarlaEngine_SameSettings(const FEpisodeSettings &A, const FEpisodeSettings &B)
{
  return
      (A.bSynchronousMode == B.bSynchronousMode) &&
      (A.bNoRenderingMode == B.bNoRenderingMode) &&
      (A.bSubstepping == B.bSubstepping) &&
      (A.FixedDeltaSeconds == B.FixedDeltaSeconds) &&
      (A.MaxSubstepDeltaTime == B.MaxSubstepDeltaTime) &&
      (A.MaxSubsteps == B.MaxSubsteps) &&
      (A.MaxCullingDistance == B.MaxCullingDistance) &&
      (A.bDeterministicRagdolls == B.bDeterministicRagdolls) &&
      (A.TileStreamingDistance == B.TileStreamingDistance) &&
      (A.ActorActiveDistance == B.ActorActiveDistance) &&
      (A.SpectatorAsEgo == B.SpectatorAsEgo) &&
      (A.bAdjustSignsHeightToGround == B.bAdjustSignsHeightToGround);
}

/// Multi-GPU secondary: the world the primary generated from OpenDRIVE, where
/// this server's OpenDriveMap reads it and with the same parameters, before
/// the map opens; for any other map, no generated world of an earlier load.
static void FCarlaEngine_PrepareGeneratedWorld(UWorld *World, const carla::multigpu::LoadMapExtras &Extras)
{
  const FString XODRPath = UOpenDrive::GetSavedXODRPath(TEXT("OpenDriveMap"));
  IFileManager::Get().Delete(*XODRPath, false, true, true);
  if (!Extras.opendrive)
  {
    return;
  }
  TArray<uint8> OpenDrive;
  OpenDrive.Append(reinterpret_cast<const uint8 *>(Extras.opendrive->data()),
      static_cast<int32>(Extras.opendrive->size()));
  if (!FFileHelper::SaveArrayToFile(OpenDrive, *XODRPath))
  {
    UE_LOG(LogCarla, Error, TEXT("Multi-GPU: cannot write the primary's OpenDRIVE world to %s"), *XODRPath);
    return;
  }
  UCarlaGameInstance *GameInstance = UCarlaStatics::GetGameInstance(World);
  if ((GameInstance == nullptr) || !Extras.generation_parameters)
  {
    return;
  }
  try
  {
    GameInstance->SetOpendriveGenerationParameters(
        carla::MsgPack::UnPack<carla::rpc::OpendriveGenerationParameters>(
            reinterpret_cast<const unsigned char *>(Extras.generation_parameters->data()),
            Extras.generation_parameters->size()));
  }
  catch (const std::exception &e)
  {
    UE_LOG(LogCarla, Error, TEXT("Multi-GPU: the primary's OpenDRIVE generation parameters do not read: %s"),
        UTF8_TO_TCHAR(e.what()));
  }
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
    // OnBeginFrame took over the process-wide time step; give it back (the
    // editor keeps running after a PIE secondary ends).
    FApp::SetUseFixedTimeStep(false);
    // The generated worlds the primary sent are of no use to anyone else.
    IFileManager::Get().DeleteDirectory(
        *FPaths::GetPath(UOpenDrive::GetSavedXODRPath(TEXT("OpenDriveMap"))), false, true);
    UOpenDrive::SetSavedXODRDirectory(FString());
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
    FCoreDelegates::OnBeginFrame.Remove(OnBeginFrameHandle);
    if (GEngine)
    {
      GEngine->OnTravelFailure().Remove(OnTravelFailureHandle);
    }
    FWorldDelegates::OnPreWorldInitialization.Remove(OnPreWorldInitializationHandle);
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
    // OpenDriveMap stub in this one. A Multi-GPU secondary keeps the worlds
    // the primary sends in a folder of its own and leaves the shared files
    // alone: the primary may run from the same installation.
    if (Settings.PrimaryIP.empty())
    {
      UCarlaEpisode::ClearGeneratedWorldFiles();
    }
    else
    {
      // Unique to this process even across PID namespaces (containers that
      // share an installation).
      UOpenDrive::SetSavedXODRDirectory(FPaths::ProjectSavedDir() / FString::Printf(
          TEXT("OpenDrive/MultiGpuSecondary-%s"), *FApp::GetInstanceId().ToString()));
    }

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
    OnPreWorldInitializationHandle = FWorldDelegates::OnPreWorldInitialization.AddRaw(
        this,
        &FCarlaEngine::OnPreWorldInitialization);

    bIsRunning = true;

    // check to convert this as secondary server
    if (!PrimaryIP.empty())
    {
      // we are secondary server, connecting to primary server
      bIsPrimaryServer = false;
      Server.SetIsSecondaryServer(true);

      // Clients subscribe with the primary's stream ids only (see GET_TOKEN).
      Server.GetStreamingServer().SetSessionsUseStreamAliases(true);
      // Except for this server's own world snapshots, which direct clients of
      // this server read; the primary never routes its stream 1 here.
      {
        const auto BroadcastStreamId =
            carla::streaming::detail::token_type(WorldObserver.GetToken()).get_stream_id();
        Server.GetStreamingServer().SetStreamAlias(BroadcastStreamId, BroadcastStreamId);
      }

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
        switch (Id) {
          case carla::multigpu::MultiGPUCommand::SEND_FRAME:
          {
            TRACE_CPUPROFILER_EVENT_SCOPE_STR("MultiGPUCommand::SEND_FRAME");
            // Parsed outside the lock, so a large frame does not hold up the
            // game thread. Only a complete frame is queued: a frame is never
            // played in part.
            FFrameData Frame;
            const carla::multigpu::FrameDataReadResult Result = Frame.Read(Data.data(), Data.size());
            const uint64_t FrameNumber = Frame.GetFrameNumber();
            bool bReportDrop = false;
            bool bReportFormat = false;
            {
              std::scoped_lock<std::mutex> Lock(FrameToProcessMutex);
              if (CurrentEpisode && !Result.IsComplete())
              {
                bReportDrop = (DroppedFrames++ == 0u);
                // This server missed the primary's state: get all of it again.
                bResyncRequested = true;
              }
              else if (CurrentEpisode)
              {
                bReportFormat = ((Result.skipped > 0u) || (Result.grown > 0u)) && !bWarnedFrameFormat;
                bWarnedFrameFormat = bWarnedFrameFormat || bReportFormat;
                {
                  TRACE_CPUPROFILER_EVENT_SCOPE_STR("FramesToProcess.emplace_back");
                  FramesToProcess.emplace_back(std::move(Frame));
                  QueuedFrames = FramesToProcess.size();
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
            // carla::log_* rather than UE_LOG, so that Shipping logs show them too.
            if (bReportDrop)
            {
              carla::log_error("multigpu: dropping a frame from the primary:",
                  carla::multigpu::FrameDataStatusName(Result.status), "at packet",
                  static_cast<uint32_t>(Result.packet_id), "(offset", Result.offset, "declaring",
                  Result.packet_size, "bytes) of a", Data.size(),
                  "byte frame. Every server must run the same CARLA build.");
            }
            if (bReportFormat)
            {
              carla::log_warning("multigpu: frame", FrameNumber, "from the primary carries",
                  Result.skipped, "packets this build does not know (first id",
                  static_cast<uint32_t>(Result.first_skipped_id), ") and", Result.grown,
                  "packets longer than it reads. Every server must run the same CARLA build.");
            }
            break;
          }
          case carla::multigpu::MultiGPUCommand::LOAD_MAP:
          {
            const carla::multigpu::LoadMapRequest Request = carla::multigpu::ParseLoadMapPayload(
                std::string_view(reinterpret_cast<const char *>(Data.data()), Data.size()));
            {
              std::scoped_lock<std::mutex> Lock(FrameToProcessMutex);
              PendingLoadMap = UTF8_TO_TCHAR(Request.map.c_str());
              PendingLoadMapExtras = Request.extras;
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
      // From here on, OnBeginFrame sets this server's time step from every
      // primary frame; the benchmarking switch must not override it.
      FApp::SetBenchmarking(false);
      OnBeginFrameHandle = FCoreDelegates::OnBeginFrame.AddRaw(this, &FCarlaEngine::OnBeginFrame);
      if (GEngine)
      {
        OnTravelFailureHandle = GEngine->OnTravelFailure().AddLambda(
            [this](UWorld *, ETravelFailure::Type, const FString &Error)
            {
              bTravelFailed = true;
              carla::log_error("multigpu: failed to load the map the primary loaded:", TCHAR_TO_UTF8(*Error));
            });
      }
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
    // The new episode has a new actor registry.
    MappedId.clear();
    MirrorState.Reset();
    DroppedFrames = 0u;
    bWarnedFrameFormat = false;
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
    // and synchronous, as it plays the primary's frames: its own clients (read-only
    // queries) then never wait for a tick that only the primary triggers
    CurrentSettings.bSynchronousMode = true;
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
    PreparedFrame.reset();
    LastPlayedFrame.reset();
    bWarnedUntimedFrame = false;
    bWarnedUnpairedFrame = false;
    bWarnedNoSensorTicks = false;
    RestoredPoses = 0u;
    RestoredPoseClasses.Reset();
    // Map actors that simulate physics (movable props) would fall or drift on
    // their own here; the primary's frames move them instead.
    for (auto &It : Episode.GetActorRegistry())
    {
      FCarlaActor *CarlaActor = It.Value.Get();
      if ((CarlaActor != nullptr) && (CarlaActor->GetActorType() == FCarlaActor::ActorType::Other) &&
          !CarlaActor->IsDormant() && (CarlaActor->GetActor() != nullptr))
      {
        if (auto *Root = Cast<UPrimitiveComponent>(CarlaActor->GetActor()->GetRootComponent()))
        {
          if (Root->IsSimulatingPhysics())
          {
            Root->SetSimulatePhysics(false);
          }
        }
      }
    }
    if (bTravelFailed)
    {
      // The engine fell back to another map: playing the primary's frames here
      // would mirror them onto the wrong world.
      bTravelFailed = false;
      DetachFromPrimary("could not load the primary's map; all servers must run the same build with the same content");
      return;
    }
    if (!LoadingMap.IsEmpty() &&
        !FPaths::GetBaseFilename(LoadingMap).Equals(Episode.GetMapName(), ESearchCase::IgnoreCase))
    {
      UE_LOG(LogCarla, Warning, TEXT("Multi-GPU: the primary loaded %s and this secondary is on %s"),
          *LoadingMap, *Episode.GetMapName());
    }

    SensorStreams.OpenEpisode(LoadingEpoch);
    bWarnedSensorBindRefused = false;

    // Re-arms the primary's full-resync flag (see Router::HandleResponse),
    // since connect-time arming predates this secondary's own level load.
    bEpisodeReadyPending = true;
    EpisodeReadyPendingSince = FPlatformTime::Seconds();
    SendEpisodeReadyWhenContentReady();
  }
}

void FCarlaEngine::SendEpisodeReadyWhenContentReady()
{
  if (!bEpisodeReadyPending || CurrentEpisode == nullptr)
  {
    return;
  }
  // The primary mirrors frames only after EPISODE_READY, so its first frame finds the furniture spawned.
  for (TActorIterator<AOpenDriveGenerator> It(CurrentEpisode->GetWorld()); It; ++It)
  {
    if (It->IsStreetFurnitureGenerating())
    {
      return;
    }
  }
  bEpisodeReadyPending = false;
  UE_LOG(LogCarla, Log, TEXT("Multi-GPU: episode content ready after %.2f s, sending EPISODE_READY"),
      FPlatformTime::Seconds() - EpisodeReadyPendingSince);
  SendEpisodeReady();
}

void FCarlaEngine::SendEpisodeReady()
{
  const std::string Ready = carla::multigpu::MakeEpisodeReadyMessage(LoadingLoadMapId);
  Secondary->Write(carla::Buffer(reinterpret_cast<const unsigned char *>(Ready.data()), Ready.size()));
}

void FCarlaEngine::DetachFromPrimary(const char *Reason)
{
  carla::log_error("multigpu:", Reason, "- this secondary stops mirroring the primary until it is restarted");
  if (Secondary)
  {
    // Joins the command threads; nothing they run waits on this thread.
    Secondary->Stop();
  }
  {
    std::scoped_lock<std::mutex> Lock(FrameToProcessMutex);
    FramesToProcess.clear();
    QueuedFrames = 0u;
  }
  PreparedFrame.reset();
  LastPlayedFrame.reset();
  FApp::SetUseFixedTimeStep(false);
}

void FCarlaEngine::NotifyEndEpisode()
{
  SensorStreams.CloseEpisode();
  Server.NotifyEndEpisode();
  uint64_t Dropped = 0u;
  {
    std::scoped_lock<std::mutex> Lock(FrameToProcessMutex);
    CurrentEpisode = nullptr;
    FramesToProcess.clear();
    QueuedFrames = 0u;
    bFramesToProcessBacklogged = false;
    Dropped = DroppedFrames;
    DroppedFrames = 0u;
  }
  if (Dropped > 1u)
  {
    carla::log_error("multigpu: dropped", Dropped, "incomplete frames from the primary in this episode");
  }
  if (DeltaMismatches > 0u)
  {
    carla::log_warning("multigpu:", DeltaMismatches, "frames of this episode ran with another time step than the primary's");
    DeltaMismatches = 0u;
  }
  if (RestoredPoses > 0u)
  {
    carla::log_warning("multigpu:", RestoredPoses, "mirror poses had moved before their capture in this episode and were restored");
    RestoredPoses = 0u;
  }
  PreparedFrame.reset();
  LastPlayedFrame.reset();
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
    if (CurrentEpisode && FCarlaEngine_IsWorldTravelPending(CurrentEpisode->GetWorld()))
    {
      // This frame belongs to a world being left: the new one waits instead.
      return;
    }
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

void FCarlaEngine::OnPreTick(UWorld *World, ELevelTick TickType, float DeltaSeconds)
{
  TRACE_CPUPROFILER_EVENT_SCOPE_STR(__FUNCTION__);
  if (TickType != ELevelTick::LEVELTICK_All)
  {
    return;
  }

  if (!bIsPrimaryServer)
  {
    const carla::multigpu::PreTickAction Action = carla::multigpu::DecidePreTick(
        bLoadMapPending,
        CurrentEpisode != nullptr,
        PreparedFrame.has_value(),
        (CurrentEpisode != nullptr) && (World == CurrentEpisode->GetWorld()));
    if (Action == carla::multigpu::PreTickAction::OpenLevel)
    {
      FString MapToLoad;
      carla::multigpu::LoadMapExtras Extras;
      {
        std::scoped_lock<std::mutex> Lock(FrameToProcessMutex);
        MapToLoad = std::move(PendingLoadMap);
        PendingLoadMap.Reset();
        Extras = std::move(PendingLoadMapExtras);
        PendingLoadMapExtras = {};
        LoadingEpoch = PendingLoadEpoch;
        LoadingLoadMapId = PendingLoadMapId;
        bLoadMapPending = false;
      }
      PreparedFrame.reset();
      LoadingMap = MapToLoad;
      // A map this package lacks would make the engine fall back to another
      // one, and frames of the primary's world would then be played there.
      if (UCarlaStatics::FindMapPath(FPaths::GetBaseFilename(MapToLoad)).IsEmpty())
      {
        DetachFromPrimary(TCHAR_TO_UTF8(*FString::Printf(
            TEXT("the primary loaded %s, which this server does not have"), *MapToLoad)));
        return;
      }
      FCarlaEngine_PrepareGeneratedWorld(CurrentEpisode->GetWorld(), Extras);
      UGameplayStatics::OpenLevel(CurrentEpisode->GetWorld(), *MapToLoad, true);
      // Frames still queued belong to the world being left; NotifyEndEpisode drops them.
      return;
    }

    if (Action == carla::multigpu::PreTickAction::Skip)
    {
      // An engine frame on this server's own clock: no primary frame, so no
      // simulation time passes either.
      return;
    }

    std::optional<FFrameData> Frame;
    {
      std::scoped_lock<std::mutex> Lock(FrameToProcessMutex);
      if (!FramesToProcess.empty() && (FramesToProcess.front().GetFrameNumber() == *PreparedFrame))
      {
        Frame.emplace(std::move(FramesToProcess.front()));
        FramesToProcess.erase(FramesToProcess.begin());
        QueuedFrames = FramesToProcess.size();
      }
    }
    if (Frame)
    {
      // Played outside the lock, so the command thread is never held up by
      // a large resync.
      PlayPrimaryFrame(*World, DeltaSeconds, *Frame);
    }
    else if (!bWarnedUnpairedFrame)
    {
      bWarnedUnpairedFrame = true;
      UE_LOG(LogCarla, Warning, TEXT("Multi-GPU: primary frame %llu, prepared for this engine frame, is gone"),
          static_cast<uint64>(*PreparedFrame));
    }
    PreparedFrame.reset();
    return;
  }

  PendingFrameTime.reset();
  if (CurrentEpisode && !bSynchronousMode && SecondaryServer->HasClientsConnected())
  {
    // set synchronous mode
    CurrentSettings.bSynchronousMode = true;
    CurrentSettings.FixedDeltaSeconds = 1 / 20.0f;
    OnEpisodeSettingsChanged(CurrentSettings);
    CurrentEpisode->ApplySettings(CurrentSettings);
    Server.GetStreamingServer().SetSynchronousMode(true);
  }

  // process RPC commands
  do
  {
    Server.RunSome(1u);
  }
  while (bSynchronousMode && !Server.TickCueReceived());

  // load_world() only schedules the travel, for the next engine frame: this
  // frame still belongs to the world being left, and is never sent.
  bool bLeavingWorld = FCarlaEngine_IsWorldTravelPending(World);

  // Frames sent while a secondary is still loading are dropped there.
  if (bSynchronousMode && !bLeavingWorld)
  {
    WaitForSecondaryEpisodes();
    // Another client's load_world can be served while waiting.
    bLeavingWorld = FCarlaEngine_IsWorldTravelPending(World);
  }

  if (CurrentEpisode && (World == CurrentEpisode->GetWorld()) && !bLeavingWorld)
  {
    // The world's clocks before UWorld::Tick adds this frame's delta to them.
    const AWorldSettings *WorldSettings = World->GetWorldSettings();
    FPendingFrameTime Time;
    Time.DeltaSeconds = DeltaSeconds;
    Time.TimeSeconds = World->TimeSeconds;
    Time.UnpausedTimeSeconds = World->UnpausedTimeSeconds;
    Time.RealTimeSeconds = World->RealTimeSeconds;
    Time.AudioTimeSeconds = World->AudioTimeSeconds;
    Time.TimeDilation = WorldSettings ? WorldSettings->GetEffectiveTimeDilation() : 1.0f;
    PendingFrameTime = Time;
  }

  // update frame counter
  UpdateFrameCounter();

  if (CurrentEpisode)
  {
    CurrentEpisode->TickTimers(DeltaSeconds);
  }
}

void FCarlaEngine::OnBeginFrame()
{
  TRACE_CPUPROFILER_EVENT_SCOPE_STR(__FUNCTION__);
  bFramePlayedThisTick = false;
  PreparedFrame.reset();
  if (!Secondary)
  {
    return;
  }

  const bool bConnected = Secondary->IsConnected();
  if (bWasConnectedToPrimary && !bConnected)
  {
    carla::log_warning("multigpu: lost the connection to the primary; this secondary stops mirroring "
        "it and runs on its own clock until it is restarted");
  }
  bWasConnectedToPrimary = bConnected;

  if (bResyncRequested && bConnected && CurrentEpisode && SensorStreams.IsEpisodeOpen())
  {
    const double Now = FPlatformTime::Seconds();
    if (Now - LastResyncRequestSeconds >= 1.0)
    {
      LastResyncRequestSeconds = Now;
      bResyncRequested = false;
      UE_LOG(LogCarla, Log, TEXT("Multi-GPU: missed state of the primary; asking it for a resync"));
      SendEpisodeReady();
    }
  }

  SendEpisodeReadyWhenContentReady();

  carla::multigpu::BeginFrameState State;
  State.connected = bConnected;
  State.has_episode = (CurrentEpisode != nullptr);
  State.travel_pending = State.has_episode && FCarlaEngine_IsWorldTravelPending(CurrentEpisode->GetWorld());
  // While EPISODE_READY is held back, frames run on this server's own clock so the content can finish.
  State.load_map_pending = bLoadMapPending || bEpisodeReadyPending;
  State.exit_requested = IsEngineExitRequested();
  const carla::multigpu::BeginFrameAction Action = carla::multigpu::DecideBeginFrame(State);

  if (Action == carla::multigpu::BeginFrameAction::OwnClock)
  {
    // Not mirroring anything: serve RPCs on this server's own clock, without
    // spinning (RunSome returns at once when there is no work).
    Server.RunSome(1u);
    static constexpr double MinFrameSeconds = 1.0 / 50.0;
    const double Elapsed = FPlatformTime::Seconds() - LastOwnClockFrameSeconds;
    if (Elapsed < MinFrameSeconds)
    {
      FPlatformProcess::Sleep(static_cast<float>(MinFrameSeconds - Elapsed));
    }
    LastOwnClockFrameSeconds = FPlatformTime::Seconds();
    FApp::SetUseFixedTimeStep(false);
    return;
  }

  // Never wait without an episode or while a map load is pending or a travel
  // is scheduled: the primary waits for this server's EPISODE_READY, which is
  // only sent from NotifyBeginEpisode, inside the travel.
  const bool bCanWait = (Action == carla::multigpu::BeginFrameAction::Wait);
  std::optional<uint64_t> Frame;
  std::optional<float> Delta;
  do
  {
    Server.RunSome(1u);
    if (!bCanWait || bLoadMapPending || IsEngineExitRequested() || !Secondary->IsConnected())
    {
      break;
    }
    if (QueuedFrames.load() == 0u)
    {
      // RunSome returns at once when it has nothing to do: without this the
      // loop spins a core until the command thread queues a frame. A sleep on
      // the monotonic clock, unlike a timed condition wait with this libc++.
      FPlatformProcess::SleepNoStats(0.0005f);
      continue;
    }
    std::scoped_lock<std::mutex> Lock(FrameToProcessMutex);
    if (!FramesToProcess.empty())
    {
      Frame = FramesToProcess.front().GetFrameNumber();
      if (const carla::multigpu::FrameTime *Time = FramesToProcess.front().GetFrameTime())
      {
        Delta = Time->delta_seconds;
      }
    }
  }
  while (!Frame);

  // This engine frame's time step: UEngine::UpdateTimeAndHandleMaxTickRate,
  // which runs right after this delegate, turns it back into this very float,
  // so the world ticks with exactly the primary's delta.
  const bool bTimed = Delta && carla::multigpu::IsUsableDelta(*Delta);
  FApp::SetUseFixedTimeStep(bTimed);
  if (bTimed)
  {
    FApp::SetFixedDeltaTime(static_cast<double>(*Delta));
  }
  PreparedFrame = Frame;
}

void FCarlaEngine::PlayPrimaryFrame(UWorld &World, float DeltaSeconds, FFrameData &Frame)
{
  TRACE_CPUPROFILER_EVENT_SCOPE_STR(__FUNCTION__);
  const uint64_t FrameNumber = Frame.GetFrameNumber();
  const carla::multigpu::FrameTime *Time = Frame.GetFrameTime();
  if (Time)
  {
    if (DeltaSeconds != Time->delta_seconds)
    {
      if (DeltaMismatches++ == 0u)
      {
        carla::log_warning("multigpu: frame", FrameNumber, "ran with a", DeltaSeconds,
            "s time step instead of the primary's", Time->delta_seconds, "s");
      }
    }

    // The primary's world clocks before this frame's accumulation: UWorld::Tick
    // then adds the same float to the same doubles, so the world time materials,
    // animations and tick intervals see is the primary's, bit for bit.
    World.TimeSeconds = Time->world_time_seconds;
    World.UnpausedTimeSeconds = Time->world_unpaused_time_seconds;
    World.RealTimeSeconds = Time->world_real_time_seconds;
    World.AudioTimeSeconds = Time->world_audio_time_seconds;
    if (AWorldSettings *WorldSettings = World.GetWorldSettings())
    {
      if (WorldSettings->TimeDilation != Time->time_dilation)
      {
        WorldSettings->TimeDilation = Time->time_dilation;
      }
    }

    const bool bConsecutive = carla::multigpu::IsConsecutive(LastPlayedFrame, FrameNumber);
    if (carla::multigpu::NeedsResync(LastPlayedFrame, FrameNumber, Time->full_resync))
    {
      // A frame went missing: this server lacks whatever it carried.
      bResyncRequested = true;
    }
    if (!bConsecutive)
    {
      // What the primary's clocks read while it served the RPCs this frame
      // replays (spawns, attachments): before its TickTimers of this frame.
      CurrentEpisode->SetTimers(
          Time->elapsed_game_time - Time->delta_seconds,
          Time->visual_game_time - Time->delta_seconds);
      FCarlaEngine::ResetFrameCounter(FrameNumber - 1u);
      UE_LOG(LogCarla, Log, TEXT("Multi-GPU: following the primary's clock from frame %llu (elapsed %.6f s)"),
          static_cast<uint64>(FrameNumber), Time->elapsed_game_time);
    }
    MirrorPrimarySettings(*Time);
    if (!Frame.HasSensorTicks() && !bWarnedNoSensorTicks)
    {
      bWarnedNoSensorTicks = true;
      carla::log_warning("multigpu: the primary does not say which sensors tick; no sensor of this "
          "secondary will produce data. Every server must run the same CARLA build.");
    }
  }

  {
    TRACE_CPUPROFILER_EVENT_SCOPE_STR("FramesToProcess.PlayFrameData");
    // Ends with the frame counter set to the primary's frame number.
    Frame.PlayFrameData(
        CurrentEpisode,
        MappedId,
        MirrorState,
        [this](uint32_t PrimaryActorId, uint32_t LocalActorId, bool bCreated)
        {
          BindReplayedSensor(PrimaryActorId, LocalActorId, bCreated);
        },
        [this](uint32_t PrimaryActorId)
        {
          SensorStreams.Unbind(Server.GetStreamingServer(), PrimaryActorId);
        });
  }

  if (Time)
  {
    CurrentEpisode->SetTimers(Time->elapsed_game_time, Time->visual_game_time);
    LastPlayedFrame = FrameNumber;
  }
  else
  {
    if (!bWarnedUntimedFrame)
    {
      bWarnedUntimedFrame = true;
      carla::log_warning("multigpu: the primary sends frames without its clock (an older build?); "
          "this secondary stamps them with its own. Every server must run the same CARLA build.");
    }
    CurrentEpisode->TickTimers(DeltaSeconds);
    LastPlayedFrame.reset();
  }
  bFramePlayedThisTick = true;
}

void FCarlaEngine::RestorePlayedPoses()
{
  TRACE_CPUPROFILER_EVENT_SCOPE_STR(__FUNCTION__);
  for (const auto &Played : MirrorState.PlayedPoses)
  {
    FCarlaActor *CarlaActor = CurrentEpisode->FindCarlaActor(Played.first);
    if ((CarlaActor == nullptr) || CarlaActor->IsDormant() || (CarlaActor->GetActor() == nullptr))
    {
      continue;
    }
    const FTransform Current = CarlaActor->GetActorGlobalTransform();
    const FTransform &Pose = Played.second;
    // 1e-3 cm and about 1e-4 degrees
    const bool bMoved =
        !Current.GetLocation().Equals(Pose.GetLocation(), 1e-3) ||
        (Current.GetRotation().AngularDistance(Pose.GetRotation()) > FMath::DegreesToRadians(1e-4));
    if (!bMoved)
    {
      continue;
    }
    AActor *Actor = CarlaActor->GetActor();
    FTransform Restored = Pose;
    Restored.SetScale3D(Actor->GetActorScale3D());
    CarlaActor->SetActorGlobalTransform(Restored, ETeleportType::None);
    ++RestoredPoses;
    const FName ClassName = Actor->GetClass()->GetFName();
    if (!RestoredPoseClasses.Contains(ClassName))
    {
      RestoredPoseClasses.Add(ClassName);
      UE_LOG(LogCarla, Warning,
          TEXT("Multi-GPU: a %s mirror moved on its own between playing frame %llu and its capture; restored the primary's pose"),
          *ClassName.ToString(), static_cast<uint64>(FCarlaEngine::GetFrameCounter()));
    }
  }
}

void FCarlaEngine::MirrorPrimarySettings(const carla::multigpu::FrameTime &Time)
{
  FEpisodeSettings Settings = CurrentEpisode->GetSettings();
  SettingsFromFrameTime(Time, Settings);
  // A secondary plays the primary's frames, so it is synchronous whatever the
  // primary runs, and never renders its own viewport.
  Settings.bSynchronousMode = true;
  Settings.bNoRenderingMode = true;
  if (FCarlaEngine_SameSettings(Settings, CurrentEpisode->GetSettings()))
  {
    return;
  }
  CurrentEpisode->ApplySettings(Settings);
  // set_episode_settings applies these two itself; do the same here.
  if (ACarlaGameModeBase *GameMode = UCarlaStatics::GetGameMode(CurrentEpisode->GetWorld()))
  {
    if (ALargeMapManager *LargeMap = GameMode->GetLMManager())
    {
      LargeMap->ConsiderSpectatorAsEgo(Settings.SpectatorAsEgo);
    }
    if (ATrafficLightManager *TrafficLightManager = GameMode->GetTrafficLightManager())
    {
      TrafficLightManager->SetAdjustSignsHeightToGround(Settings.bAdjustSignsHeightToGround);
    }
  }
  UE_LOG(LogCarla, Log, TEXT("Multi-GPU: mirroring the primary's settings (fixed delta %s, substepping %s)"),
      Settings.FixedDeltaSeconds.IsSet() ? *FString::SanitizeFloat(Settings.FixedDeltaSeconds.GetValue()) : TEXT("none"),
      Settings.bSubstepping ? TEXT("on") : TEXT("off"));
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
    // First, so that the frame sent below carries what a replay just set
    // (actor poses, visual time), as the primary's own snapshot does.
    auto* EpisodeRecorder = GetCurrentEpisode()->GetRecorder();
    if (EpisodeRecorder)
    {
      EpisodeRecorder->Ticking(DeltaSeconds);
    }

    // Only frames whose start OnPreTick recorded, of the episode's world, and
    // never the frame that leaves it.
    if (bIsPrimaryServer && PendingFrameTime && SecondaryServer->HasClientsConnected() &&
        (World == GetCurrentEpisode()->GetWorld()) && !FCarlaEngine_IsWorldTravelPending(World))
    {
      const bool bWasNewConnection = bNewConnection.exchange(false);
      FFrameData &FrameData = GetCurrentEpisode()->GetFrameData();
      FrameData.GetFrameData(GetCurrentEpisode(), true, bWasNewConnection);

      carla::multigpu::FrameTime Time;
      Time.frame = FCarlaEngine::GetFrameCounter();
      Time.delta_seconds = PendingFrameTime->DeltaSeconds;
      Time.elapsed_game_time = GetCurrentEpisode()->GetElapsedGameTime();
      Time.visual_game_time = GetCurrentEpisode()->GetVisualGameTime();
      Time.world_time_seconds = PendingFrameTime->TimeSeconds;
      Time.world_unpaused_time_seconds = PendingFrameTime->UnpausedTimeSeconds;
      Time.world_real_time_seconds = PendingFrameTime->RealTimeSeconds;
      Time.world_audio_time_seconds = PendingFrameTime->AudioTimeSeconds;
      Time.time_dilation = PendingFrameTime->TimeDilation;
      Time.full_resync = bWasNewConnection;
      FrameTimeFromSettings(GetCurrentEpisode()->GetSettings(), Time);
      FrameData.SetFrameTime(Time);

      std::ostringstream OutStream;
      FrameData.Write(OutStream);

      // send frame data to secondary
      std::string Tmp(OutStream.str());
      SecondaryServer->GetCommander().SendFrameData(carla::Buffer(std::move((unsigned char *) Tmp.c_str()), (size_t) Tmp.size()));

#if !UE_BUILD_SHIPPING
      if (CVarCarlaMultiGpuVerifyFrames.GetValueOnGameThread() != 0)
      {
        FFrameData Check;
        const carla::multigpu::FrameDataReadResult Result =
            Check.Read(reinterpret_cast<const uint8_t *>(Tmp.data()), Tmp.size());
        if (!Result.IsComplete() || (Result.skipped > 0u) || (Result.grown > 0u))
        {
          UE_LOG(LogCarla, Error,
              TEXT("Multi-GPU: the frame sent to the secondaries does not read back: %s at packet %u (offset %llu), %u unknown and %u longer packets"),
              UTF8_TO_TCHAR(carla::multigpu::FrameDataStatusName(Result.status)),
              static_cast<uint32>(Result.packet_id), static_cast<uint64>(Result.offset),
              Result.skipped, Result.grown);
        }
      }
#endif // !UE_BUILD_SHIPPING

      // Game thread, like the get_sensor_token RPC: it must not run inside a
      // Router callback, and the secondary answers GET_TOKEN on its command
      // thread, so this cannot wait on either game thread.
      if (bWasNewConnection)
      {
        const std::size_t Rerouted = SecondaryServer->GetCommander().RerouteLostSensors();
        if (Rerouted > 0u)
        {
          UE_LOG(LogCarla, Log, TEXT("Multi-GPU: routed %d sensors of a lost secondary to a new one"),
              static_cast<int32>(Rerouted));
        }
      }
    }

    // Every server records its own events here, with or without secondaries
    // (a secondary never sends them), so they never pile up.
    GetCurrentEpisode()->GetFrameData().Clear();
  }
  PendingFrameTime.reset();

  // A secondary only captures and broadcasts frames that carry the primary's
  // state, on the world it was played on.
  const bool bCaptureThisFrame = bIsPrimaryServer ||
      (bFramePlayedThisTick && CurrentEpisode && (World == CurrentEpisode->GetWorld()));
  if ((TickType == ELevelTick::LEVELTICK_All) && (CurrentEpisode != nullptr) && bCaptureThisFrame)
  {
    if (!bIsPrimaryServer)
    {
      RestorePlayedPoses();
    }

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
  bFramePlayedThisTick = false;
}

void FCarlaEngine::OnPreWorldInitialization(UWorld *World, const UWorld::InitializationValues IVS)
{
  // Chaos rebuilds its dynamic BVH tree in an async task and swaps it in only
  // when that task has finished, so the frame a contact is first detected
  // depends on wall-clock timing. Its grid has no such history. Chaos reads
  // this cvar when it builds a world's acceleration structures, and changing
  // it while they exist breaks the swap (a check in
  // CopyUnBuiltDynamicAccelerationStructures), hence here and not on
  // apply_settings.
  static const TCHAR *CVarName = TEXT("p.Chaos.AccelerationStructureUseDynamicTree");
  IConsoleVariable *CVar = IConsoleManager::Get().FindConsoleVariable(CVarName);
  if (CVar == nullptr)
  {
    if (CurrentSettings.bDeterministicCollisions)
    {
      UE_LOG(LogCarla, Warning, TEXT("%s not found: collisions are not deterministic"), CVarName);
    }
    return;
  }
  if (CurrentSettings.bDeterministicCollisions)
  {
    if (!DynamicTreeBeforeDeterministicCollisions.IsSet())
    {
      DynamicTreeBeforeDeterministicCollisions = CVar->GetInt();
    }
    CVar->Set(0, ECVF_SetByCode);
  }
  else if (DynamicTreeBeforeDeterministicCollisions.IsSet())
  {
    CVar->Set(DynamicTreeBeforeDeterministicCollisions.GetValue(), ECVF_SetByCode);
    DynamicTreeBeforeDeterministicCollisions.Reset();
  }
}

void FCarlaEngine::OnEpisodeSettingsChanged(const FEpisodeSettings &Settings)
{
  CurrentSettings = FEpisodeSettings(Settings);

  // A secondary plays the primary's frames: it is synchronous whatever it is told.
  const bool bWasSynchronous = bSynchronousMode;
  bSynchronousMode = bIsPrimaryServer ? Settings.bSynchronousMode : true;

  // Tick cues sent while asynchronous were never consumed: entering synchronous
  // mode with them banked would run that many frames without any client
  // ticking, which breaks determinism. Start synchronous mode from zero, as a
  // fresh server does.
  if (bIsPrimaryServer && !bWasSynchronous && bSynchronousMode)
  {
    Server.ResetTickCues();
  }

#if WITH_EDITOR
  if (GEngine && GEngine->GameViewport)
  {
    // -RenderOffScreen keeps the invisible main viewport's world render off
    // regardless of the episode's no-rendering setting (see OnPostTick).
    GEngine->GameViewport->bDisableWorldRendering = Settings.bNoRenderingMode ||
        FParse::Param(FCommandLine::Get(), TEXT("RenderOffScreen"));
  }
#endif
  if (bIsPrimaryServer)
  {
    FCarlaEngine_SetFixedDeltaSeconds(Settings.FixedDeltaSeconds);
  }
  else
  {
    // OnBeginFrame sets this server's time step from every primary frame. Its
    // sensor streams wait for slow clients instead of dropping their data.
    Server.GetStreamingServer().SetSynchronousMode(true);
  }

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
