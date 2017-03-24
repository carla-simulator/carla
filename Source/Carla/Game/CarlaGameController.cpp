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

// Wait for the scene init to be sent, return false if we need to restart the
// server.
/// @todo At the moment we just ignored what it is sent.
static bool ReadSceneInit(carla::CarlaServer &Server)
{
  carla::Mode Mode;
  uint32 Scene;
  bool Success = false;
  UE_LOG(LogCarlaServer, Log, TEXT("(tryReadSceneInit) Waiting for client..."));
  while (!Success) {
    if (!Server.tryReadSceneInit(Mode, Scene, Success))
      return false;
  }
  return true;
}

// Send the available start spots to the client and wait for them to answer.
// Return false if the server needs restart.
static bool SendAndReadSceneValues(
    carla::CarlaServer &Server,
    const TArray<APlayerStart *> &AvailableStartSpots,
    uint32 &StartIndex)
{
  check(AvailableStartSpots.Num() > 0);
  // Retrieve the location of each player start.
  carla::Scene_Values sceneValues;
  sceneValues.possible_positions.reserve(AvailableStartSpots.Num());
  for (APlayerStart *StartSpot : AvailableStartSpots) {
    check(StartSpot != nullptr);
    const FVector &Location = StartSpot->GetActorLocation();
    sceneValues.possible_positions.push_back({Location.X, Location.Y});
  }
  // Send the positions.
  /// @todo At the moment we don't send the cameras' projection matrices.
  UE_LOG(LogCarlaServer, Log, TEXT("Sending %d available start positions"), sceneValues.possible_positions.size());
  if (!Server.sendSceneValues(sceneValues))
    return false;
  // Wait till we receive an answer.
  uint32 EndIndex;
  bool Success = false;
  UE_LOG(LogCarlaServer, Log, TEXT("(tryReadEpisodeStart) Waiting for client..."));
  while (!Success) {
    if (!Server.tryReadEpisodeStart(StartIndex, EndIndex, Success))
      return false;
  }
  UE_LOG(LogCarlaServer, Log, TEXT("Episode start received: { StartIndex = %d, EndIndex = %d }"), StartIndex, EndIndex);
  // Make sure the index is in range.
  if (StartIndex >= AvailableStartSpots.Num()) {
    UE_LOG(
        LogCarlaServer,
        Error,
        TEXT("Requested start position #%d but we only have %d, using first position instead"),
        StartIndex,
        AvailableStartSpots.Num());
    StartIndex = 0u;
  }
  return true;
}

static bool SendReward(
    carla::CarlaServer &Server,
    const ACarlaPlayerState &PlayerState)
{
  auto reward = std::make_unique<carla::Reward_Values>();
  reward->timestamp = PlayerState.GetTimeStamp();
  Set(reward->player_location, PlayerState.GetLocation());
  Set(reward->player_orientation, PlayerState.GetOrientation());
  Set(reward->player_acceleration, PlayerState.GetAcceleration());
  Set(reward->forward_speed, PlayerState.GetForwardSpeed());
  Set(reward->collision_car, PlayerState.GetCollisionIntensityCars());
  Set(reward->collision_pedestrian, PlayerState.GetCollisionIntensityPedestrians());
  Set(reward->collision_general, PlayerState.GetCollisionIntensityOther());
  Set(reward->intersect_other_lane, PlayerState.GetOtherLaneIntersectionFactor());
  Set(reward->intersect_offroad, PlayerState.GetOffRoadIntersectionFactor());
  { // Add images.
    using CPS = ACarlaPlayerState;
    auto &ImageRGB0 = PlayerState.GetImage(CPS::ImageRGB0);
    if (ImageRGB0.BitMap.Num() > 0) {
      // Do not add any camera if first is invalid, also assume all the images
      // have the same size.
      reward->image_width = ImageRGB0.SizeX;
      reward->image_height = ImageRGB0.SizeY;
      Set(reward->image_rgb_0, ImageRGB0.BitMap);
      Set(reward->image_rgb_1, PlayerState.GetImage(CPS::ImageRGB1).BitMap);
      Set(reward->image_depth_0, PlayerState.GetImage(CPS::ImageDepth0).BitMap);
      Set(reward->image_depth_1, PlayerState.GetImage(CPS::ImageDepth1).BitMap);
    }
  }
  UE_LOG(LogCarlaServer, Log, TEXT("Sending reward"));
  return Server.sendReward(reward.release());
}

static bool TryReadControl(carla::CarlaServer &Server, ACarlaVehicleController &Player)
{
  float Steer;
  float Throttle;
  bool Success = false;
  bool Result = Server.tryReadControl(Steer, Throttle, Success);
  if (Result && Success) {
    UE_LOG(LogCarlaServer, Log, TEXT("Read control: { Steer = %f, Throttle = %f }"), Steer, Throttle);
    Player.SetSteeringInput(Steer);
    Player.SetThrottleInput(Throttle);
  }
  return Result;
}

// =============================================================================
// -- CarlaGameController ------------------------------------------------------
// =============================================================================

CarlaGameController::CarlaGameController() :
  Server(MakeUnique<carla::CarlaServer>(2001u, 2002u, 2000u)),
  Player(nullptr) {}

CarlaGameController::~CarlaGameController()
{
  UE_LOG(LogCarlaServer, Log, TEXT("Destroying CarlaGameController..."));
}

void CarlaGameController::Initialize()
{
  if (bServerNeedsRestart) {
    UE_LOG(LogCarlaServer, Log, TEXT("Initializing CarlaServer"));
    if (Server->init(1u) && ReadSceneInit(*Server)) {
      bServerNeedsRestart = false;
    } else {
      UE_LOG(LogCarlaServer, Warning, TEXT("Failed to initialize, server needs restart"));
    }
  }
}

APlayerStart *CarlaGameController::ChoosePlayerStart(
    const TArray<APlayerStart *> &AvailableStartSpots)
{
  uint32 StartIndex;
  if (!SendAndReadSceneValues(*Server, AvailableStartSpots, StartIndex)) {
    UE_LOG(LogCarlaServer, Warning, TEXT("Unable to choose start position, server needs restart"));
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
  if (!bServerNeedsRestart) {
    UE_LOG(LogCarlaServer, Log, TEXT("Ready to play, notifying client"));
    if (!Server->sendEndReset()) {
      bServerNeedsRestart = true;
      UE_LOG(LogCarlaServer, Warning, TEXT("Unable to send end reset, server needs restart"));
    }
  }
}

void CarlaGameController::Tick(float DeltaSeconds)
{
  check(Player != nullptr);
  if (bServerNeedsRestart || Server->needsRestart() || !TickServer()) {
    UE_LOG(LogCarlaServer, Warning, TEXT("Client disconnected, server needs restart"));
    bServerNeedsRestart = true;
    RestartLevel();
  }
}

bool CarlaGameController::TickServer()
{
  // Check if the client requested a new episode.
  bool bNewEpisodeRequested = false;
  if (!Server->newEpisodeRequested(bNewEpisodeRequested)) {
    return false;
  } else if (bNewEpisodeRequested) {
    UE_LOG(LogCarlaServer, Log, TEXT("New episode requested"));
    RestartLevel();
    return true;
  }

  // Send reward and try to read control.
  return
      SendReward(*Server, Player->GetPlayerState()) &&
      TryReadControl(*Server, *Player);
}

void CarlaGameController::RestartLevel()
{
  UE_LOG(LogCarlaServer, Log, TEXT("Restarting the level..."));
  Player->RestartLevel();
}
