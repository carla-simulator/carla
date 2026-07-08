// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "Carla/Game/CarlaEngine.h"
#include "Carla.h"
#include "Carla/Game/CarlaEpisode.h"
#include "Carla/Game/CarlaStaticDelegates.h"
#include "Carla/Game/CarlaStatics.h"
#include "Carla/Lights/CarlaLightSubsystem.h"
#include "Carla/Recorder/CarlaRecorder.h"
#include "Carla/Settings/CarlaSettings.h"
#include "Carla/Settings/EpisodeSettings.h"
#include "Carla/MapGen/LargeMapManager.h"

#include <util/disable-ue4-macros.h>
#include <carla/Logging.h>
#include <carla/multigpu/primaryCommands.h>
#include <carla/multigpu/commands.h>
#include <carla/multigpu/secondary.h>
#include <carla/multigpu/secondaryCommands.h>
#include <carla/ros2/ROS2.h>
#include <carla/ros2/middleware/Middleware.h>
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

      // define the commands executor (when a command comes from the primary server)
      auto CommandExecutor = [=, this](carla::multigpu::MultiGPUCommand Id, carla::Buffer Data) {
        struct CarlaStreamBuffer : public std::streambuf
        {
            CarlaStreamBuffer(char *buf, std::size_t size) { setg(buf, buf, buf + size); }
        };
        switch (Id) {
          case carla::multigpu::MultiGPUCommand::SEND_FRAME:
          {
            if(GetCurrentEpisode())
            {
              TRACE_CPUPROFILER_EVENT_SCOPE_STR("MultiGPUCommand::SEND_FRAME");
              // convert frame data from buffer to istream
              CarlaStreamBuffer TempStream((char *) Data.data(), Data.size());
              std::istream InStream(&TempStream);
              GetCurrentEpisode()->GetFrameData().Read(InStream);
              {
                TRACE_CPUPROFILER_EVENT_SCOPE_STR("FramesToProcess.emplace_back");
                std::scoped_lock<std::mutex> Lock(FrameToProcessMutex);
                FramesToProcess.emplace_back(GetCurrentEpisode()->GetFrameData());
              }
            }
            // forces a tick
            Server.Tick();
            break;
          }
          case carla::multigpu::MultiGPUCommand::LOAD_MAP:
          {
            FString FinalPath((char *) Data.data());
            UGameplayStatics::OpenLevel(CurrentEpisode->GetWorld(), *FinalPath, true);
            break;
          }
          case carla::multigpu::MultiGPUCommand::GET_TOKEN:
          {
            // get the sensor id
            auto sensor_id = *(reinterpret_cast<carla::streaming::detail::stream_id_type *>(Data.data()));
            // query dispatcher
            carla::streaming::detail::token_type token(Server.GetStreamingServer().GetToken(sensor_id));
            carla::Buffer buf(reinterpret_cast<unsigned char *>(&token), (size_t) sizeof(token));
            carla::log_info("responding with a token for port ", token.get_port());
            Secondary->Write(std::move(buf));
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
            // get the sensor id
            auto sensor_id = *(reinterpret_cast<carla::streaming::detail::stream_id_type *>(Data.data()));
            // query dispatcher
            Server.GetStreamingServer().EnableForROS(sensor_id);
            // return a 'true'
            bool res = true;
            carla::Buffer buf(reinterpret_cast<unsigned char *>(&res), (size_t) sizeof(bool));
            carla::log_info("responding ENABLE_ROS with a true");
            Secondary->Write(std::move(buf));
            break;
          }
          case carla::multigpu::MultiGPUCommand::DISABLE_ROS:
          {
            // get the sensor id
            auto sensor_id = *(reinterpret_cast<carla::streaming::detail::stream_id_type *>(Data.data()));
            // query dispatcher
            Server.GetStreamingServer().DisableForROS(sensor_id);
            // return a 'true'
            bool res = true;
            carla::Buffer buf(reinterpret_cast<unsigned char *>(&res), (size_t) sizeof(bool));
            carla::log_info("responding DISABLE_ROS with a true");
            Secondary->Write(std::move(buf));
            break;
          }
          case carla::multigpu::MultiGPUCommand::IS_ENABLED_ROS:
          {
            // get the sensor id
            auto sensor_id = *(reinterpret_cast<carla::streaming::detail::stream_id_type *>(Data.data()));
            // query dispatcher
            bool res = Server.GetStreamingServer().IsEnabledForROS(sensor_id);
            carla::Buffer buf(reinterpret_cast<unsigned char *>(&res), (size_t) sizeof(bool));
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
    else if (!ROS2->Enable(true, Parsed.middleware))
    {
      UE_LOG(LogCarla, Error,
          TEXT("ROS2: --rmw='%s' is not compiled into this binary. Available: %s. ROS2 is DISABLED for this session."),
          *Settings.RmwName,
          UTF8_TO_TCHAR(carla::ros2::GetAvailableMiddleware().c_str()));
    }
    else
    {
      UE_LOG(LogCarla, Log, TEXT("ROS2: enabled with middleware '%s'."), *Settings.RmwName);
      // Apply the configured default topic visibility before any sensor stream is
      // created. Gated on Settings.ROS2 so non-ROS2 runs never force every stream
      // active (which would make every sensor produce data each tick).
      Server.GetStreamingServer().SetROS2TopicVisibilityDefaultEnabled(Settings.ROS2TopicVisibility);
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
  CurrentEpisode = &Episode;

  // Reset map settings
  UWorld* World = CurrentEpisode->GetWorld();
  ALargeMapManager* LargeMapManager = UCarlaStatics::GetLargeMapManager(World);
  if (LargeMapManager)
  {
    CurrentSettings.TileStreamingDistance = LargeMapManager->GetLayerStreamingDistance();
    CurrentSettings.ActorActiveDistance = LargeMapManager->GetActorStreamingDistance();
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
}

void FCarlaEngine::NotifyEndEpisode()
{
  Server.NotifyEndEpisode();
  CurrentEpisode = nullptr;
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
    }
    else
    {
      // process frame data
      do
      {
        Server.RunSome(1u);
      }
      while (!FramesToProcess.size());
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
          FramesToProcess.front().PlayFrameData(CurrentEpisode, MappedId);
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
        GetCurrentEpisode()->GetFrameData().GetFrameData(GetCurrentEpisode(), true, bNewConnection);
        bNewConnection = false;
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
