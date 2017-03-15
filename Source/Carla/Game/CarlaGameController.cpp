// CARLA, Copyright (C) 2017 Computer Vision Center (CVC)

#include "Carla.h"
#include "CarlaGameController.h"

#include "GameFramework/PlayerStart.h"

#include "CarlaVehicleController.h"

// -----------------------------------------------------------------------------

#include <vector>

struct Position {
  float x;
  float y;
};

struct Scene_Values {
  std::vector<Position> _possible_Positions;
};

class CarlaServer {
public:
  void sendWorld() {}
  bool tryReadSceneInit(int &mode, int &scene) { mode = 0; return true; }
  void sendSceneValues(const Scene_Values &) {}
  bool tryReadEpisodeStart(size_t &start_index, size_t &end_index) { start_index = 0u; return true; }
  void sendEndReset() {}
};

// =============================================================================
// -- static local methods -----------------------------------------------------
// =============================================================================

static void ReadSceneInit(CarlaServer &Server)
{
  int mode;
  int scene;
  while (!Server.tryReadSceneInit(mode, scene)) {
    // wait.
  }
}

static void SendSceneValues(CarlaServer &Server, const TArray<FTransform> &Transforms)
{
  Scene_Values sceneValues;
  sceneValues._possible_Positions.reserve(Transforms.Num());
  for (const FTransform &Transform : Transforms) {
    const FVector &Location = Transform.GetLocation();
    sceneValues._possible_Positions.push_back({Location.X, Location.Y});
  }
  Server.sendSceneValues(sceneValues);
}

static size_t ReadEpisodeStart(CarlaServer &Server)
{
  size_t StartIndex;
  size_t EndIndex;
  while (!Server.tryReadEpisodeStart(StartIndex, EndIndex)) {
    // wait.
  }
  return StartIndex;
}

static bool TryReadEpisodeStart(CarlaServer &Server, size_t &StartIndex)
{
  size_t EndIndex;
  return Server.tryReadEpisodeStart(StartIndex, EndIndex);
}

static void SendReward(CarlaServer &Server, ACarlaVehicleController &Player)
{

}

static void ReadControl(CarlaServer &Server, ACarlaVehicleController &Player)
{

}

// =============================================================================
// -- CarlaGameController ------------------------------------------------------
// =============================================================================

CarlaGameController::CarlaGameController() :
  Server(MakeUnique<CarlaServer>()),
  Player(nullptr)
{
  Server->sendWorld();
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
  size_t StartIndex;
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
    SendReward(*Server, *Player);
    ReadControl(*Server, *Player);
  }
}
