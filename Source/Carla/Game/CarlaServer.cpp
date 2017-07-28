// CARLA, Copyright (C) 2017 Computer Vision Center (CVC)

#include "Carla.h"
#include "CarlaServer.h"

#include "GameFramework/PlayerStart.h"

#include "CarlaPlayerState.h"
#include "CarlaVehicleController.h"
#include "SceneCaptureCamera.h"
#include "Settings/CarlaSettings.h"

#include <carla/carla_server.h>

// =============================================================================
// -- Static local methods -----------------------------------------------------
// =============================================================================

static CarlaServer::ErrorCode ParseErrorCode(const uint32 ErrorCode)
{
  if (ErrorCode == CARLA_SERVER_SUCCESS) {
    return CarlaServer::Success;
  } else if (ErrorCode == CARLA_SERVER_TRY_AGAIN) {
    return CarlaServer::TryAgain;
  } else {
    return CarlaServer::Error;
  }
}

static int32 GetTimeOut(uint32 TimeOut, const bool bBlocking)
{
  return (bBlocking ? TimeOut : 0u);
}

// =============================================================================
// -- Set functions ------------------------------------------------------------
// =============================================================================

static inline void Set(float &lhs, float rhs)
{
  lhs = rhs;
}

static inline void Set(carla_vector3d &lhs, const FVector &rhs)
{
  lhs = {rhs.X, rhs.Y, rhs.Z};
}

static inline void Set(carla_transform &lhs, const FTransform &rhs)
{
  Set(lhs.location, rhs.GetLocation());
  Set(lhs.orientation, rhs.GetRotation().GetForwardVector());
}

static void Set(carla_image &cImage, const FCapturedImage &uImage)
{
  if (uImage.BitMap.Num() > 0) {
    cImage.width = uImage.SizeX;
    cImage.height = uImage.SizeY;
    cImage.type = PostProcessEffect::ToUInt(uImage.PostProcessEffect);
    cImage.data = &uImage.BitMap.GetData()->DWColor();

#ifdef CARLA_SERVER_EXTRA_LOG
    {
      auto GetImageType = [](carla::ImageType Type) {
        switch (Type) {
          case carla::IMAGE:        return TEXT("IMAGE");
          case carla::SCENE_FINAL:  return TEXT("SCENE_FINAL");
          case carla::DEPTH:        return TEXT("DEPTH");
          case carla::SEMANTIC_SEG: return TEXT("SEMANTIC_SEG");
          default:                  return TEXT("ERROR!");
        }
      };
      const auto Size = uImage.BitMap.Num();
      UE_LOG(LogCarlaServer, Log, TEXT("Sending image %dx%d (%d) %s"), cImage.width, cImage.height, Size, GetImageType(cImage.type()));
    }
  } else {
    UE_LOG(LogCarlaServer, Warning, TEXT("Sending empty image"));
#endif // CARLA_SERVER_EXTRA_LOG
  }
}

// =============================================================================
// -- CarlaServer --------------------------------------------------------------
// =============================================================================

CarlaServer::CarlaServer(const uint32 InWorldPort, const uint32 InTimeOut) :
  WorldPort(InWorldPort),
  TimeOut(InTimeOut),
  Server(carla_make_server()) {
  check(Server != nullptr);
}

CarlaServer::~CarlaServer()
{
  carla_free_server(Server);
}

CarlaServer::ErrorCode CarlaServer::Connect()
{
  UE_LOG(LogCarlaServer, Log, TEXT("Waiting for the client to connect..."));
  return ParseErrorCode(carla_server_connect(Server, WorldPort, TimeOut));
}

CarlaServer::ErrorCode CarlaServer::ReadNewEpisode(UCarlaSettings &Settings, const bool bBlocking)
{
  carla_request_new_episode values;
  auto ec = ParseErrorCode(carla_read_request_new_episode(Server, values, GetTimeOut(TimeOut, bBlocking)));
  if (Success == ec) {
    auto IniFile = FString(values.ini_file_length, ANSI_TO_TCHAR(values.ini_file));
    UE_LOG(LogCarlaServer, Log, TEXT("Received new episode"));
#ifdef CARLA_SERVER_EXTRA_LOG
    UE_LOG(LogCarlaServer, Log, TEXT("Received CarlaSettings.ini:\n%s"), *IniFile);
#endif // CARLA_SERVER_EXTRA_LOG
    Settings.LoadSettingsFromString(IniFile);
  }
  return ec;
}

CarlaServer::ErrorCode CarlaServer::SendSceneDescription(
      const TArray<APlayerStart *> &AvailableStartSpots,
      const bool bBlocking)
{
  const int32 NumberOfStartSpots = AvailableStartSpots.Num();
  auto StartSpots = MakeUnique<carla_transform[]>(NumberOfStartSpots);

  for (auto i = 0u; i < NumberOfStartSpots; ++i) {
    Set(StartSpots[i], AvailableStartSpots[i]->GetActorTransform());
  }

  UE_LOG(LogCarlaServer, Log, TEXT("Sending %d available start positions"), NumberOfStartSpots);
  carla_scene_description scene;
  scene.player_start_spots = StartSpots.Get();
  scene.number_of_player_start_spots = NumberOfStartSpots;

  return ParseErrorCode(carla_write_scene_description(Server, scene, GetTimeOut(TimeOut, bBlocking)));
}

CarlaServer::ErrorCode CarlaServer::ReadEpisodeStart(uint32 &StartPositionIndex, const bool bBlocking)
{
  carla_episode_start values;
  auto ec = ParseErrorCode(carla_read_episode_start(Server, values, GetTimeOut(TimeOut, bBlocking)));
  if (Success == ec) {
    StartPositionIndex = values.player_start_spot_index;
    UE_LOG(LogCarlaServer, Log, TEXT("Episode start received: { StartIndex = %d }"), StartPositionIndex);
  }
  return ec;
}

CarlaServer::ErrorCode CarlaServer::SendEpisodeReady(const bool bBlocking)
{
  UE_LOG(LogCarlaServer, Log, TEXT("Ready to play, notifying client"));
  const carla_episode_ready values = {true};
  return ParseErrorCode(carla_write_episode_ready(Server, values, GetTimeOut(TimeOut, bBlocking)));
}

CarlaServer::ErrorCode CarlaServer::ReadControl(ACarlaVehicleController &Player, const bool bBlocking)
{
  carla_control values;
  auto ec = ParseErrorCode(carla_read_control(Server, values, GetTimeOut(TimeOut, bBlocking)));
  if (Success == ec) {
    Player.SetSteeringInput(values.steer);
    Player.SetThrottleInput(values.throttle);
    Player.SetBrakeInput(values.brake);
    Player.SetHandbrakeInput(values.hand_brake);
    Player.SetReverse(values.reverse);
#ifdef CARLA_SERVER_EXTRA_LOG
    UE_LOG(
        LogCarlaServer,
        Log,
        TEXT("Read control: { Steer = %f, Throttle = %f, Brake = %f, Handbrake = %s, Reverse = %s }"),
        values.steer,
        values.throttle,
        values.brake,
        (values.hand_brake ? TEXT("True") : TEXT("False")),
        (values.reverse ? TEXT("True") : TEXT("False")));
#endif // CARLA_SERVER_EXTRA_LOG
  } else if ((!bBlocking) && (TryAgain == ec)) {
    UE_LOG(LogCarlaServer, Warning, TEXT("No control received from the client this frame!"));
  }
  return ec;
}

static void SetBoxAndSpeed(carla_agent &values, const ACharacter *Walker)
{
  values.forward_speed = FVector::DotProduct(Walker->GetVelocity(), Walker->GetActorRotation().Vector()) * 0.036f;
  /// @todo Perhaps the box it is not the same for every walker...
  values.box_extent = {45.0f, 35.0f, 100.0f};
}

static void SetBoxAndSpeed(carla_agent &values, const AWheeledVehicle *Vehicle)
{
  /// @todo This is very slow and the code is duplicated at
  /// ACarlaVehicleController.
  values.forward_speed = Vehicle->GetVehicleMovementComponent()->GetForwardSpeed() * 0.036f;
  TArray<UBoxComponent *> BoundingBoxes;
  Vehicle->GetComponents<UBoxComponent>(BoundingBoxes);
  if (BoundingBoxes.Num() > 0) {
    Set(values.box_extent, BoundingBoxes[0]->GetScaledBoxExtent());
  } else {
    UE_LOG(LogCarla, Error, TEXT("Vehicle is missing the bounding box!"));
  }
}

template <typename T>
static void AddAgents(TArray<carla_agent> &Agents, const TArray<T> &Actors, uint32 type)
{
  for (auto &&Actor : Actors) {
    Agents.Emplace();
    auto &values = Agents.Last();
    values.id = GetTypeHash(Actor);
    values.type = type;
    Set(values.transform, Actor->GetActorTransform());
    SetBoxAndSpeed(values, Actor);
  }
}

static void GetAgentInfo(
    const ACarlaGameState &GameState,
    TArray<carla_agent> &Agents)
{
  auto *WalkerSpawner = GameState.GetWalkerSpawner();
  auto *VehicleSpawner = GameState.GetVehicleSpawner();

  uint32 NumberOfAgents = 0u;
  if (WalkerSpawner != nullptr) {
    NumberOfAgents += WalkerSpawner->GetCurrentNumberOfWalkers();
  }
  if (VehicleSpawner != nullptr) {
    NumberOfAgents += VehicleSpawner->GetNumberOfSpawnedVehicles();
  }
  Agents.Reserve(NumberOfAgents);

  if (WalkerSpawner != nullptr) {
    AddAgents(Agents, WalkerSpawner->GetWalkersWhiteList(), CARLA_SERVER_AGENT_PEDESTRIAN);
    AddAgents(Agents, WalkerSpawner->GetWalkersBlackList(), CARLA_SERVER_AGENT_PEDESTRIAN);
  }
  if (VehicleSpawner != nullptr) {
    AddAgents(Agents, VehicleSpawner->GetVehicles(), CARLA_SERVER_AGENT_VEHICLE);
  }
}

CarlaServer::ErrorCode CarlaServer::SendMeasurements(
    const ACarlaGameState &GameState,
    const ACarlaPlayerState &PlayerState,
    const bool bSendNonPlayerAgentsInfo)
{
  // Measurements.
  carla_measurements values;
  values.platform_timestamp = PlayerState.GetPlatformTimeStamp();
  values.game_timestamp = PlayerState.GetGameTimeStamp();
  auto &player = values.player_measurements;
  Set(player.transform, PlayerState.GetTransform());
  Set(player.acceleration, PlayerState.GetAcceleration());
  Set(player.forward_speed, PlayerState.GetForwardSpeed());
  Set(player.collision_vehicles, PlayerState.GetCollisionIntensityCars());
  Set(player.collision_pedestrians, PlayerState.GetCollisionIntensityPedestrians());
  Set(player.collision_other, PlayerState.GetCollisionIntensityOther());
  Set(player.intersection_otherlane, PlayerState.GetOtherLaneIntersectionFactor());
  Set(player.intersection_offroad, PlayerState.GetOffRoadIntersectionFactor());

  TArray<carla_agent> Agents;
  if (bSendNonPlayerAgentsInfo) {
    GetAgentInfo(GameState, Agents);
  }
  values.non_player_agents = (Agents.Num() > 0 ? Agents.GetData() : nullptr);
  values.number_of_non_player_agents = Agents.Num();

  // Images.
  const auto NumberOfImages = PlayerState.GetNumberOfImages();
  TUniquePtr<carla_image[]> images;
  if (NumberOfImages > 0) {
    images = MakeUnique<carla_image[]>(NumberOfImages);
    for (auto i = 0u; i < NumberOfImages; ++i) {
      Set(images[i], PlayerState.GetImages()[i]);
    }
  }

  return ParseErrorCode(carla_write_measurements(Server, values, images.Get(), NumberOfImages));
}
