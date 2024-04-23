// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "Carla.h"
#include "Carla/Game/CarlaEngine.h"

#include "Carla/Game/CarlaEpisode.h"
#include "Carla/Game/CarlaStaticDelegates.h"
#include "Carla/Game/CarlaStatics.h"
#include "Carla/Lights/CarlaLightSubsystem.h"
#include "Carla/Recorder/CarlaRecorder.h"
#include "Carla/Settings/CarlaSettings.h"
#include "Carla/Settings/EpisodeSettings.h"

#include "Runtime/Core/Public/Misc/App.h"
#include "PhysicsEngine/PhysicsSettings.h"
#include "Carla/MapGen/LargeMapManager.h"

#include <compiler/disable-ue4-macros.h>
#include <carla/Logging.h>
#include <carla/multigpu/primaryCommands.h>
#include <carla/multigpu/commands.h>
#include <carla/multigpu/secondary.h>
#include <carla/multigpu/secondaryCommands.h>
#include <carla/ros2/ROS2.h>
#include <carla/streaming/EndPoint.h>
#include <carla/streaming/Server.h>
#include <compiler/enable-ue4-macros.h>

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

    auto BroadcastStream     = Server.Start(Settings.RPCPort, StreamingPort, SecondaryPort);
    Server.AsyncRun(FCarlaEngine_GetNumberOfThreadsForRPCServer());

    WorldObserver.SetStream(BroadcastStream);

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
      auto CommandExecutor = [=](carla::multigpu::MultiGPUCommand Id, carla::Buffer Data) {
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
                std::lock_guard<std::mutex> Lock(FrameToProcessMutex);
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
    auto ROS2 = carla::ros2::ROS2::GetInstance();
    ROS2->Enable(true);
  }
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
          std::lock_guard<std::mutex> Lock(FrameToProcessMutex);
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

  if (GEngine && GEngine->GameViewport)
  {
    GEngine->GameViewport->bDisableWorldRendering = Settings.bNoRenderingMode;
  }

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
