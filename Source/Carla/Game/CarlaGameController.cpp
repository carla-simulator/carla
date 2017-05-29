// CARLA, Copyright (C) 2017 Computer Vision Center (CVC)

#include "Carla.h"
#include "CarlaGameController.h"

#include "GameFramework/PlayerStart.h"

#include "CarlaPlayerState.h"
#include "CarlaSettings.h"
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

static carla::ImageType GetImageType(EPostProcessEffect PostProcessEffect)
{
  /// @todo #19
  return (PostProcessEffect == EPostProcessEffect::Depth ? carla::DEPTH : carla::IMAGE);
}

static void Set(carla::Image &cImage, const FCapturedImage &uImage)
{
  if (uImage.BitMap.Num() > 0) {
    cImage.width = uImage.SizeX;
    cImage.height = uImage.SizeY;
    cImage.type = GetImageType(uImage.PostProcessEffect);
    cImage.image.reserve(uImage.BitMap.Num());
    for (const auto &color : uImage.BitMap) {
      cImage.image.emplace_back();
      cImage.image.back().R = color.R;
      cImage.image.back().G = color.G;
      cImage.image.back().B = color.B;
      cImage.image.back().A = color.A;
    }
    check(cImage.image.size() == (cImage.width * cImage.height));
#ifdef CARLA_SERVER_EXTRA_LOG
    {
      const auto Size = cImage.image.size();
      const FString Type = (cImage.type == carla::IMAGE ? TEXT("IMAGE") : TEXT("DEPTH"));
      UE_LOG(LogCarlaServer, Log, TEXT("Sending image %dx%d (%d) %s"), cImage.width, cImage.height, Size, *Type);
    }
  } else {
    UE_LOG(LogCarlaServer, Warning, TEXT("Sending empty image"));
#endif // CARLA_SERVER_EXTRA_LOG
  }
}

// =============================================================================
// -- Other static methods -----------------------------------------------------
// =============================================================================

// Wait for a new episode to be received and update CarlaSettings. Return false
// if we need to restart the server.
static bool WaitForNewEpisode(
    carla::CarlaServer &Server,
    UCarlaSettings &CarlaSettings)
{
  std::string IniStr;
  bool Success = false;
  while (!Success) {
    if (!Server.newEpisodeRequested(IniStr, Success))
      return false;
  }
  CarlaSettings.LoadSettingsFromString(IniStr.c_str());
  return true;
}

static bool TryReadNewEpisode(
    carla::CarlaServer &Server,
    UCarlaSettings &CarlaSettings,
    bool &bNewEpisodeRequested)
{
  std::string IniStr;
  bool Success = false;
  bool Result = Server.newEpisodeRequested(IniStr, Success);
  if (Result && Success) {
    CarlaSettings.LoadSettingsFromString(IniStr.c_str());
  }
  return Result;
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
  UE_LOG(LogCarlaServer, Log, TEXT("Sending %d available start positions"), sceneValues.possible_positions.size());
  if (!Server.sendSceneValues(sceneValues))
    return false;
  // Wait till we receive an answer.
  uint32 EndIndex = 0u;
  bool Success = false;
  UE_LOG(LogCarlaServer, Log, TEXT("(tryReadEpisodeStart) Waiting for client..."));
  while (!Success) {
    if (!Server.tryReadEpisodeStart(StartIndex, EndIndex, Success))
      return false;
  }
  UE_LOG(LogCarlaServer, Log, TEXT("Episode start received: { StartIndex = %d, EndIndex = %d }"), StartIndex, EndIndex);
  // Make sure the index is in range.
  if (StartIndex >= static_cast<uint32>(AvailableStartSpots.Num())) {
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
  reward->platform_timestamp = PlayerState.GetPlatformTimeStamp();
  reward->game_timestamp = PlayerState.GetGameTimeStamp();
  Set(reward->player_location, PlayerState.GetLocation());
  Set(reward->player_orientation, PlayerState.GetOrientation());
  Set(reward->player_acceleration, PlayerState.GetAcceleration());
  Set(reward->forward_speed, PlayerState.GetForwardSpeed());
  Set(reward->collision_car, PlayerState.GetCollisionIntensityCars());
  Set(reward->collision_pedestrian, PlayerState.GetCollisionIntensityPedestrians());
  Set(reward->collision_general, PlayerState.GetCollisionIntensityOther());
  Set(reward->intersect_other_lane, PlayerState.GetOtherLaneIntersectionFactor());
  Set(reward->intersect_offroad, PlayerState.GetOffRoadIntersectionFactor());
  if (PlayerState.HasImages()) {
    reward->images.reserve(PlayerState.GetImages().Num());
    for (const auto &Image : PlayerState.GetImages()) {
      reward->images.emplace_back();
      Set(reward->images.back(), Image);
    }
  }
#ifdef CARLA_SERVER_EXTRA_LOG
  UE_LOG(LogCarlaServer, Log, TEXT("Sending reward"));
#endif // CARLA_SERVER_EXTRA_LOG
  return Server.sendReward(reward.release());
}

static bool TryReadControl(carla::CarlaServer &Server, ACarlaVehicleController &Player)
{
  carla::Control_Values Control;
  bool Success = false;
  bool Result = Server.tryReadControl(Control, Success);
  if (Result && Success) {
#ifdef CARLA_SERVER_EXTRA_LOG
    UE_LOG(
        LogCarlaServer,
        Log,
        TEXT("Read control: { Steer = %f, Throttle = %f, Brake = %f, Handbrake = %s, Reverse = %s }"),
        Control.steer,
        Control.gas,
        Control.brake,
        (Control.hand_brake ? TEXT("True") : TEXT("False")),
        (Control.reverse ? TEXT("True") : TEXT("False")));
#endif // CARLA_SERVER_EXTRA_LOG
    Player.SetSteeringInput(Control.steer);
    Player.SetThrottleInput(Control.gas);
    Player.SetBrakeInput(Control.brake);
    Player.SetHandbrakeInput(Control.hand_brake);
    Player.SetReverse(Control.reverse);
  }
  return Result;
}

// =============================================================================
// -- CarlaGameController ------------------------------------------------------
// =============================================================================

CarlaGameController::CarlaGameController(uint32 WorldPort, uint32 WritePort, uint32 ReadPort) :
  Server(MakeUnique<carla::CarlaServer>(WritePort, ReadPort, WorldPort)),
  Player(nullptr) {}

CarlaGameController::~CarlaGameController() {}

void CarlaGameController::Initialize(UCarlaSettings &InCarlaSettings)
{
  CarlaSettings = &InCarlaSettings;
  if (bServerNeedsRestart) {
    UE_LOG(LogCarlaServer, Log, TEXT("Initializing CarlaServer"));
    if (WaitForNewEpisode(*Server, *CarlaSettings)) {
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
  if (!TryReadNewEpisode(*Server, *CarlaSettings, bNewEpisodeRequested)) {
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
