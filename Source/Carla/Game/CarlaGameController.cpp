// CARLA, Copyright (C) 2017 Computer Vision Center (CVC)

#include "Carla.h"
#include "CarlaGameController.h"

#include "GameFramework/PlayerStart.h"

#include "CarlaPlayerState.h"
#include "CarlaVehicleController.h"

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

static void ReadSceneInit(carla::CarlaServer &Server)
{
  carla::Mode mode;
  uint32 scene;
  while (!Server.tryReadSceneInit(mode, scene)) {
    // wait.
  }
}

static void SendSceneValues(carla::CarlaServer &Server, const TArray<FTransform> &Transforms)
{
  carla::Scene_Values sceneValues;
  sceneValues.possible_Positions.reserve(Transforms.Num());
  for (const FTransform &Transform : Transforms) {
    const FVector &Location = Transform.GetLocation();
    sceneValues.possible_Positions.push_back({Location.X, Location.Y});
  }
  Server.sendSceneValues(sceneValues);
}

static uint32 ReadEpisodeStart(carla::CarlaServer &Server)
{
  uint32 StartIndex;
  uint32 EndIndex;
  while (!Server.tryReadEpisodeStart(StartIndex, EndIndex)) {
    // wait.
  }
  return StartIndex;
}

static bool TryReadEpisodeStart(carla::CarlaServer &Server, uint32 &StartIndex)
{
  uint32 EndIndex;
  return Server.tryReadEpisodeStart(StartIndex, EndIndex);
}

static void SendReward(carla::CarlaServer &Server, const ACarlaPlayerState &PlayerState)
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
  // Set(reward.image_width, );
  // Set(reward.image_height, );
  // Set(reward.image_rgb_0, );
  // Set(reward.image_rgb_1, );
  // Set(reward.image_depth_0, );
  // Set(reward.image_depth_1, );
  Server.sendReward(reward);
}

static void ReadControl(carla::CarlaServer &Server, ACarlaVehicleController &Player)
{
  float steer;
  float throttle;
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
  Server(MakeUnique<carla::CarlaServer>(2000u, 2001u, 2002u)),
  Player(nullptr)
{
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

}

void CarlaGameController::Tick(float DeltaSeconds)
{
  check(Player != nullptr);
  uint32 StartIndex;
  if (bIsResetting) {
    // Resetting the world.
    ReadSceneInit(*Server);
    SendSceneValues(*Server, AvailableStartTransforms);
    StartIndex = ReadEpisodeStart(*Server);
    Player->SetActorTransform(AvailableStartTransforms[StartIndex]);
    Server->sendEndReset();
    bIsResetting = false;
  } else if (TryReadEpisodeStart(*Server, StartIndex)) {
    // Handle request for resetting the world.
    Player->SetActorTransform(AvailableStartTransforms[StartIndex]);
    Server->sendEndReset();
  } else {
    // Regular tick.
    SendReward(*Server, Player->GetPlayerState());
    ReadControl(*Server, *Player);
  }
}
