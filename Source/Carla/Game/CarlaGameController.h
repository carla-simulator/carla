// CARLA, Copyright (C) 2017 Computer Vision Center (CVC)

#pragma once

#include "CarlaGameControllerBase.h"

class ACarlaVehicleController;

namespace carla {
  class CarlaServer;
}

/// Implements remote control of game and player.
class CARLA_API CarlaGameController : public CarlaGameControllerBase
{
public:

  CarlaGameController();

  ~CarlaGameController();

  virtual void Initialize() override;

  virtual APlayerStart *ChoosePlayerStart(const TArray<APlayerStart *> &AvailableStartSpots) override;

  virtual void RegisterPlayer(AController &NewPlayer) override;

  virtual void RegisterCaptureCamera(const ASceneCaptureCamera &CaptureCamera) override;

  virtual void BeginPlay() override;

  virtual void Tick(float DeltaSeconds) override;

private:

  void RestartLevel(bool ServerNeedsRestart);

  TUniquePtr<carla::CarlaServer> Server;

  ACarlaVehicleController *Player;

  std::array<const ASceneCaptureCamera *, 2u> Cameras;

  bool bServerNeedsRestart = true;
};
