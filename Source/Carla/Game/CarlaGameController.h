// CARLA, Copyright (C) 2017 Computer Vision Center (CVC)

#pragma once

#include "CarlaGameControllerBase.h"

class CarlaServer;
class ACarlaVehicleController;

/// Implements remote control of game and player.
class CARLA_API CarlaGameController : public CarlaGameControllerBase
{
public:

  CarlaGameController();

  virtual APlayerStart *ChoosePlayerStart(const TArray<APlayerStart *> &AvailableStartSpots) override;

  virtual void RegisterPlayer(AController *NewPlayer) override;

  virtual void Tick(float DeltaSeconds) override;

private:

  TUniquePtr<CarlaServer> Server;

  ACarlaVehicleController *Player;

  TArray<FTransform> AvailableStartTransforms;

  bool bIsResetting = true;
};
