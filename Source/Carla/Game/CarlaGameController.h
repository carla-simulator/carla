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

  virtual APlayerStart *ChoosePlayerStart(const TArray<APlayerStart *> &AvailableStartSpots) override;

  virtual void RegisterPlayer(AController &NewPlayer) override;

  virtual void RegisterCaptureCamera(const ASceneCaptureCamera &CaptureCamera) override;

  virtual void Tick(float DeltaSeconds) override;

private:

  TUniquePtr<carla::CarlaServer> Server;

  ACarlaVehicleController *Player;

  TArray<FTransform> AvailableStartTransforms;

  bool bIsResetting = true;
};
