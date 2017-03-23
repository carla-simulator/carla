// CARLA, Copyright (C) 2017 Computer Vision Center (CVC)

#include "Carla.h"
#include "CarlaGameController.h"

#include "GameFramework/PlayerStart.h"

#include "CarlaPlayerState.h"
#include "CarlaVehicleController.h"
#include "SceneCaptureCamera.h"

#include <carla/CarlaServer.h>

#define CSTEXT(text) TEXT("CarlaServer: " text)

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

static void Set(std::vector<carla::Color> &cImage, const TArray<FColor> &BitMap)
{
  if (BitMap.Num() > 0) {
    cImage.reserve(BitMap.Num());
    for (const auto &color : BitMap) {
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
  UE_LOG(LogCarla, Log, CSTEXT("Waiting for tryReadSceneInit..."));
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
    check(StartSpot != nullptr);
    const FVector &Location = StartSpot->GetActorLocation();
    UE_LOG(LogCarla, Log, CSTEXT("Found start position {%f, %f}"), Location.X, Location.Y);
    sceneValues.possible_positions.push_back({Location.X, Location.Y});
  }
  // Send scene values.
  UE_LOG(LogCarla, Log, CSTEXT("Send scene values: %d positions"), sceneValues.possible_positions.size());
  Server.sendSceneValues(sceneValues);
  // Wait till we receive the answer.
  uint32 EndIndex;
  UE_LOG(LogCarla, Log, CSTEXT("Waiting for episode start..."));
  while (!Server.tryReadEpisodeStart(StartIndex, EndIndex)) {
    if (Server.needsRestart())
      return false;
  }
  UE_LOG(LogCarla, Log, CSTEXT("Episode start received: %d -> %d"), StartIndex, EndIndex);
  // Make sure the index is in range.
  if (StartIndex >= AvailableStartSpots.Num()) {
    UE_LOG(LogCarla, Warning, CSTEXT("Received invalid start index, using zero instead"));
    StartIndex = 0u;
  }
  return true;
}

static void SendReward(
    carla::CarlaServer &Server,
    const ACarlaPlayerState &PlayerState)
{
  carla::Reward_Values reward;
  reward.timestamp = PlayerState.GetTimeStamp();
  Set(reward.player_location, PlayerState.GetLocation());
  Set(reward.player_orientation, PlayerState.GetOrientation());
  Set(reward.player_acceleration, PlayerState.GetAcceleration());
  Set(reward.forward_speed, PlayerState.GetForwardSpeed());
  Set(reward.collision_car, PlayerState.GetCollisionIntensityCars());
  Set(reward.collision_pedestrian, PlayerState.GetCollisionIntensityPedestrians());
  Set(reward.collision_general, PlayerState.GetCollisionIntensityOther());
  Set(reward.intersect_other_lane, PlayerState.GetOtherLaneIntersectionFactor());
  Set(reward.intersect_offroad, PlayerState.GetOffRoadIntersectionFactor());
  { // Add images.
    using CPS = ACarlaPlayerState;
    auto &ImageRGB0 = PlayerState.GetImage(CPS::ImageRGB0);
    if (ImageRGB0.BitMap.Num() > 0) {
      // Do not add any camera if first is invalid, also assume all the images
      // have the same size.
      reward.image_width = ImageRGB0.SizeX;
      reward.image_height = ImageRGB0.SizeY;
      Set(reward.image_rgb_0, ImageRGB0.BitMap);
      Set(reward.image_rgb_1, PlayerState.GetImage(CPS::ImageRGB1).BitMap);
      Set(reward.image_depth_0, PlayerState.GetImage(CPS::ImageDepth0).BitMap);
      Set(reward.image_depth_1, PlayerState.GetImage(CPS::ImageDepth1).BitMap);
    }
  }
  UE_LOG(LogCarla, Log, CSTEXT("Sending reward"));
  Server.sendReward(reward);
}

static void TryReadControl(carla::CarlaServer &Server, ACarlaVehicleController &Player)
{
  float steer;
  float throttle;
  if (Server.tryReadControl(steer, throttle)) {
    Player.SetSteeringInput(steer);
    Player.SetThrottleInput(throttle);
    UE_LOG(LogCarla, Log, CSTEXT("Read control: steer = %f, throttle = %f"), steer, throttle);
  }
}

// =============================================================================
// -- CarlaGameController ------------------------------------------------------
// =============================================================================

CarlaGameController::CarlaGameController() :
  Server(MakeUnique<carla::CarlaServer>(2001u, 2002u, 2000u)),
  Player(nullptr) {}

CarlaGameController::~CarlaGameController()
{
  UE_LOG(LogCarla, Log, CSTEXT("Destroying CarlaGameController..."));
}

void CarlaGameController::Initialize()
{
  if (bServerNeedsRestart) {
    UE_LOG(LogCarla, Log, CSTEXT("Initializing CarlaServer"));
    Server->init(1u);
    if (ReadSceneInit(*Server)) {
      bServerNeedsRestart = false;
    } else {
      UE_LOG(LogCarla, Warning, CSTEXT("Read scene init failed, server needs restart"));
    }
  }
}

APlayerStart *CarlaGameController::ChoosePlayerStart(
    const TArray<APlayerStart *> &AvailableStartSpots)
{
  uint32 StartIndex;
  if (!SendAndReadSceneValues(*Server, AvailableStartSpots, StartIndex)) {
    UE_LOG(LogCarla, Warning, CSTEXT("Read scene values failed, server needs restart"));
    StartIndex = 0u;
  }
  return AvailableStartSpots[StartIndex];
}

void CarlaGameController::RegisterPlayer(AController &NewPlayer)
{
  Player = Cast<ACarlaVehicleController>(&NewPlayer);
  check(Player != nullptr);
}

void CarlaGameController::BeginPlay()
{
  Server->sendEndReset();
}

void CarlaGameController::Tick(float DeltaSeconds)
{
  check(Player != nullptr);
  if (bServerNeedsRestart || Server->needsRestart()) {
    RestartLevel(true);
    return;
  }

  if (Server->newEpisodeRequested()) {
    UE_LOG(LogCarla, Log, CSTEXT("New episode requested"));
    RestartLevel(false);
    return;
  }

  SendReward(*Server, Player->GetPlayerState());
  TryReadControl(*Server, *Player);
}

void CarlaGameController::RestartLevel(bool ServerNeedsRestart)
{
  UE_LOG(LogCarla, Log, CSTEXT("Restarting..."));
  bServerNeedsRestart = ServerNeedsRestart;
  Player->RestartLevel();
}
