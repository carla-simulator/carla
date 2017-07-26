// CARLA, Copyright (C) 2017 Computer Vision Center (CVC)

#pragma once

#include "CarlaGameControllerBase.h"

class ACarlaVehicleController;
class CarlaServer;

/// Implements remote control of game and player.
class CARLA_API CarlaGameController : public CarlaGameControllerBase
{
public:

  explicit CarlaGameController();

  ~CarlaGameController();

  virtual void Initialize(UCarlaSettings &CarlaSettings) override;

  virtual APlayerStart *ChoosePlayerStart(const TArray<APlayerStart *> &AvailableStartSpots) override;

  virtual void RegisterPlayer(AController &NewPlayer) override;

  virtual void BeginPlay() override;

  virtual void Tick(float DeltaSeconds) override;

private:

  void RestartLevel();

  TUniquePtr<CarlaServer> Server;

  ACarlaVehicleController *Player = nullptr;

  UCarlaSettings *CarlaSettings = nullptr;
};
