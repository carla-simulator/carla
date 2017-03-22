// CARLA, Copyright (C) 2017 Computer Vision Center (CVC)

#include "Carla.h"
#include "CarlaGameController.h"

#include "GameFramework/PlayerStart.h"

#include "CarlaPlayerState.h"
#include "CarlaVehicleController.h"
#include "SceneCaptureCamera.h"

#include <carla/CarlaServer.h>

// =============================================================================
// -- Set functions ------------------------------------------------------------
// =============================================================================

static inline void Set(float &lhs, float rhs)
{
  lhs = rhs;
}

static inline void Set(carla::Vector3D &cVector, const FVector &uVector)
{
  cVector = {uVector.X, uVector.Y, uVector.Z};
}

static inline void Set(carla::Vector2D &cVector, const FVector &uVector)
{
  cVector = {uVector.X, uVector.Y};
}

static void Set(std::vector<carla::Color> &cImage, const ASceneCaptureCamera *Camera)
{
  if (Camera != nullptr) {
    cImage.reserve(Camera->GetImage().Num());
    for (const auto &color : Camera->GetImage()) {
      cImage.emplace_back();
      cImage.back().R = color.R;
      cImage.back().G = color.G;
      cImage.back().B = color.B;
      cImage.back().A = color.A;
    }
  }
}

// =============================================================================
// -- Other static methods -----------------------------------------------------
// =============================================================================

static bool ReadSceneInit(carla::CarlaServer &Server)
{
  carla::Mode mode;
  uint32 scene;
  UE_LOG(LogCarla, Log, TEXT("Waiting for tryReadSceneInit..."));
  while (!Server.tryReadSceneInit(mode, scene)) {
    if (Server.needsRestart())
      return false;
  }
  return true;
}

static bool SendAndReadSceneValues(
    carla::CarlaServer &Server,
    const TArray<APlayerStart *> &AvailableStartSpots,
    uint32 &StartIndex)
{
  check(AvailableStartSpots.Num() > 0);

  carla::Scene_Values sceneValues;
  sceneValues.possible_positions.reserve(AvailableStartSpots.Num());
  for (APlayerStart *StartSpot : AvailableStartSpots) {
    const FVector &Location = StartSpot->GetActorLocation();
    UE_LOG(LogCarla, Log, TEXT("Found start position {%f, %f}"), Location.X, Location.Y);
    sceneValues.possible_positions.push_back({Location.X, Location.Y});
  }
  // Send scene values.
  UE_LOG(LogCarla, Log, TEXT("Send scene values: %d positions"), sceneValues.possible_positions.size());
  Server.sendSceneValues(sceneValues);
  // Wait till we receive the answer.
  uint32 EndIndex;
  UE_LOG(LogCarla, Log, TEXT("Waiting for episode start..."));
  while (!Server.tryReadEpisodeStart(StartIndex, EndIndex)) {
    if (Server.needsRestart())
      return false;
  }
  UE_LOG(LogCarla, Log, TEXT("Episode start received: %d -> %d"), StartIndex, EndIndex);
  // Make sure the index is in range.
  if (StartIndex >= AvailableStartSpots.Num()) {
    UE_LOG(LogCarla, Warning, TEXT("Received invalid start index, using zero instead"));
    StartIndex = 0u;
  }
  return true;
}

static void SendReward(
    carla::CarlaServer &Server,
    const ACarlaPlayerState &PlayerState,
    const std::array<const ASceneCaptureCamera *, 2u> &Cameras)
{
  carla::Reward_Values reward;
  reward.timestamp = FMath::RoundHalfToZero(1000.0 * FPlatformTime::Seconds());
  Set(reward.player_location, PlayerState.GetLocation());
  Set(reward.player_orientation, PlayerState.GetOrientation());
  Set(reward.player_acceleration, PlayerState.GetAcceleration());
  Set(reward.forward_speed, PlayerState.GetForwardSpeed());
  Set(reward.collision_car, PlayerState.GetCollisionIntensityCars());
  Set(reward.collision_pedestrian, PlayerState.GetCollisionIntensityPedestrians());
  Set(reward.collision_general, PlayerState.GetCollisionIntensityOther());
  // Set(reward.intersect_other_lane, );
  // Set(reward.intersect_offroad, );
  if (Cameras[0u] != nullptr) {
    reward.image_width = Cameras[0u]->GetImageSizeX();
    reward.image_height = Cameras[0u]->GetImageSizeY();
  }
  Set(reward.image_rgb_0, Cameras[0u]);
  Set(reward.image_rgb_1, Cameras[1u]);
  // Set(reward.image_depth_0, );
  // Set(reward.image_depth_1, );
  UE_LOG(LogCarla, Log, TEXT("Sending reward"));
  Server.sendReward(reward);
}

static void TryReadControl(carla::CarlaServer &Server, ACarlaVehicleController &Player)
{
  float steer;
  float throttle;
  if (Server.tryReadControl(steer, throttle)) {
    Player.SetSteeringInput(steer);
    Player.SetThrottleInput(throttle);
    UE_LOG(LogCarla, Log, TEXT("Read control: steer = %f, throttle = %f"), steer, throttle);
  }
}

// =============================================================================
// -- CarlaGameController ------------------------------------------------------
// =============================================================================

CarlaGameController::CarlaGameController() :
  Server(MakeUnique<carla::CarlaServer>(2001u, 2002u, 2000u)),
  Player(nullptr),
  Cameras({{nullptr}}) {}

APlayerStart *CarlaGameController::ChoosePlayerStart(
    const TArray<APlayerStart *> &AvailableStartSpots)
{
  if (bServerNeedsRestart) {
    UE_LOG(LogCarla, Log, TEXT("Initializing CarlaServer"));
    Server->init(1u);
    if (ReadSceneInit(*Server)) {
      bServerNeedsRestart = false;
    } else {
      RestartLevel(true);
    }
  }
  uint32 StartIndex = 0u;
  if (!SendAndReadSceneValues(*Server, AvailableStartSpots, StartIndex)) {
    RestartLevel(true);
  }
  return AvailableStartSpots[StartIndex];
}

void CarlaGameController::RegisterPlayer(AController &NewPlayer)
{
  Player = Cast<ACarlaVehicleController>(&NewPlayer);
  check(Player != nullptr);
}

void CarlaGameController::RegisterCaptureCamera(const ASceneCaptureCamera &CaptureCamera)
{
  for (auto i = 0u; i < Cameras.size(); ++i) {
    if (Cameras[i] == nullptr) {
      Cameras[i] = &CaptureCamera;
      UE_LOG(LogCarla, Log, TEXT("Registered capture camera %d"), i);
      return;
    }
  }
  UE_LOG(
      LogCarla,
      Warning,
      TEXT("Attempting to register a camera but already have %d, captures from this camera won't be send"),
      Cameras.size());
}

void CarlaGameController::Tick(float DeltaSeconds)
{
  check(Player != nullptr);
  if (Server->needsRestart()) {
    RestartLevel(true);
    return;
  }

  if (Server->newEpisodeRequested()) {
    UE_LOG(LogCarla, Log, TEXT("New episode requested"));
    RestartLevel(false);
    return;
  }

  SendReward(*Server, Player->GetPlayerState(), Cameras);
  TryReadControl(*Server, *Player);
}

void CarlaGameController::RestartLevel(bool ServerNeedsRestart)
{
  UE_LOG(LogCarla, Log, TEXT("Restarting..."));
  bServerNeedsRestart = ServerNeedsRestart;
  Player->RestartLevel();
}
