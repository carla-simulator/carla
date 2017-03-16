// CARLA, Copyright (C) 2017 Computer Vision Center (CVC)

#include "Carla.h"
#include "CarlaGameController.h"

#include "GameFramework/PlayerStart.h"

#include "CarlaPlayerState.h"
#include "CarlaVehicleController.h"
#include "SceneCaptureCamera.h"

#include <carla/CarlaServer.h>

// =============================================================================
// -- static local methods -----------------------------------------------------
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

static void ReadSceneInit(carla::CarlaServer &Server)
{
  carla::Mode mode;
  uint32 scene;
  UE_LOG(LogCarla, Log, TEXT("Waiting for tryReadSceneInit..."));
  while (!Server.tryReadSceneInit(mode, scene)) {
    // wait.
  }
}

static void SendSceneValues(carla::CarlaServer &Server, const TArray<FTransform> &Transforms)
{
  carla::Scene_Values sceneValues;
  sceneValues.possible_positions.reserve(Transforms.Num());
  for (const FTransform &Transform : Transforms) {
    const FVector &Location = Transform.GetLocation();
    UE_LOG(LogCarla, Log, TEXT("Start position {%f, %f}"), Location.X, Location.Y);
    sceneValues.possible_positions.push_back({Location.X, Location.Y});
  }
  UE_LOG(LogCarla, Log, TEXT("Send scene values"));
  Server.sendSceneValues(sceneValues);
}

static uint32 ReadEpisodeStart(carla::CarlaServer &Server)
{
  uint32 StartIndex;
  uint32 EndIndex;
  UE_LOG(LogCarla, Log, TEXT("Waiting for tryReadEpisodeStart..."));
  while (!Server.tryReadEpisodeStart(StartIndex, EndIndex)) {
    // wait.
  }
  UE_LOG(LogCarla, Log, TEXT("tryReadEpisodeStart received: %d - %d"), StartIndex, EndIndex);
  return StartIndex;
}

static bool TryReadEpisodeStart(carla::CarlaServer &Server, uint32 &StartIndex)
{
  uint32 EndIndex;
  UE_LOG(LogCarla, Log, TEXT("tryReadEpisodeStart"));
  return Server.tryReadEpisodeStart(StartIndex, EndIndex);
}

static void SendReward(
    carla::CarlaServer &Server,
    const ACarlaPlayerState &PlayerState,
    const std::array<const ASceneCaptureCamera *, 2u> &Cameras)
{
  carla::Reward_Values reward;
  reward.timestamp = FMath::RoundHalfToZero(FPlatformTime::Seconds());
  Set(reward.player_location, PlayerState.GetLocation());
  Set(reward.player_orientation, PlayerState.GetOrientation());
  Set(reward.player_acceleration, PlayerState.GetAcceleration());
  // Set(reward.player_acceleration, );
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
  UE_LOG(LogCarla, Log, TEXT("Send reward"));
  Server.sendReward(reward);
}

static void ReadControl(carla::CarlaServer &Server, ACarlaVehicleController &Player)
{
  float steer;
  float throttle;
  UE_LOG(LogCarla, Log, TEXT("Read control"));
  if (Server.tryReadControl(steer, throttle)) {
    Player.SetSteeringInput(steer);
    Player.SetThrottleInput(throttle);
#ifdef WITH_EDITOR
    UE_LOG(LogCarla, Log, TEXT("Read control: steer = %f, throttle = %f"), steer, throttle);
#endif // WITH_EDITOR
  }
}

// =============================================================================
// -- CarlaGameController ------------------------------------------------------
// =============================================================================

CarlaGameController::CarlaGameController() :
  Server(MakeUnique<carla::CarlaServer>(2001u, 2002u, 2000u)),
  Player(nullptr),
  Cameras({{nullptr}})
{
  UE_LOG(LogCarla, Log, TEXT("Initializing CarlaServer"));
  Server->init(1u);
}

APlayerStart *CarlaGameController::ChoosePlayerStart(
    const TArray<APlayerStart *> &AvailableStartSpots)
{
  if (AvailableStartTransforms.Num() == 0) {
    AvailableStartTransforms.Reserve(AvailableStartSpots.Num());
    for (APlayerStart *StartSpot : AvailableStartSpots) {
      AvailableStartTransforms.Add(StartSpot->GetActorTransform());
    }
  }
  return AvailableStartSpots[0u];
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
  uint32 StartIndex;
  if (bIsResetting) {
    UE_LOG(LogCarla, Log, TEXT("Resetting the world"));
    // Resetting the world.
    ReadSceneInit(*Server);
    SendSceneValues(*Server, AvailableStartTransforms);
    StartIndex = ReadEpisodeStart(*Server);
    Player->SetActorTransform(AvailableStartTransforms[StartIndex]);
    Server->sendEndReset();
    bIsResetting = false;
  } else if (TryReadEpisodeStart(*Server, StartIndex)) {
    // Handle request for resetting the world.
    UE_LOG(LogCarla, Log, TEXT("Handle request for resetting the world"));
    Player->SetActorTransform(AvailableStartTransforms[StartIndex]);
    Server->sendEndReset();
  } else {
    // Regular tick.
    UE_LOG(LogCarla, Log, TEXT("Tick!"));
    SendReward(*Server, Player->GetPlayerState(), Cameras);
    ReadControl(*Server, *Player);
  }
}
