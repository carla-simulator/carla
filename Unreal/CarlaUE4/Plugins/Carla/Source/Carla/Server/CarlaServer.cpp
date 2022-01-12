// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "Carla.h"
#include "Carla/Server/CarlaServer.h"
#include "Carla/Traffic/TrafficLightGroup.h"
#include "EngineUtils.h"

#include "Carla/OpenDrive/OpenDrive.h"
#include "Carla/Util/DebugShapeDrawer.h"
#include "Carla/Util/NavigationMesh.h"
#include "Carla/Util/RayTracer.h"
#include "Carla/Vehicle/CarlaWheeledVehicle.h"
#include "Carla/Walker/WalkerController.h"
#include "Carla/Walker/WalkerBase.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Carla/Game/Tagger.h"
#include "Carla/Vehicle/MovementComponents/CarSimManagerComponent.h"
#include "Carla/Vehicle/MovementComponents/ChronoMovementComponent.h"
#include "Carla/Lights/CarlaLightSubsystem.h"
#include "Carla/Actor/ActorData.h"
#include "CarlaServerResponse.h"
#include "Carla/Util/BoundingBoxCalculator.h"
#include "Misc/FileHelper.h"

#include <compiler/disable-ue4-macros.h>
#include <carla/Functional.h>
#include <carla/Version.h>
#include <carla/rpc/Actor.h>
#include <carla/rpc/ActorDefinition.h>
#include <carla/rpc/ActorDescription.h>
#include <carla/rpc/BoneTransformDataIn.h>
#include <carla/rpc/Command.h>
#include <carla/rpc/CommandResponse.h>
#include <carla/rpc/DebugShape.h>
#include <carla/rpc/EnvironmentObject.h>
#include <carla/rpc/EpisodeInfo.h>
#include <carla/rpc/EpisodeSettings.h>
#include "carla/rpc/LabelledPoint.h"
#include <carla/rpc/LightState.h>
#include <carla/rpc/MapInfo.h>
#include <carla/rpc/MapLayer.h>
#include <carla/rpc/Response.h>
#include <carla/rpc/Server.h>
#include <carla/rpc/String.h>
#include <carla/rpc/Transform.h>
#include <carla/rpc/Vector2D.h>
#include <carla/rpc/Vector3D.h>
#include <carla/rpc/VehicleDoor.h>
#include <carla/rpc/VehicleControl.h>
#include <carla/rpc/VehiclePhysicsControl.h>
#include <carla/rpc/VehicleLightState.h>
#include <carla/rpc/VehicleLightStateList.h>
#include <carla/rpc/WalkerBoneControlIn.h>
#include <carla/rpc/WalkerBoneControlOut.h>
#include <carla/rpc/WalkerControl.h>
#include <carla/rpc/VehicleWheels.h>
#include <carla/rpc/WeatherParameters.h>
#include <carla/streaming/Server.h>
#include <carla/rpc/Texture.h>
#include <carla/rpc/MaterialParameter.h>
#include <compiler/enable-ue4-macros.h>

#include <vector>
#include <map>
#include <tuple>

template <typename T>
using R = carla::rpc::Response<T>;

// =============================================================================
// -- Static local functions ---------------------------------------------------
// =============================================================================

template <typename T, typename Other>
static std::vector<T> MakeVectorFromTArray(const TArray<Other> &Array)
{
  return {Array.GetData(), Array.GetData() + Array.Num()};
}

// =============================================================================
// -- FCarlaServer::FPimpl -----------------------------------------------
// =============================================================================

class FCarlaServer::FPimpl
{
public:

  FPimpl(uint16_t RPCPort, uint16_t StreamingPort)
    : Server(RPCPort),
      StreamingServer(StreamingPort),
      BroadcastStream(StreamingServer.MakeStream())
  {
    BindActions();
  }

  /// Map of pairs < port , ip > with all the Traffic Managers active in the simulation
  std::map<uint16_t, std::string> TrafficManagerInfo;

  carla::rpc::Server Server;

  carla::streaming::Server StreamingServer;

  carla::streaming::Stream BroadcastStream;

  UCarlaEpisode *Episode = nullptr;

  size_t TickCuesReceived = 0u;

private:

  void BindActions();
};

// =============================================================================
// -- Define helper macros -----------------------------------------------------
// =============================================================================

#if WITH_EDITOR
#  define CARLA_ENSURE_GAME_THREAD() check(IsInGameThread());
#else
#  define CARLA_ENSURE_GAME_THREAD()
#endif // WITH_EDITOR

#define RESPOND_ERROR(str) {                                              \
    UE_LOG(LogCarlaServer, Log, TEXT("Responding error: %s"), TEXT(str)); \
    return carla::rpc::ResponseError(str); }

#define RESPOND_ERROR_FSTRING(fstr) {                                 \
    UE_LOG(LogCarlaServer, Log, TEXT("Responding error: %s"), *fstr); \
    return carla::rpc::ResponseError(carla::rpc::FromFString(fstr)); }

#define REQUIRE_CARLA_EPISODE() \
    CARLA_ENSURE_GAME_THREAD();   \
    if (Episode == nullptr) { RESPOND_ERROR("episode not ready"); }

carla::rpc::ResponseError RespondError(
    const FString& FuncName,
    const FString& ErrorMessage,
    const FString& ExtraInfo = "")
{
  FString TotalMessage = "Responding error from function " + FuncName + ": " +
      ErrorMessage + ". " + ExtraInfo;
  UE_LOG(LogCarlaServer, Log, TEXT("%s"), *TotalMessage);
  return carla::rpc::ResponseError(carla::rpc::FromFString(TotalMessage));
}

carla::rpc::ResponseError RespondError(
    const FString& FuncName,
    const ECarlaServerResponse& Error,
    const FString& ExtraInfo = "")
{
  return RespondError(FuncName, GetStringError(Error), ExtraInfo);
}

class ServerBinder
{
public:

  constexpr ServerBinder(const char *name, carla::rpc::Server &srv, bool sync)
    : _name(name),
      _server(srv),
      _sync(sync) {}

  template <typename FuncT>
  auto operator<<(FuncT func)
  {
    if (_sync)
    {
      _server.BindSync(_name, func);
    }
    else
    {
      _server.BindAsync(_name, func);
    }
    return func;
  }

private:

  const char *_name;

  carla::rpc::Server &_server;

  bool _sync;
};

#define BIND_SYNC(name)   auto name = ServerBinder(# name, Server, true)
#define BIND_ASYNC(name)  auto name = ServerBinder(# name, Server, false)

// =============================================================================
// -- Bind Actions -------------------------------------------------------------
// =============================================================================

void FCarlaServer::FPimpl::BindActions()
{
  namespace cr = carla::rpc;
  namespace cg = carla::geom;

  /// Looks for a Traffic Manager running on port
  BIND_SYNC(is_traffic_manager_running) << [this] (uint16_t port) ->R<bool>
  {
    return (TrafficManagerInfo.find(port) != TrafficManagerInfo.end());
  };

  /// Gets a pair filled with the <IP, port> of the Trafic Manager running on port.
  /// If there is no Traffic Manager running the pair will be ("", 0)
  BIND_SYNC(get_traffic_manager_running) << [this] (uint16_t port) ->R<std::pair<std::string, uint16_t>>
  {
    auto it = TrafficManagerInfo.find(port);
    if(it != TrafficManagerInfo.end()) {
      return std::pair<std::string, uint16_t>(it->second, it->first);
    }
    return std::pair<std::string, uint16_t>("",0);
  };

  /// Add a new Traffic Manager running on <IP, port>
  BIND_SYNC(add_traffic_manager_running) << [this] (std::pair<std::string, uint16_t> trafficManagerInfo) ->R<bool>
  {
    uint16_t port = trafficManagerInfo.second;
    auto it = TrafficManagerInfo.find(port);
    if(it == TrafficManagerInfo.end()) {
      TrafficManagerInfo.insert(
        std::pair<uint16_t, std::string>(port, trafficManagerInfo.first));
      return true;
    }
    return false;

  };

  BIND_SYNC(destroy_traffic_manager) << [this] (uint16_t port) ->R<bool>
  {
    auto it = TrafficManagerInfo.find(port);
    if(it != TrafficManagerInfo.end()) {
      TrafficManagerInfo.erase(it);
      return true;
    }
    return false;
  };

  BIND_ASYNC(version) << [] () -> R<std::string>
  {
    return carla::version();
  };

  // ~~ Tick ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

  BIND_SYNC(tick_cue) << [this]() -> R<uint64_t>
  {
    TRACE_CPUPROFILER_EVENT_SCOPE(TickCueReceived);
    ++TickCuesReceived;
    return FCarlaEngine::GetFrameCounter();
  };

  // ~~ Load new episode ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

  BIND_ASYNC(get_available_maps) << [this]() -> R<std::vector<std::string>>
  {
    const auto MapNames = UCarlaStatics::GetAllMapNames();
    std::vector<std::string> result;
    result.reserve(MapNames.Num());
    for (const auto &MapName : MapNames)
    {
      if (MapName.Contains("/Sublevels/"))
        continue;
      if (MapName.Contains("/BaseMap/"))
        continue;
      if (MapName.Contains("/BaseLargeMap/"))
        continue;
      if (MapName.Contains("_Tile_"))
        continue;

      result.emplace_back(cr::FromFString(MapName));
    }
    return result;
  };

  BIND_SYNC(load_new_episode) << [this](const std::string &map_name, const bool reset_settings, cr::MapLayer MapLayers) -> R<void>
  {
    REQUIRE_CARLA_EPISODE();

    UCarlaGameInstance* GameInstance = UCarlaStatics::GetGameInstance(Episode->GetWorld());
    if (!GameInstance)
    {
      RESPOND_ERROR("unable to find CARLA game instance");
    }
    GameInstance->SetMapLayer(static_cast<int32>(MapLayers));

    if(!Episode->LoadNewEpisode(cr::ToFString(map_name), reset_settings))
    {
      RESPOND_ERROR("map not found");
    }

    return R<void>::Success();
  };

  BIND_SYNC(load_map_layer) << [this](cr::MapLayer MapLayers) -> R<void>
  {
    REQUIRE_CARLA_EPISODE();

    ACarlaGameModeBase* GameMode = UCarlaStatics::GetGameMode(Episode->GetWorld());
    if (!GameMode)
    {
      RESPOND_ERROR("unable to find CARLA game mode");
    }
    GameMode->LoadMapLayer(static_cast<int32>(MapLayers));

    return R<void>::Success();
  };

  BIND_SYNC(unload_map_layer) << [this](cr::MapLayer MapLayers) -> R<void>
  {
    REQUIRE_CARLA_EPISODE();

    ACarlaGameModeBase* GameMode = UCarlaStatics::GetGameMode(Episode->GetWorld());
    if (!GameMode)
    {
      RESPOND_ERROR("unable to find CARLA game mode");
    }
    GameMode->UnLoadMapLayer(static_cast<int32>(MapLayers));

    return R<void>::Success();
  };

  BIND_SYNC(copy_opendrive_to_file) << [this](const std::string &opendrive, cr::OpendriveGenerationParameters Params) -> R<void>
  {
    REQUIRE_CARLA_EPISODE();
    if (!Episode->LoadNewOpendriveEpisode(cr::ToLongFString(opendrive), Params))
    {
      RESPOND_ERROR("opendrive could not be correctly parsed");
    }
    return R<void>::Success();
  };

  BIND_SYNC(apply_color_texture_to_objects) << [this](
      const std::vector<std::string> &actors_name,
      const cr::MaterialParameter& parameter,
      const cr::TextureColor& Texture) -> R<void>
  {
    REQUIRE_CARLA_EPISODE();
    ACarlaGameModeBase* GameMode = UCarlaStatics::GetGameMode(Episode->GetWorld());
    if (!GameMode)
    {
      RESPOND_ERROR("unable to find CARLA game mode");
    }
    TArray<AActor*> ActorsToPaint;
    for(const std::string& actor_name : actors_name)
    {
      AActor* ActorToPaint = GameMode->FindActorByName(cr::ToFString(actor_name));
      if (ActorToPaint)
      {
        ActorsToPaint.Add(ActorToPaint);
      }
    }

    if(!ActorsToPaint.Num())
    {
      RESPOND_ERROR("unable to find Actor to apply the texture");
    }

    UTexture2D* UETexture = GameMode->CreateUETexture(Texture);

    for(AActor* ActorToPaint : ActorsToPaint)
    {
      GameMode->ApplyTextureToActor(
          ActorToPaint,
          UETexture,
          parameter);
    }
    return R<void>::Success();
  };

  BIND_SYNC(apply_float_color_texture_to_objects) << [this](
      const std::vector<std::string> &actors_name,
      const cr::MaterialParameter& parameter,
      const cr::TextureFloatColor& Texture) -> R<void>
  {
    REQUIRE_CARLA_EPISODE();
    ACarlaGameModeBase* GameMode = UCarlaStatics::GetGameMode(Episode->GetWorld());
    if (!GameMode)
    {
      RESPOND_ERROR("unable to find CARLA game mode");
    }
    TArray<AActor*> ActorsToPaint;
    for(const std::string& actor_name : actors_name)
    {
      AActor* ActorToPaint = GameMode->FindActorByName(cr::ToFString(actor_name));
      if (ActorToPaint)
      {
        ActorsToPaint.Add(ActorToPaint);
      }
    }

    if(!ActorsToPaint.Num())
    {
      RESPOND_ERROR("unable to find Actor to apply the texture");
    }

    UTexture2D* UETexture = GameMode->CreateUETexture(Texture);

    for(AActor* ActorToPaint : ActorsToPaint)
    {
      GameMode->ApplyTextureToActor(
          ActorToPaint,
          UETexture,
          parameter);
    }
    return R<void>::Success();
  };

  BIND_SYNC(get_names_of_all_objects) << [this]() -> R<std::vector<std::string>>
  {
    REQUIRE_CARLA_EPISODE();
    ACarlaGameModeBase* GameMode = UCarlaStatics::GetGameMode(Episode->GetWorld());
    if (!GameMode)
    {
      RESPOND_ERROR("unable to find CARLA game mode");
    }
    TArray<FString> NamesFString = GameMode->GetNamesOfAllActors();
    std::vector<std::string> NamesStd;
    for (const FString &Name : NamesFString)
    {
      NamesStd.emplace_back(cr::FromFString(Name));
    }
    return NamesStd;
  };

  // ~~ Episode settings and info ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

  BIND_SYNC(get_episode_info) << [this]() -> R<cr::EpisodeInfo>
  {
    REQUIRE_CARLA_EPISODE();
    return cr::EpisodeInfo{Episode->GetId(), BroadcastStream.token()};
  };

  BIND_SYNC(get_map_info) << [this]() -> R<cr::MapInfo>
  {
    REQUIRE_CARLA_EPISODE();
    ACarlaGameModeBase* GameMode = UCarlaStatics::GetGameMode(Episode->GetWorld());
    const auto &SpawnPoints = Episode->GetRecommendedSpawnPoints();
    FString FullMapPath = GameMode->GetFullMapPath();
    FString MapDir = FullMapPath.RightChop(FullMapPath.Find("Content/", ESearchCase::CaseSensitive) + 8);
    MapDir += "/" + Episode->GetMapName();
    return cr::MapInfo{
      cr::FromFString(MapDir),
      MakeVectorFromTArray<cg::Transform>(SpawnPoints)};
  };

  BIND_SYNC(get_map_data) << [this]() -> R<std::string>
  {
    REQUIRE_CARLA_EPISODE();
    return cr::FromLongFString(UOpenDrive::GetXODR(Episode->GetWorld()));
  };

  BIND_SYNC(get_navigation_mesh) << [this]() -> R<std::vector<uint8_t>>
  {
    REQUIRE_CARLA_EPISODE();
    auto FileContents = FNavigationMesh::Load(Episode->GetMapName());
    // make a mem copy (from TArray to std::vector)
    std::vector<uint8_t> Result(FileContents.Num());
    memcpy(&Result[0], FileContents.GetData(), FileContents.Num());
    return Result;
  };

  BIND_SYNC(get_required_files) << [this](std::string folder = "") -> R<std::vector<std::string>>
  {
    REQUIRE_CARLA_EPISODE();

    // Check that the path ends in a slash, add it otherwise
    if (folder[folder.size() - 1] != '/' && folder[folder.size() - 1] != '\\') {
      folder += "/";
    }

    // Get the map's folder absolute path and check if it's in its own folder
    ACarlaGameModeBase* GameMode = UCarlaStatics::GetGameMode(Episode->GetWorld());
    const auto mapDir = GameMode->GetFullMapPath();
    const auto folderDir = mapDir + "/" + folder.c_str();
    const auto fileName = mapDir.EndsWith(Episode->GetMapName()) ? "*" : Episode->GetMapName();

    // Find all the xodr and bin files from the map
    TArray<FString> Files;
    IFileManager::Get().FindFilesRecursive(Files, *folderDir, *(fileName + ".xodr"), true, false, false);
    IFileManager::Get().FindFilesRecursive(Files, *folderDir, *(fileName + ".bin"), true, false, false);

    // Remove the start of the path until the content folder and put each file in the result
    std::vector<std::string> result;
    for (auto File : Files) {
      File.RemoveFromStart(FPaths::ConvertRelativePathToFull(FPaths::ProjectContentDir()));
      result.emplace_back(TCHAR_TO_UTF8(*File));
    }

    return result;
  };
  BIND_SYNC(request_file) << [this](std::string name) -> R<std::vector<uint8_t>>
  {
    REQUIRE_CARLA_EPISODE();

    // Get the absolute path of the file
    FString path(FPaths::ConvertRelativePathToFull(FPaths::ProjectContentDir()));
    path.Append(name.c_str());

    // Copy the binary data of the file into the result and return it
    TArray<uint8_t> Content;
    FFileHelper::LoadFileToArray(Content, *path, 0);
    std::vector<uint8_t> Result(Content.Num());
    memcpy(&Result[0], Content.GetData(), Content.Num());

    return Result;
  };

  BIND_SYNC(get_episode_settings) << [this]() -> R<cr::EpisodeSettings>
  {
    REQUIRE_CARLA_EPISODE();
    return cr::EpisodeSettings{Episode->GetSettings()};
  };

  BIND_SYNC(set_episode_settings) << [this](
      const cr::EpisodeSettings &settings) -> R<uint64_t>
  {
    REQUIRE_CARLA_EPISODE();
    Episode->ApplySettings(settings);
    StreamingServer.SetSynchronousMode(settings.synchronous_mode);
    return FCarlaEngine::GetFrameCounter();
  };

  BIND_SYNC(get_actor_definitions) << [this]() -> R<std::vector<cr::ActorDefinition>>
  {
    REQUIRE_CARLA_EPISODE();
    return MakeVectorFromTArray<cr::ActorDefinition>(Episode->GetActorDefinitions());
  };

  BIND_SYNC(get_spectator) << [this]() -> R<cr::Actor>
  {
    REQUIRE_CARLA_EPISODE();
    FCarlaActor* CarlaActor = Episode->FindCarlaActor(Episode->GetSpectatorPawn());
    if (!CarlaActor)
    {
      RESPOND_ERROR("internal error: unable to find spectator");
    }
    return Episode->SerializeActor(CarlaActor);
  };

  BIND_SYNC(get_all_level_BBs) << [this](uint8 QueriedTag) -> R<std::vector<cg::BoundingBox>>
  {
    REQUIRE_CARLA_EPISODE();
    TArray<FBoundingBox> Result;
    ACarlaGameModeBase* GameMode = UCarlaStatics::GetGameMode(Episode->GetWorld());
    if (!GameMode)
    {
      RESPOND_ERROR("unable to find CARLA game mode");
    }
    Result = GameMode->GetAllBBsOfLevel(QueriedTag);
    ALargeMapManager* LargeMap = GameMode->GetLMManager();
    if (LargeMap)
    {
      for(auto& Box : Result)
      {
        Box.Origin = LargeMap->LocalToGlobalLocation(Box.Origin);
      }
    }
    return MakeVectorFromTArray<cg::BoundingBox>(Result);
  };

  BIND_SYNC(get_environment_objects) << [this](uint8 QueriedTag) -> R<std::vector<cr::EnvironmentObject>>
  {
    REQUIRE_CARLA_EPISODE();
    ACarlaGameModeBase* GameMode = UCarlaStatics::GetGameMode(Episode->GetWorld());
    if (!GameMode)
    {
      RESPOND_ERROR("unable to find CARLA game mode");
    }
    TArray<FEnvironmentObject> Result = GameMode->GetEnvironmentObjects(QueriedTag);
    ALargeMapManager* LargeMap = GameMode->GetLMManager();
    if (LargeMap)
    {
      for(auto& Object : Result)
      {
        Object.Transform = LargeMap->LocalToGlobalTransform(Object.Transform);
      }
    }
    return MakeVectorFromTArray<cr::EnvironmentObject>(Result);
  };

  BIND_SYNC(enable_environment_objects) << [this](std::vector<uint64_t> EnvObjectIds, bool Enable) -> R<void>
  {
    REQUIRE_CARLA_EPISODE();
    ACarlaGameModeBase* GameMode = UCarlaStatics::GetGameMode(Episode->GetWorld());
    if (!GameMode)
    {
      RESPOND_ERROR("unable to find CARLA game mode");
    }

    TSet<uint64> EnvObjectIdsSet;
    for(uint64 Id : EnvObjectIds)
    {
      EnvObjectIdsSet.Emplace(Id);
    }

    GameMode->EnableEnvironmentObjects(EnvObjectIdsSet, Enable);
    return R<void>::Success();
  };

  // ~~ Weather ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

  BIND_SYNC(get_weather_parameters) << [this]() -> R<cr::WeatherParameters>
  {
    REQUIRE_CARLA_EPISODE();
    auto *Weather = Episode->GetWeather();
    if (Weather == nullptr)
    {
      RESPOND_ERROR("internal error: unable to find weather");
    }
    return Weather->GetCurrentWeather();
  };

  BIND_SYNC(set_weather_parameters) << [this](
      const cr::WeatherParameters &weather) -> R<void>
  {
    REQUIRE_CARLA_EPISODE();
    auto *Weather = Episode->GetWeather();
    if (Weather == nullptr)
    {
      RESPOND_ERROR("internal error: unable to find weather");
    }
    Weather->ApplyWeather(weather);
    return R<void>::Success();
  };

  // ~~ Actor operations ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

  BIND_SYNC(get_actors_by_id) << [this](
      const std::vector<FCarlaActor::IdType> &ids) -> R<std::vector<cr::Actor>>
  {
    REQUIRE_CARLA_EPISODE();
    std::vector<cr::Actor> Result;
    Result.reserve(ids.size());
    for (auto &&Id : ids)
    {
      FCarlaActor* View = Episode->FindCarlaActor(Id);
      if (View)
      {
        Result.emplace_back(Episode->SerializeActor(View));
      }
    }
    return Result;
  };

  BIND_SYNC(spawn_actor) << [this](
      cr::ActorDescription Description,
      const cr::Transform &Transform) -> R<cr::Actor>
  {
    REQUIRE_CARLA_EPISODE();

    auto Result = Episode->SpawnActorWithInfo(Transform, std::move(Description));

    if (Result.Key != EActorSpawnResultStatus::Success)
    {
      UE_LOG(LogCarla, Error, TEXT("Actor not Spawned"));
      RESPOND_ERROR_FSTRING(FActorSpawnResult::StatusToString(Result.Key));
    }

    ALargeMapManager* LargeMap = UCarlaStatics::GetLargeMapManager(Episode->GetWorld());
    if(LargeMap)
    {
      LargeMap->OnActorSpawned(*Result.Value);
    }

    return Episode->SerializeActor(Result.Value);
  };

  BIND_SYNC(spawn_actor_with_parent) << [this](
      cr::ActorDescription Description,
      const cr::Transform &Transform,
      cr::ActorId ParentId,
      cr::AttachmentType InAttachmentType) -> R<cr::Actor>
  {
    REQUIRE_CARLA_EPISODE();

    auto Result = Episode->SpawnActorWithInfo(Transform, std::move(Description));
    if (Result.Key != EActorSpawnResultStatus::Success)
    {
      RESPOND_ERROR_FSTRING(FActorSpawnResult::StatusToString(Result.Key));
    }

    FCarlaActor* CarlaActor = Episode->FindCarlaActor(Result.Value->GetActorId());
    if (!CarlaActor)
    {
      RESPOND_ERROR("internal error: actor could not be spawned");
    }

    FCarlaActor* ParentCarlaActor = Episode->FindCarlaActor(ParentId);

    if (!ParentCarlaActor)
    {
      RESPOND_ERROR("unable to attach actor: parent actor not found");
    }

    CarlaActor->SetParent(ParentId);
    CarlaActor->SetAttachmentType(InAttachmentType);
    ParentCarlaActor->AddChildren(CarlaActor->GetActorId());

    // Only is possible to attach if the actor has been really spawned and
    // is not in dormant state
    if(!ParentCarlaActor->IsDormant())
    {
      Episode->AttachActors(
          CarlaActor->GetActor(),
          ParentCarlaActor->GetActor(),
          static_cast<EAttachmentType>(InAttachmentType));
    }
    else
    {
      Episode->PutActorToSleep(CarlaActor->GetActorId());
    }

    return Episode->SerializeActor(CarlaActor);
  };

  BIND_SYNC(destroy_actor) << [this](cr::ActorId ActorId) -> R<bool>
  {
    REQUIRE_CARLA_EPISODE();
    FCarlaActor* CarlaActor = Episode->FindCarlaActor(ActorId);
    if ( !CarlaActor )
    {
      RESPOND_ERROR("unable to destroy actor: not found");
    }
    UE_LOG(LogCarla, Log, TEXT("CarlaServer destroy_actor %d"), ActorId);
    // We need to force the actor state change, since dormant actors
    //  will ignore the FCarlaActor destruction
    CarlaActor->SetActorState(cr::ActorState::PendingKill);
    if (!Episode->DestroyActor(ActorId))
    {
      RESPOND_ERROR("internal error: unable to destroy actor");
    }
    return true;
  };

  // ~~ Actor physics ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

  BIND_SYNC(set_actor_location) << [this](
      cr::ActorId ActorId,
      cr::Location Location) -> R<void>
  {
    REQUIRE_CARLA_EPISODE();
    FCarlaActor* CarlaActor = Episode->FindCarlaActor(ActorId);
    if (!CarlaActor)
    {
      return RespondError(
          "set_actor_location",
          ECarlaServerResponse::ActorNotFound,
          " Actor Id: " + FString::FromInt(ActorId));
    }

    CarlaActor->SetActorGlobalLocation(
        Location, ETeleportType::TeleportPhysics);
    return R<void>::Success();
  };

  BIND_SYNC(set_actor_transform) << [this](
      cr::ActorId ActorId,
      cr::Transform Transform) -> R<void>
  {
    REQUIRE_CARLA_EPISODE();
    FCarlaActor* CarlaActor = Episode->FindCarlaActor(ActorId);
    if (!CarlaActor)
    {
      return RespondError(
          "set_actor_transform",
          ECarlaServerResponse::ActorNotFound,
          " Actor Id: " + FString::FromInt(ActorId));
    }

    CarlaActor->SetActorGlobalTransform(
        Transform, ETeleportType::TeleportPhysics);
    return R<void>::Success();
  };

  BIND_SYNC(set_walker_state) << [this] (
      cr::ActorId ActorId,
      cr::Transform Transform,
      float Speed) -> R<void>
  {
    REQUIRE_CARLA_EPISODE();
    FCarlaActor* CarlaActor = Episode->FindCarlaActor(ActorId);
    if (!CarlaActor)
    {
      return RespondError(
          "set_walker_state",
          ECarlaServerResponse::ActorNotFound,
          " Actor Id: " + FString::FromInt(ActorId));
    }

    // apply walker transform
    ECarlaServerResponse Response =
        CarlaActor->SetWalkerState(
            Transform,
            cr::WalkerControl(
              Transform.GetForwardVector(), Speed, false));
    if (Response != ECarlaServerResponse::Success)
    {
      return RespondError(
          "set_walker_state",
          Response,
          " Actor Id: " + FString::FromInt(ActorId));
    }
    return R<void>::Success();
  };

  BIND_SYNC(set_actor_target_velocity) << [this](
      cr::ActorId ActorId,
      cr::Vector3D vector) -> R<void>
  {
    REQUIRE_CARLA_EPISODE();
    FCarlaActor* CarlaActor = Episode->FindCarlaActor(ActorId);
    if (!CarlaActor)
    {
      return RespondError(
          "set_actor_target_velocity",
          ECarlaServerResponse::ActorNotFound,
          " Actor Id: " + FString::FromInt(ActorId));
    }
    ECarlaServerResponse Response =
        CarlaActor->SetActorTargetVelocity(vector.ToCentimeters().ToFVector());
    if (Response != ECarlaServerResponse::Success)
    {
      return RespondError(
          "set_actor_target_velocity",
          Response,
          " Actor Id: " + FString::FromInt(ActorId));
    }
    return R<void>::Success();
  };

  BIND_SYNC(set_actor_target_angular_velocity) << [this](
      cr::ActorId ActorId,
      cr::Vector3D vector) -> R<void>
  {
    REQUIRE_CARLA_EPISODE();
    FCarlaActor* CarlaActor = Episode->FindCarlaActor(ActorId);
    if (!CarlaActor)
    {
      return RespondError(
          "set_actor_target_angular_velocity",
          ECarlaServerResponse::ActorNotFound,
          " Actor Id: " + FString::FromInt(ActorId));
    }
    ECarlaServerResponse Response =
        CarlaActor->SetActorTargetAngularVelocity(vector.ToFVector());
    if (Response != ECarlaServerResponse::Success)
    {
      return RespondError(
          "set_actor_target_angular_velocity",
          Response,
          " Actor Id: " + FString::FromInt(ActorId));
    }
    return R<void>::Success();
  };

  BIND_SYNC(enable_actor_constant_velocity) << [this](
      cr::ActorId ActorId,
      cr::Vector3D vector) -> R<void>
  {
    REQUIRE_CARLA_EPISODE();
    FCarlaActor* CarlaActor = Episode->FindCarlaActor(ActorId);
    if (!CarlaActor)
    {
      return RespondError(
          "enable_actor_constant_velocity",
          ECarlaServerResponse::ActorNotFound,
          " Actor Id: " + FString::FromInt(ActorId));
    }

    ECarlaServerResponse Response =
        CarlaActor->EnableActorConstantVelocity(vector.ToCentimeters().ToFVector());
    if (Response != ECarlaServerResponse::Success)
    {
      return RespondError(
          "enable_actor_constant_velocity",
          Response,
          " Actor Id: " + FString::FromInt(ActorId));
    }

    return R<void>::Success();
  };

  BIND_SYNC(disable_actor_constant_velocity) << [this](
      cr::ActorId ActorId) -> R<void>
  {
    REQUIRE_CARLA_EPISODE();
    FCarlaActor* CarlaActor = Episode->FindCarlaActor(ActorId);
    if (!CarlaActor)
    {
      return RespondError(
          "disable_actor_constant_velocity",
          ECarlaServerResponse::ActorNotFound,
          " Actor Id: " + FString::FromInt(ActorId));
    }

    ECarlaServerResponse Response =
        CarlaActor->DisableActorConstantVelocity();
    if (Response != ECarlaServerResponse::Success)
    {
      return RespondError(
          "disable_actor_constant_velocity",
          Response,
          " Actor Id: " + FString::FromInt(ActorId));
    }

    return R<void>::Success();
  };

  BIND_SYNC(add_actor_impulse) << [this](
      cr::ActorId ActorId,
      cr::Vector3D vector) -> R<void>
  {
    REQUIRE_CARLA_EPISODE();
    FCarlaActor* CarlaActor = Episode->FindCarlaActor(ActorId);
    if (!CarlaActor)
    {
      return RespondError(
          "add_actor_impulse",
          ECarlaServerResponse::ActorNotFound,
          " Actor Id: " + FString::FromInt(ActorId));
    }

    ECarlaServerResponse Response =
        CarlaActor->AddActorImpulse(vector.ToCentimeters().ToFVector());
    if (Response != ECarlaServerResponse::Success)
    {
      return RespondError(
          "add_actor_impulse",
          Response,
          " Actor Id: " + FString::FromInt(ActorId));
    }
    return R<void>::Success();
  };

  BIND_SYNC(add_actor_impulse_at_location) << [this](
      cr::ActorId ActorId,
      cr::Vector3D impulse,
      cr::Vector3D location) -> R<void>
  {
    REQUIRE_CARLA_EPISODE();
    FCarlaActor* CarlaActor = Episode->FindCarlaActor(ActorId);
    if (!CarlaActor)
    {
      return RespondError(
          "add_actor_impulse_at_location",
          ECarlaServerResponse::ActorNotFound,
          " Actor Id: " + FString::FromInt(ActorId));
    }
    FVector UELocation = location.ToCentimeters().ToFVector();
    ACarlaGameModeBase* GameMode = UCarlaStatics::GetGameMode(Episode->GetWorld());
    ALargeMapManager* LargeMap = GameMode->GetLMManager();
    if (LargeMap)
    {
      UELocation = LargeMap->GlobalToLocalLocation(UELocation);
    }
    ECarlaServerResponse Response =
        CarlaActor->AddActorImpulseAtLocation(impulse.ToCentimeters().ToFVector(), UELocation);
    if (Response != ECarlaServerResponse::Success)
    {
      return RespondError(
          "add_actor_impulse_at_location",
          Response,
          " Actor Id: " + FString::FromInt(ActorId));
    }

    return R<void>::Success();
  };

  BIND_SYNC(add_actor_force) << [this](
      cr::ActorId ActorId,
      cr::Vector3D vector) -> R<void>
  {
    REQUIRE_CARLA_EPISODE();
    FCarlaActor* CarlaActor = Episode->FindCarlaActor(ActorId);
    if (!CarlaActor)
    {
      return RespondError(
          "add_actor_force",
          ECarlaServerResponse::ActorNotFound,
          " Actor Id: " + FString::FromInt(ActorId));
    }
    ECarlaServerResponse Response =
        CarlaActor->AddActorForce(vector.ToCentimeters().ToFVector());
    if (Response != ECarlaServerResponse::Success)
    {
      return RespondError(
          "add_actor_force",
          Response,
          " Actor Id: " + FString::FromInt(ActorId));
    }
    return R<void>::Success();
  };

  BIND_SYNC(add_actor_force_at_location) << [this](
      cr::ActorId ActorId,
      cr::Vector3D force,
      cr::Vector3D location) -> R<void>
  {
    REQUIRE_CARLA_EPISODE();
    FCarlaActor* CarlaActor = Episode->FindCarlaActor(ActorId);
    if (!CarlaActor)
    {
      return RespondError(
          "add_actor_force_at_location",
          ECarlaServerResponse::ActorNotFound,
          " Actor Id: " + FString::FromInt(ActorId));
    }
    FVector UELocation = location.ToCentimeters().ToFVector();
    ACarlaGameModeBase* GameMode = UCarlaStatics::GetGameMode(Episode->GetWorld());
    ALargeMapManager* LargeMap = GameMode->GetLMManager();
    if (LargeMap)
    {
      UELocation = LargeMap->GlobalToLocalLocation(UELocation);
    }
    ECarlaServerResponse Response =
        CarlaActor->AddActorForceAtLocation(UELocation, force.ToCentimeters().ToFVector());
    if (Response != ECarlaServerResponse::Success)
    {
      return RespondError(
          "add_actor_force_at_location",
          Response,
          " Actor Id: " + FString::FromInt(ActorId));
    }
    return R<void>::Success();
  };

  BIND_SYNC(add_actor_angular_impulse) << [this](
      cr::ActorId ActorId,
      cr::Vector3D vector) -> R<void>
  {
    REQUIRE_CARLA_EPISODE();
    FCarlaActor* CarlaActor = Episode->FindCarlaActor(ActorId);
    if (!CarlaActor)
    {
      return RespondError(
          "add_actor_angular_impulse",
          ECarlaServerResponse::ActorNotFound,
          " Actor Id: " + FString::FromInt(ActorId));
    }
    ECarlaServerResponse Response =
        CarlaActor->AddActorAngularImpulse(vector.ToFVector());
    if (Response != ECarlaServerResponse::Success)
    {
      return RespondError(
          "add_actor_angular_impulse",
          Response,
          " Actor Id: " + FString::FromInt(ActorId));
    }
    return R<void>::Success();
  };

  BIND_SYNC(add_actor_torque) << [this](
      cr::ActorId ActorId,
      cr::Vector3D vector) -> R<void>
  {
    REQUIRE_CARLA_EPISODE();
    FCarlaActor* CarlaActor = Episode->FindCarlaActor(ActorId);
    if (!CarlaActor)
    {
      return RespondError(
          "add_actor_torque",
          ECarlaServerResponse::ActorNotFound,
          " Actor Id: " + FString::FromInt(ActorId));
    }
    ECarlaServerResponse Response =
        CarlaActor->AddActorTorque(vector.ToFVector());
    if (Response != ECarlaServerResponse::Success)
    {
      return RespondError(
          "add_actor_torque",
          Response,
          " Actor Id: " + FString::FromInt(ActorId));
    }
    return R<void>::Success();
  };

  BIND_SYNC(get_physics_control) << [this](
      cr::ActorId ActorId) -> R<cr::VehiclePhysicsControl>
  {
    REQUIRE_CARLA_EPISODE();
    FCarlaActor* CarlaActor = Episode->FindCarlaActor(ActorId);
        if (!CarlaActor)
    {
      return RespondError(
          "get_physics_control",
          ECarlaServerResponse::ActorNotFound,
          " Actor Id: " + FString::FromInt(ActorId));
    }
    FVehiclePhysicsControl PhysicsControl;
    ECarlaServerResponse Response =
        CarlaActor->GetPhysicsControl(PhysicsControl);
    if (Response != ECarlaServerResponse::Success)
    {
      return RespondError(
          "get_physics_control",
          Response,
          " Actor Id: " + FString::FromInt(ActorId));
    }
    return cr::VehiclePhysicsControl(PhysicsControl);
  };

  BIND_SYNC(get_vehicle_light_state) << [this](
      cr::ActorId ActorId) -> R<cr::VehicleLightState>
  {
    REQUIRE_CARLA_EPISODE();
    FCarlaActor* CarlaActor = Episode->FindCarlaActor(ActorId);
    if (!CarlaActor)
    {
      return RespondError(
          "get_vehicle_light_state",
          ECarlaServerResponse::ActorNotFound,
          " Actor Id: " + FString::FromInt(ActorId));
    }
    FVehicleLightState LightState;
    ECarlaServerResponse Response =
        CarlaActor->GetVehicleLightState(LightState);
    if (Response != ECarlaServerResponse::Success)
    {
      return RespondError(
          "get_vehicle_light_state",
          Response,
          " Actor Id: " + FString::FromInt(ActorId));
    }
    return cr::VehicleLightState(LightState);
  };

  BIND_SYNC(apply_physics_control) << [this](
      cr::ActorId ActorId,
      cr::VehiclePhysicsControl PhysicsControl) -> R<void>
  {
    REQUIRE_CARLA_EPISODE();
    FCarlaActor* CarlaActor = Episode->FindCarlaActor(ActorId);
    if (!CarlaActor)
    {
      return RespondError(
          "apply_physics_control",
          ECarlaServerResponse::ActorNotFound,
          " Actor Id: " + FString::FromInt(ActorId));
    }
    ECarlaServerResponse Response =
        CarlaActor->ApplyPhysicsControl(FVehiclePhysicsControl(PhysicsControl));
    if (Response != ECarlaServerResponse::Success)
    {
      return RespondError(
          "apply_physics_control",
          Response,
          " Actor Id: " + FString::FromInt(ActorId));
    }
    return R<void>::Success();
  };

  BIND_SYNC(set_vehicle_light_state) << [this](
      cr::ActorId ActorId,
      cr::VehicleLightState LightState) -> R<void>
  {
    REQUIRE_CARLA_EPISODE();
    FCarlaActor* CarlaActor = Episode->FindCarlaActor(ActorId);
    if (!CarlaActor)
    {
      return RespondError(
          "set_vehicle_light_state",
          ECarlaServerResponse::ActorNotFound,
          " Actor Id: " + FString::FromInt(ActorId));
    }
    ECarlaServerResponse Response =
        CarlaActor->SetVehicleLightState(FVehicleLightState(LightState));
    if (Response != ECarlaServerResponse::Success)
    {
      return RespondError(
          "set_vehicle_light_state",
          Response,
          " Actor Id: " + FString::FromInt(ActorId));
    }
    return R<void>::Success();
  };


  BIND_SYNC(open_vehicle_door) << [this](
      cr::ActorId ActorId,
      cr::VehicleDoor DoorIdx) -> R<void>
  {
    REQUIRE_CARLA_EPISODE();
    FCarlaActor* CarlaActor = Episode->FindCarlaActor(ActorId);
    if (!CarlaActor)
    {
      return RespondError(
          "open_vehicle_door",
          ECarlaServerResponse::ActorNotFound,
          " Actor Id: " + FString::FromInt(ActorId));
    }
    ECarlaServerResponse Response =
        CarlaActor->OpenVehicleDoor(static_cast<EVehicleDoor>(DoorIdx));
    if (Response != ECarlaServerResponse::Success)
    {
      return RespondError(
          "open_vehicle_door",
          Response,
          " Actor Id: " + FString::FromInt(ActorId));
    }
    return R<void>::Success();
  };

  BIND_SYNC(close_vehicle_door) << [this](
      cr::ActorId ActorId,
      cr::VehicleDoor DoorIdx) -> R<void>
  {
    REQUIRE_CARLA_EPISODE();
    FCarlaActor* CarlaActor = Episode->FindCarlaActor(ActorId);
    if (!CarlaActor)
    {
      return RespondError(
          "close_vehicle_door",
          ECarlaServerResponse::ActorNotFound,
          " Actor Id: " + FString::FromInt(ActorId));
    }
    ECarlaServerResponse Response =
        CarlaActor->CloseVehicleDoor(static_cast<EVehicleDoor>(DoorIdx));
    if (Response != ECarlaServerResponse::Success)
    {
      return RespondError(
          "close_vehicle_door",
          Response,
          " Actor Id: " + FString::FromInt(ActorId));
    }
    return R<void>::Success();
  };

  BIND_SYNC(set_wheel_steer_direction) << [this](
    cr::ActorId ActorId,
    cr::VehicleWheelLocation WheelLocation,
    float AngleInDeg) -> R<void>
  {
    REQUIRE_CARLA_EPISODE();
    FCarlaActor* CarlaActor = Episode->FindCarlaActor(ActorId);
    if(!CarlaActor){
      return RespondError(
          "set_wheel_steer_direction",
          ECarlaServerResponse::ActorNotFound,
          " Actor Id: " + FString::FromInt(ActorId));
    }
    ECarlaServerResponse Response =
        CarlaActor->SetWheelSteerDirection(
            static_cast<EVehicleWheelLocation>(WheelLocation), AngleInDeg);
    if (Response != ECarlaServerResponse::Success)
    {
      return RespondError(
          "set_wheel_steer_direction",
          Response,
          " Actor Id: " + FString::FromInt(ActorId));
    }
    return R<void>::Success();
  };

  BIND_SYNC(get_wheel_steer_angle) << [this](
      const cr::ActorId ActorId,
      cr::VehicleWheelLocation WheelLocation) -> R<float>
  {
    REQUIRE_CARLA_EPISODE();
    FCarlaActor* CarlaActor = Episode->FindCarlaActor(ActorId);
    if(!CarlaActor){
      return RespondError(
          "get_wheel_steer_angle",
          ECarlaServerResponse::ActorNotFound,
          " Actor Id: " + FString::FromInt(ActorId));
    }
    float Angle;
    ECarlaServerResponse Response =
        CarlaActor->GetWheelSteerAngle(
            static_cast<EVehicleWheelLocation>(WheelLocation), Angle);
    if (Response != ECarlaServerResponse::Success)
    {
      return RespondError(
          "get_wheel_steer_angle",
          Response,
          " Actor Id: " + FString::FromInt(ActorId));
    }
    return Angle;
  };

  BIND_SYNC(set_actor_simulate_physics) << [this](
      cr::ActorId ActorId,
      bool bEnabled) -> R<void>
  {
    REQUIRE_CARLA_EPISODE();
    FCarlaActor* CarlaActor = Episode->FindCarlaActor(ActorId);
    if (!CarlaActor)
    {
      return RespondError(
          "set_actor_simulate_physics",
          ECarlaServerResponse::ActorNotFound,
          " Actor Id: " + FString::FromInt(ActorId));
    }
    ECarlaServerResponse Response =
        CarlaActor->SetActorSimulatePhysics(bEnabled);
    if (Response != ECarlaServerResponse::Success)
    {
      return RespondError(
          "set_actor_simulate_physics",
          Response,
          " Actor Id: " + FString::FromInt(ActorId));
    }
    return R<void>::Success();
  };

  BIND_SYNC(set_actor_enable_gravity) << [this](
      cr::ActorId ActorId,
      bool bEnabled) -> R<void>
  {
    REQUIRE_CARLA_EPISODE();
    FCarlaActor* CarlaActor = Episode->FindCarlaActor(ActorId);
    if (!CarlaActor)
    {
      return RespondError(
          "set_actor_enable_gravity",
          ECarlaServerResponse::ActorNotFound,
          " Actor Id: " + FString::FromInt(ActorId));
    }
    ECarlaServerResponse Response =
        CarlaActor->SetActorEnableGravity(bEnabled);
    if (Response != ECarlaServerResponse::Success)
    {
      return RespondError(
          "set_actor_enable_gravity",
          Response,
          " Actor Id: " + FString::FromInt(ActorId));
    }
    return R<void>::Success();
  };

  // ~~ Apply control ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

  BIND_SYNC(apply_control_to_vehicle) << [this](
      cr::ActorId ActorId,
      cr::VehicleControl Control) -> R<void>
  {
    REQUIRE_CARLA_EPISODE();
    FCarlaActor* CarlaActor = Episode->FindCarlaActor(ActorId);
    if (!CarlaActor)
    {
      return RespondError(
          "apply_control_to_vehicle",
          ECarlaServerResponse::ActorNotFound,
          " Actor Id: " + FString::FromInt(ActorId));
    }
    ECarlaServerResponse Response =
        CarlaActor->ApplyControlToVehicle(Control, EVehicleInputPriority::Client);
    if (Response != ECarlaServerResponse::Success)
    {
      return RespondError(
          "apply_control_to_vehicle",
          Response,
          " Actor Id: " + FString::FromInt(ActorId));
    }
    return R<void>::Success();
  };

  BIND_SYNC(apply_control_to_walker) << [this](
      cr::ActorId ActorId,
      cr::WalkerControl Control) -> R<void>
  {
    REQUIRE_CARLA_EPISODE();
    FCarlaActor* CarlaActor = Episode->FindCarlaActor(ActorId);
    if (!CarlaActor)
    {
      return RespondError(
          "apply_control_to_walker",
          ECarlaServerResponse::ActorNotFound,
          " Actor Id: " + FString::FromInt(ActorId));
    }
    ECarlaServerResponse Response =
        CarlaActor->ApplyControlToWalker(Control);
    if (Response != ECarlaServerResponse::Success)
    {
      return RespondError(
          "apply_control_to_walker",
          Response,
          " Actor Id: " + FString::FromInt(ActorId));
    }
    return R<void>::Success();
  };

  BIND_SYNC(get_bones_transform) << [this](
      cr::ActorId ActorId) -> R<cr::WalkerBoneControlOut>
  {
    REQUIRE_CARLA_EPISODE();
    FCarlaActor* CarlaActor = Episode->FindCarlaActor(ActorId);
    if (!CarlaActor)
    {
      return RespondError(
          "get_bones_transform",
          ECarlaServerResponse::ActorNotFound,
          " Actor Id: " + FString::FromInt(ActorId));
    }
    FWalkerBoneControlOut Bones;
    ECarlaServerResponse Response =
        CarlaActor->GetBonesTransform(Bones);
    if (Response != ECarlaServerResponse::Success)
    {
      return RespondError(
          "get_bones_transform",
          Response,
          " Actor Id: " + FString::FromInt(ActorId));
    }
    
    std::vector<carla::rpc::BoneTransformDataOut> BoneData;
    for (auto Bone : Bones.BoneTransforms) 
    {
      carla::rpc::BoneTransformDataOut Data;
      Data.bone_name = std::string(TCHAR_TO_UTF8(*Bone.Get<0>()));
      FWalkerBoneControlOutData Transforms = Bone.Get<1>();
      Data.world = Transforms.World;
      Data.component = Transforms.Component;
      Data.relative = Transforms.Relative;
      BoneData.push_back(Data);
    }
    return carla::rpc::WalkerBoneControlOut(BoneData);
  };

  BIND_SYNC(set_bones_transform) << [this](
      cr::ActorId ActorId,
      carla::rpc::WalkerBoneControlIn Bones) -> R<void>
  {
    REQUIRE_CARLA_EPISODE();
    FCarlaActor* CarlaActor = Episode->FindCarlaActor(ActorId);
    if (!CarlaActor)
    {
      return RespondError(
          "set_bones_transform",
          ECarlaServerResponse::ActorNotFound,
          " Actor Id: " + FString::FromInt(ActorId));
    }

    FWalkerBoneControlIn Bones2 = FWalkerBoneControlIn(Bones);
    ECarlaServerResponse Response = CarlaActor->SetBonesTransform(Bones2);
    if (Response != ECarlaServerResponse::Success)
    {
      return RespondError(
          "set_bones_transform",
          Response,
          " Actor Id: " + FString::FromInt(ActorId));
    }
    
    return R<void>::Success();
  };

  BIND_SYNC(blend_pose) << [this](
      cr::ActorId ActorId,
      float Blend) -> R<void>
  {
    REQUIRE_CARLA_EPISODE();
    FCarlaActor* CarlaActor = Episode->FindCarlaActor(ActorId);
    if (!CarlaActor)
    {
      return RespondError(
          "blend_pose",
          ECarlaServerResponse::ActorNotFound,
          " Actor Id: " + FString::FromInt(ActorId));
    }

    ECarlaServerResponse Response = CarlaActor->BlendPose(Blend);
    if (Response != ECarlaServerResponse::Success)
    {
      return RespondError(
          "blend_pose",
          Response,
          " Actor Id: " + FString::FromInt(ActorId));
    }
    
    return R<void>::Success();
  };

  BIND_SYNC(get_pose_from_animation) << [this](
      cr::ActorId ActorId) -> R<void>
  {
    REQUIRE_CARLA_EPISODE();
    FCarlaActor* CarlaActor = Episode->FindCarlaActor(ActorId);
    if (!CarlaActor)
    {
      return RespondError(
          "get_pose_from_animation",
          ECarlaServerResponse::ActorNotFound,
          " Actor Id: " + FString::FromInt(ActorId));
    }

    ECarlaServerResponse Response = CarlaActor->GetPoseFromAnimation();
    if (Response != ECarlaServerResponse::Success)
    {
      return RespondError(
          "get_pose_from_animation",
          Response,
          " Actor Id: " + FString::FromInt(ActorId));
    }
    
    return R<void>::Success();
  };

  BIND_SYNC(set_actor_autopilot) << [this](
      cr::ActorId ActorId,
      bool bEnabled) -> R<void>
  {
    REQUIRE_CARLA_EPISODE();
    FCarlaActor* CarlaActor = Episode->FindCarlaActor(ActorId);
    if (!CarlaActor)
    {
      return RespondError(
          "set_actor_autopilot",
          ECarlaServerResponse::ActorNotFound,
          " Actor Id: " + FString::FromInt(ActorId));
    }
    ECarlaServerResponse Response =
        CarlaActor->SetActorAutopilot(bEnabled);
    if (Response != ECarlaServerResponse::Success)
    {
      return RespondError(
          "set_actor_autopilot",
          Response,
          " Actor Id: " + FString::FromInt(ActorId));
    }
    return R<void>::Success();
  };

  BIND_SYNC(show_vehicle_debug_telemetry) << [this](
      cr::ActorId ActorId,
      bool bEnabled) -> R<void>
  {
    REQUIRE_CARLA_EPISODE();
    FCarlaActor* CarlaActor = Episode->FindCarlaActor(ActorId);
    if (!CarlaActor)
    {
      return RespondError(
          "show_vehicle_debug_telemetry",
          ECarlaServerResponse::ActorNotFound,
          " Actor Id: " + FString::FromInt(ActorId));
    }
    ECarlaServerResponse Response =
        CarlaActor->ShowVehicleDebugTelemetry(bEnabled);
    if (Response != ECarlaServerResponse::Success)
    {
      return RespondError(
          "show_vehicle_debug_telemetry",
          Response,
          " Actor Id: " + FString::FromInt(ActorId));
    }
    return R<void>::Success();
  };

  BIND_SYNC(enable_carsim) << [this](
      cr::ActorId ActorId,
      std::string SimfilePath) -> R<void>
  {
    REQUIRE_CARLA_EPISODE();
    FCarlaActor* CarlaActor = Episode->FindCarlaActor(ActorId);
    if (!CarlaActor)
    {
      return RespondError(
          "enable_carsim",
          ECarlaServerResponse::ActorNotFound,
          " Actor Id: " + FString::FromInt(ActorId));
    }
    ECarlaServerResponse Response =
        CarlaActor->EnableCarSim(carla::rpc::ToFString(SimfilePath));
    if (Response != ECarlaServerResponse::Success)
    {
      return RespondError(
          "enable_carsim",
          Response,
          " Actor Id: " + FString::FromInt(ActorId));
    }
    return R<void>::Success();
  };

  BIND_SYNC(use_carsim_road) << [this](
      cr::ActorId ActorId,
      bool bEnabled) -> R<void>
  {
    REQUIRE_CARLA_EPISODE();
    FCarlaActor* CarlaActor = Episode->FindCarlaActor(ActorId);
    if (!CarlaActor)
    {
      return RespondError(
          "use_carsim_road",
          ECarlaServerResponse::ActorNotFound,
          " Actor Id: " + FString::FromInt(ActorId));
    }
    ECarlaServerResponse Response =
        CarlaActor->UseCarSimRoad(bEnabled);
    if (Response != ECarlaServerResponse::Success)
    {
      return RespondError(
          "use_carsim_road",
          Response,
          " Actor Id: " + FString::FromInt(ActorId));
    }
    return R<void>::Success();
  };

  BIND_SYNC(enable_chrono_physics) << [this](
      cr::ActorId ActorId,
      uint64_t MaxSubsteps,
      float MaxSubstepDeltaTime,
      std::string VehicleJSON,
      std::string PowertrainJSON,
      std::string TireJSON,
      std::string BaseJSONPath) -> R<void>
  {
    REQUIRE_CARLA_EPISODE();
    FCarlaActor* CarlaActor = Episode->FindCarlaActor(ActorId);
    if (!CarlaActor)
    {
      return RespondError(
          "enable_chrono_physics",
          ECarlaServerResponse::ActorNotFound,
          " Actor Id: " + FString::FromInt(ActorId));
    }
    ECarlaServerResponse Response =
        CarlaActor->EnableChronoPhysics(
            MaxSubsteps, MaxSubstepDeltaTime,
            cr::ToFString(VehicleJSON),
            cr::ToFString(PowertrainJSON),
            cr::ToFString(TireJSON),
            cr::ToFString(BaseJSONPath));
    if (Response != ECarlaServerResponse::Success)
    {
      return RespondError(
          "enable_chrono_physics",
          Response,
          " Actor Id: " + FString::FromInt(ActorId));
    }
    return R<void>::Success();
  };

  // ~~ Traffic lights ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

  BIND_SYNC(set_traffic_light_state) << [this](
      cr::ActorId ActorId,
      cr::TrafficLightState trafficLightState) -> R<void>
  {
    REQUIRE_CARLA_EPISODE();
    FCarlaActor* CarlaActor = Episode->FindCarlaActor(ActorId);
    if (!CarlaActor)
    {
      return RespondError(
          "set_traffic_light_state",
          ECarlaServerResponse::ActorNotFound,
          " Actor Id: " + FString::FromInt(ActorId));
    }
    ECarlaServerResponse Response =
        CarlaActor->SetTrafficLightState(
        static_cast<ETrafficLightState>(trafficLightState));
    if (Response != ECarlaServerResponse::Success)
    {
      return RespondError(
          "set_traffic_light_state",
          Response,
          " Actor Id: " + FString::FromInt(ActorId));
    }
    return R<void>::Success();
  };

  BIND_SYNC(set_traffic_light_green_time) << [this](
      cr::ActorId ActorId,
      float GreenTime) -> R<void>
  {
    REQUIRE_CARLA_EPISODE();
    FCarlaActor* CarlaActor = Episode->FindCarlaActor(ActorId);
    if (!CarlaActor)
    {
      return RespondError(
          "set_traffic_light_green_time",
          ECarlaServerResponse::ActorNotFound,
          " Actor Id: " + FString::FromInt(ActorId));
    }
    ECarlaServerResponse Response =
        CarlaActor->SetLightGreenTime(GreenTime);
    if (Response != ECarlaServerResponse::Success)
    {
      return RespondError(
          "set_traffic_light_green_time",
          Response,
          " Actor Id: " + FString::FromInt(ActorId));
    }
    return R<void>::Success();
  };

  BIND_SYNC(set_traffic_light_yellow_time) << [this](
      cr::ActorId ActorId,
      float YellowTime) -> R<void>
  {
    REQUIRE_CARLA_EPISODE();
    FCarlaActor* CarlaActor = Episode->FindCarlaActor(ActorId);
    if (!CarlaActor)
    {
      return RespondError(
          "set_traffic_light_yellow_time",
          ECarlaServerResponse::ActorNotFound,
          " Actor Id: " + FString::FromInt(ActorId));
    }
    ECarlaServerResponse Response =
        CarlaActor->SetLightYellowTime(YellowTime);
    if (Response != ECarlaServerResponse::Success)
    {
      return RespondError(
          "set_traffic_light_yellow_time",
          Response,
          " Actor Id: " + FString::FromInt(ActorId));
    }
    return R<void>::Success();
  };

  BIND_SYNC(set_traffic_light_red_time) << [this](
      cr::ActorId ActorId,
      float RedTime) -> R<void>
  {
    REQUIRE_CARLA_EPISODE();
    FCarlaActor* CarlaActor = Episode->FindCarlaActor(ActorId);
    if (!CarlaActor)
    {
      return RespondError(
          "set_traffic_light_red_time",
          ECarlaServerResponse::ActorNotFound,
          " Actor Id: " + FString::FromInt(ActorId));
    }
    ECarlaServerResponse Response =
        CarlaActor->SetLightRedTime(RedTime);
    if (Response != ECarlaServerResponse::Success)
    {
      return RespondError(
          "set_traffic_light_red_time",
          Response,
          " Actor Id: " + FString::FromInt(ActorId));
    }
    return R<void>::Success();
  };

  BIND_SYNC(freeze_traffic_light) << [this](
      cr::ActorId ActorId,
      bool Freeze) -> R<void>
  {
    REQUIRE_CARLA_EPISODE();
    FCarlaActor* CarlaActor = Episode->FindCarlaActor(ActorId);
    if (!CarlaActor)
    {
      return RespondError(
          "freeze_traffic_light",
          ECarlaServerResponse::ActorNotFound,
          " Actor Id: " + FString::FromInt(ActorId));
    }
    ECarlaServerResponse Response =
        CarlaActor->FreezeTrafficLight(Freeze);
    if (Response != ECarlaServerResponse::Success)
    {
      return RespondError(
          "freeze_traffic_light",
          Response,
          " Actor Id: " + FString::FromInt(ActorId));
    }
    return R<void>::Success();
  };

  BIND_SYNC(reset_traffic_light_group) << [this](
      cr::ActorId ActorId) -> R<void>
  {
    REQUIRE_CARLA_EPISODE();
    FCarlaActor* CarlaActor = Episode->FindCarlaActor(ActorId);
    if (!CarlaActor)
    {
      return RespondError(
          "reset_traffic_light_group",
          ECarlaServerResponse::ActorNotFound,
          " Actor Id: " + FString::FromInt(ActorId));
    }
    ECarlaServerResponse Response =
        CarlaActor->ResetTrafficLightGroup();
    if (Response != ECarlaServerResponse::Success)
    {
      return RespondError(
          "reset_traffic_light_group",
          Response,
          " Actor Id: " + FString::FromInt(ActorId));
    }
    return R<void>::Success();
  };

  BIND_SYNC(reset_all_traffic_lights) << [this]() -> R<void>
  {
    REQUIRE_CARLA_EPISODE();
    for (TActorIterator<ATrafficLightGroup> It(Episode->GetWorld()); It; ++It)
    {
      It->ResetGroup();
    }
    return R<void>::Success();
  };

  BIND_SYNC(freeze_all_traffic_lights) << [this]
      (bool frozen) -> R<void>
  {
    REQUIRE_CARLA_EPISODE();
    auto* GameMode = UCarlaStatics::GetGameMode(Episode->GetWorld());
    if (!GameMode)
    {
      RESPOND_ERROR("unable to find CARLA game mode");
    }
    auto* TraffiLightManager = GameMode->GetTrafficLightManager();
    TraffiLightManager->SetFrozen(frozen);
    return R<void>::Success();
  };

  BIND_SYNC(get_vehicle_light_states) << [this]() -> R<cr::VehicleLightStateList>
  {
    REQUIRE_CARLA_EPISODE();
    cr::VehicleLightStateList List;

    auto It = Episode->GetActorRegistry().begin();
    for (; It != Episode->GetActorRegistry().end(); ++It)
    {
      const FCarlaActor& View = *(It.Value().Get());
      if (View.GetActorType() == FCarlaActor::ActorType::Vehicle)
      {
        if(View.IsDormant())
        {
          // todo: implement
        }
        else
        {
          auto Actor = View.GetActor();
          if (!Actor->IsPendingKill())
          {
            const ACarlaWheeledVehicle *Vehicle = Cast<ACarlaWheeledVehicle>(Actor);
            List.emplace_back(
                View.GetActorId(),
                cr::VehicleLightState(Vehicle->GetVehicleLightState()).GetLightStateAsValue());
          }
        }
      }
    }
    return List;
  };

  BIND_SYNC(get_group_traffic_lights) << [this](
      const cr::ActorId ActorId) -> R<std::vector<cr::ActorId>>
  {
    REQUIRE_CARLA_EPISODE();
    FCarlaActor* CarlaActor = Episode->FindCarlaActor(ActorId);
    if (!CarlaActor)
    {
      RESPOND_ERROR("unable to get group traffic lights: actor not found");
    }
    if (CarlaActor->IsDormant())
    {
      //todo implement
      return std::vector<cr::ActorId>();
    }
    else
    {
      auto TrafficLight = Cast<ATrafficLightBase>(CarlaActor->GetActor());
      if (TrafficLight == nullptr)
      {
        RESPOND_ERROR("unable to get group traffic lights: actor is not a traffic light");
      }
      std::vector<cr::ActorId> Result;
      for (auto* TLight : TrafficLight->GetGroupTrafficLights())
      {
        auto* View = Episode->FindCarlaActor(TLight);
        if (View)
        {
          Result.push_back(View->GetActorId());
        }
      }
      return Result;
    }
  };

  BIND_SYNC(get_light_boxes) << [this](
      const cr::ActorId ActorId) -> R<std::vector<cg::BoundingBox>>
  {
    REQUIRE_CARLA_EPISODE();
    FCarlaActor* CarlaActor = Episode->FindCarlaActor(ActorId);
    if (CarlaActor)
    {
      return RespondError(
          "get_light_boxes",
          ECarlaServerResponse::ActorNotFound,
          " Actor Id: " + FString::FromInt(ActorId));
    }
    if (CarlaActor->IsDormant())
    {
      return RespondError(
          "get_light_boxes",
          ECarlaServerResponse::FunctionNotAvailiableWhenDormant,
          " Actor Id: " + FString::FromInt(ActorId));
    }
    else
    {
      ATrafficLightBase* TrafficLight = Cast<ATrafficLightBase>(CarlaActor->GetActor());
      if (!TrafficLight)
      {
        return RespondError(
          "get_light_boxes",
          ECarlaServerResponse::NotATrafficLight,
          " Actor Id: " + FString::FromInt(ActorId));
      }
      TArray<FBoundingBox> Result;
      TArray<uint8> OutTag;
      UBoundingBoxCalculator::GetTrafficLightBoundingBox(
          TrafficLight, Result, OutTag,
          static_cast<uint8>(carla::rpc::CityObjectLabel::TrafficLight));
      return MakeVectorFromTArray<cg::BoundingBox>(Result);
    }
  };

  // ~~ Logging and playback ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

  BIND_SYNC(start_recorder) << [this](std::string name, bool AdditionalData) -> R<std::string>
  {
    REQUIRE_CARLA_EPISODE();
    return R<std::string>(Episode->StartRecorder(name, AdditionalData));
  };

  BIND_SYNC(stop_recorder) << [this]() -> R<void>
  {
    REQUIRE_CARLA_EPISODE();
    Episode->GetRecorder()->Stop();
    return R<void>::Success();
  };

  BIND_SYNC(show_recorder_file_info) << [this](
      std::string name,
      bool show_all) -> R<std::string>
  {
    REQUIRE_CARLA_EPISODE();
    return R<std::string>(Episode->GetRecorder()->ShowFileInfo(
        name,
        show_all));
  };

  BIND_SYNC(show_recorder_collisions) << [this](
      std::string name,
      char type1,
      char type2) -> R<std::string>
  {
    REQUIRE_CARLA_EPISODE();
    return R<std::string>(Episode->GetRecorder()->ShowFileCollisions(
        name,
        type1,
        type2));
  };

  BIND_SYNC(show_recorder_actors_blocked) << [this](
      std::string name,
      double min_time,
      double min_distance) -> R<std::string>
  {
    REQUIRE_CARLA_EPISODE();
    return R<std::string>(Episode->GetRecorder()->ShowFileActorsBlocked(
        name,
        min_time,
        min_distance));
  };

  BIND_SYNC(replay_file) << [this](
      std::string name,
      double start,
      double duration,
      uint32_t follow_id,
      bool replay_sensors) -> R<std::string>
  {
    REQUIRE_CARLA_EPISODE();
    return R<std::string>(Episode->GetRecorder()->ReplayFile(
        name,
        start,
        duration,
        follow_id,
        replay_sensors));
  };

  BIND_SYNC(set_replayer_time_factor) << [this](double time_factor) -> R<void>
  {
    REQUIRE_CARLA_EPISODE();
    Episode->GetRecorder()->SetReplayerTimeFactor(time_factor);
    return R<void>::Success();
  };

  BIND_SYNC(set_replayer_ignore_hero) << [this](bool ignore_hero) -> R<void>
  {
    REQUIRE_CARLA_EPISODE();
    Episode->GetRecorder()->SetReplayerIgnoreHero(ignore_hero);
    return R<void>::Success();
  };

  BIND_SYNC(stop_replayer) << [this](bool keep_actors) -> R<void>
  {
    REQUIRE_CARLA_EPISODE();
    Episode->GetRecorder()->StopReplayer(keep_actors);
    return R<void>::Success();
  };

  // ~~ Draw debug shapes ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

  BIND_SYNC(draw_debug_shape) << [this](const cr::DebugShape &shape) -> R<void>
  {
    REQUIRE_CARLA_EPISODE();
    auto *World = Episode->GetWorld();
    check(World != nullptr);
    FDebugShapeDrawer Drawer(*World);
    Drawer.Draw(shape);
    return R<void>::Success();
  };

  // ~~ Apply commands in batch ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

  using C = cr::Command;
  using CR = cr::CommandResponse;
  using ActorId = carla::ActorId;

  auto parse_result = [](ActorId id, const auto &response) {
    return response.HasError() ? CR{response.GetError()} : CR{id};
  };

#define MAKE_RESULT(operation) return parse_result(c.actor, operation);

  auto command_visitor = carla::Functional::MakeRecursiveOverload(
      [=](auto self, const C::SpawnActor &c) -> CR {
        auto result = c.parent.has_value() ?
        spawn_actor_with_parent(
            c.description,
            c.transform,
            *c.parent,
            cr::AttachmentType::Rigid) :
        spawn_actor(c.description, c.transform);
        if (!result.HasError())
        {
          ActorId id = result.Get().id;
          auto set_id = carla::Functional::MakeOverload(
              [](C::SpawnActor &) {},
              [id](auto &s) { s.actor = id; });
          for (auto command : c.do_after)
          {
            boost::apply_visitor(set_id, command.command);
            boost::apply_visitor(self, command.command);
          }
          return id;
        }
        return result.GetError();
      },
      [=](auto, const C::DestroyActor &c) {         MAKE_RESULT(destroy_actor(c.actor)); },
      [=](auto, const C::ApplyVehicleControl &c) {  MAKE_RESULT(apply_control_to_vehicle(c.actor, c.control)); },
      [=](auto, const C::ApplyWalkerControl &c) {   MAKE_RESULT(apply_control_to_walker(c.actor, c.control)); },
      [=](auto, const C::ApplyVehiclePhysicsControl &c) {  MAKE_RESULT(apply_physics_control(c.actor, c.physics_control)); },
      [=](auto, const C::ApplyTransform &c) {       MAKE_RESULT(set_actor_transform(c.actor, c.transform)); },
      [=](auto, const C::ApplyTargetVelocity &c) {  MAKE_RESULT(set_actor_target_velocity(c.actor, c.velocity)); },
      [=](auto, const C::ApplyTargetAngularVelocity &c) { MAKE_RESULT(set_actor_target_angular_velocity(c.actor, c.angular_velocity)); },
      [=](auto, const C::ApplyImpulse &c) {         MAKE_RESULT(add_actor_impulse(c.actor, c.impulse)); },
      [=](auto, const C::ApplyForce &c) {           MAKE_RESULT(add_actor_force(c.actor, c.force)); },
      [=](auto, const C::ApplyAngularImpulse &c) {  MAKE_RESULT(add_actor_angular_impulse(c.actor, c.impulse)); },
      [=](auto, const C::ApplyTorque &c) {          MAKE_RESULT(add_actor_torque(c.actor, c.torque)); },
      [=](auto, const C::SetSimulatePhysics &c) {   MAKE_RESULT(set_actor_simulate_physics(c.actor, c.enabled)); },
      [=](auto, const C::SetEnableGravity &c) {   MAKE_RESULT(set_actor_enable_gravity(c.actor, c.enabled)); },
      // TODO: SetAutopilot should be removed. This is the old way to control the vehicles
      [=](auto, const C::SetAutopilot &c) {         MAKE_RESULT(set_actor_autopilot(c.actor, c.enabled)); },
      [=](auto, const C::ShowDebugTelemetry &c) {   MAKE_RESULT(show_vehicle_debug_telemetry(c.actor, c.enabled)); },
      [=](auto, const C::SetVehicleLightState &c) { MAKE_RESULT(set_vehicle_light_state(c.actor, c.light_state)); },
//      [=](auto, const C::OpenVehicleDoor &c) {      MAKE_RESULT(open_vehicle_door(c.actor, c.door_idx)); },
//      [=](auto, const C::CloseVehicleDoor &c) {     MAKE_RESULT(close_vehicle_door(c.actor, c.door_idx)); },
      [=](auto, const C::ApplyWalkerState &c) {     MAKE_RESULT(set_walker_state(c.actor, c.transform, c.speed)); });

#undef MAKE_RESULT

  BIND_SYNC(apply_batch) << [=](
      const std::vector<cr::Command> &commands,
      bool do_tick_cue)
  {
    std::vector<CR> result;
    result.reserve(commands.size());
    for (const auto &command : commands)
    {
      result.emplace_back(boost::apply_visitor(command_visitor, command.command));
    }
    if (do_tick_cue)
    {
      tick_cue();
    }
    return result;
  };

  // ~~ Light Subsystem ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

  BIND_SYNC(query_lights_state) << [this](std::string client) -> R<std::vector<cr::LightState>>
  {
    REQUIRE_CARLA_EPISODE();
    std::vector<cr::LightState> result;
    auto *World = Episode->GetWorld();
    if(World) {
      UCarlaLightSubsystem* CarlaLightSubsystem = World->GetSubsystem<UCarlaLightSubsystem>();
      result = CarlaLightSubsystem->GetLights(FString(client.c_str()));
    }
    return result;
  };

  BIND_SYNC(update_lights_state) << [this]
    (std::string client, const std::vector<cr::LightState>& lights, bool discard_client) -> R<void>
  {
    REQUIRE_CARLA_EPISODE();
    auto *World = Episode->GetWorld();
    if(World) {
      UCarlaLightSubsystem* CarlaLightSubsystem = World->GetSubsystem<UCarlaLightSubsystem>();
      CarlaLightSubsystem->SetLights(FString(client.c_str()), lights, discard_client);
    }
    return R<void>::Success();
  };


  // ~~ Ray Casting ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

  BIND_SYNC(project_point) << [this]
      (cr::Location Location, cr::Vector3D Direction, float SearchDistance)
      -> R<std::pair<bool,cr::LabelledPoint>>
  {
    REQUIRE_CARLA_EPISODE();
    auto *World = Episode->GetWorld();
    constexpr float meter_to_centimeter = 100.0f;
    FVector UELocation = Location;
    ACarlaGameModeBase* GameMode = UCarlaStatics::GetGameMode(Episode->GetWorld());
    ALargeMapManager* LargeMap = GameMode->GetLMManager();
    if (LargeMap)
    {
      UELocation = LargeMap->GlobalToLocalLocation(UELocation);
    }
    return URayTracer::ProjectPoint(UELocation, Direction.ToFVector(),
        meter_to_centimeter * SearchDistance, World);
  };

  BIND_SYNC(cast_ray) << [this]
      (cr::Location StartLocation, cr::Location EndLocation)
      -> R<std::vector<cr::LabelledPoint>>
  {
    REQUIRE_CARLA_EPISODE();
    auto *World = Episode->GetWorld();
    FVector UEStartLocation = StartLocation;
    FVector UEEndLocation = EndLocation;
    ACarlaGameModeBase* GameMode = UCarlaStatics::GetGameMode(Episode->GetWorld());
    ALargeMapManager* LargeMap = GameMode->GetLMManager();
    if (LargeMap)
    {
      UEStartLocation = LargeMap->GlobalToLocalLocation(UEStartLocation);
      UEEndLocation = LargeMap->GlobalToLocalLocation(UEEndLocation);
    }
    return URayTracer::CastRay(StartLocation, EndLocation, World);
  };

}

// =============================================================================
// -- Undef helper macros ------------------------------------------------------
// =============================================================================

#undef BIND_ASYNC
#undef BIND_SYNC
#undef REQUIRE_CARLA_EPISODE
#undef RESPOND_ERROR_FSTRING
#undef RESPOND_ERROR
#undef CARLA_ENSURE_GAME_THREAD

// =============================================================================
// -- FCarlaServer -------------------------------------------------------
// =============================================================================

FCarlaServer::FCarlaServer() : Pimpl(nullptr) {}

FCarlaServer::~FCarlaServer() {}

FDataMultiStream FCarlaServer::Start(uint16_t RPCPort, uint16_t StreamingPort)
{
  Pimpl = MakeUnique<FPimpl>(RPCPort, StreamingPort);
  StreamingPort = Pimpl->StreamingServer.GetLocalEndpoint().port();
  UE_LOG(
      LogCarlaServer,
      Log,
      TEXT("Initialized CarlaServer: Ports(rpc=%d, streaming=%d)"),
      RPCPort,
      StreamingPort);
  return Pimpl->BroadcastStream;
}

void FCarlaServer::NotifyBeginEpisode(UCarlaEpisode &Episode)
{
  check(Pimpl != nullptr);
  UE_LOG(LogCarlaServer, Log, TEXT("New episode '%s' started"), *Episode.GetMapName());
  Pimpl->Episode = &Episode;
}

void FCarlaServer::NotifyEndEpisode()
{
  check(Pimpl != nullptr);
  Pimpl->Episode = nullptr;
}

void FCarlaServer::AsyncRun(uint32 NumberOfWorkerThreads)
{
  check(Pimpl != nullptr);
  /// @todo Define better the number of threads each server gets.
  int32_t RPCThreads = std::max(2u, NumberOfWorkerThreads / 2u);
  int32_t StreamingThreads = std::max(2u, NumberOfWorkerThreads - RPCThreads);

  UE_LOG(LogCarla, Log, TEXT("FCommandLine %s"), FCommandLine::Get());

  if(!FParse::Value(FCommandLine::Get(), TEXT("-RPCThreads="), RPCThreads))
  {
    RPCThreads = std::max(2u, NumberOfWorkerThreads / 2u);
  }
  if(!FParse::Value(FCommandLine::Get(), TEXT("-StreamingThreads="), StreamingThreads))
  {
    StreamingThreads = std::max(2u, NumberOfWorkerThreads - RPCThreads);
  }

  UE_LOG(LogCarla, Log, TEXT("FCarlaServer AsyncRun %d, RPCThreads %d, StreamingThreads %d"),
        NumberOfWorkerThreads, RPCThreads, StreamingThreads);

  Pimpl->Server.AsyncRun(RPCThreads);
  Pimpl->StreamingServer.AsyncRun(StreamingThreads);

}

void FCarlaServer::RunSome(uint32 Milliseconds)
{
  TRACE_CPUPROFILER_EVENT_SCOPE_STR(__FUNCTION__);
  Pimpl->Server.SyncRunFor(carla::time_duration::milliseconds(Milliseconds));
}

bool FCarlaServer::TickCueReceived()
{
  if (Pimpl->TickCuesReceived > 0u)
  {
    --(Pimpl->TickCuesReceived);
    return true;
  }
  return false;
}

void FCarlaServer::Stop()
{
  check(Pimpl != nullptr);
  Pimpl->Server.Stop();
}

FDataStream FCarlaServer::OpenStream() const
{
  check(Pimpl != nullptr);
  return Pimpl->StreamingServer.MakeStream();
}
