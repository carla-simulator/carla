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
  static_assert(
      (PostProcessEffect::ToUInt(EPostProcessEffect::None) == carla::IMAGE) &&
      (PostProcessEffect::ToUInt(EPostProcessEffect::SceneFinal) == carla::SCENE_FINAL) &&
      (PostProcessEffect::ToUInt(EPostProcessEffect::Depth) == carla::DEPTH) &&
      (PostProcessEffect::ToUInt(EPostProcessEffect::SemanticSegmentation) == carla::SEMANTIC_SEG),
      "Enum values do not match");
  return carla::ImageType(PostProcessEffect::ToUInt(PostProcessEffect));
}

static void Set(carla::Image &cImage, const FCapturedImage &uImage)
{
  if (uImage.BitMap.Num() > 0) {
    cImage.make(GetImageType(uImage.PostProcessEffect), uImage.SizeX, uImage.SizeY);
    for (auto i = 0u; i < cImage.size(); ++i) {
      cImage[i].R = uImage.BitMap[i].R;
      cImage[i].G = uImage.BitMap[i].G;
      cImage[i].B = uImage.BitMap[i].B;
      cImage[i].A = uImage.BitMap[i].A;
    }
    check(cImage.size() == (cImage.width() * cImage.height()));
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
      const auto Size = cImage.size();
      UE_LOG(LogCarlaServer, Log, TEXT("Sending image %dx%d (%d) %s"), cImage.width(), cImage.height(), Size, GetImageType(cImage.type()));
    }
  } else {
    UE_LOG(LogCarlaServer, Warning, TEXT("Sending empty image"));
#endif // CARLA_SERVER_EXTRA_LOG
  }
}

// =============================================================================
// -- Other static methods -----------------------------------------------------
// =============================================================================

static FString ToFString(const carla::CarlaString &String)
{
  const auto Result = FString(ANSI_TO_TCHAR(String.data()));
#ifdef CARLA_SERVER_EXTRA_LOG
  UE_LOG(LogCarlaServer, Log, TEXT("Received CarlaString:\n%s"), *Result);
#endif // CARLA_SERVER_EXTRA_LOG
  return Result;
}

// Wait for a new episode to be received and update CarlaSettings. Return false
// if we need to restart the server.
static bool WaitForNewEpisode(
    carla::CarlaServer &Server,
    UCarlaSettings &CarlaSettings)
{
  carla::CarlaString IniStr;
  bool Success = false;
  while (!Success) {
    if (!Server.newEpisodeRequested(IniStr, Success))
      return false;
  }
  CarlaSettings.LoadSettingsFromString(ToFString(IniStr));
  return true;
}

static bool TryReadNewEpisode(
    carla::CarlaServer &Server,
    UCarlaSettings &CarlaSettings,
    bool &bNewEpisodeRequested)
{
  carla::CarlaString IniStr;
  bool Success = false;
  bool Result = Server.newEpisodeRequested(IniStr, Success);
  if (Result && Success) {
    CarlaSettings.LoadSettingsFromString(ToFString(IniStr));
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
  sceneValues.possible_positions.clearAndResize(AvailableStartSpots.Num());
  for (auto i = 0u; i < sceneValues.possible_positions.size(); ++i) {
    auto *StartSpot = AvailableStartSpots[i];
    check(StartSpot != nullptr);
    const FVector &Location = StartSpot->GetActorLocation();
    sceneValues.possible_positions[i] = {Location.X, Location.Y};
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
  auto reward = MakeUnique<carla::Reward_Values>();
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
    reward->images.clearAndResize(PlayerState.GetImages().Num());
    for (auto i = 0u; i < reward->images.size(); ++i) {
      Set(reward->images[i], PlayerState.GetImages()[i]);
    }
  }
#ifdef CARLA_SERVER_EXTRA_LOG
  UE_LOG(LogCarlaServer, Log, TEXT("Sending reward"));
#endif // CARLA_SERVER_EXTRA_LOG
  return Server.sendReward(reward.Release());
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
