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

  CarlaGameController(uint32 WorldPort, uint32 WritePort, uint32 ReadPort);

  ~CarlaGameController();

  virtual void Initialize(UCarlaSettings &CarlaSettings) override;

  virtual APlayerStart *ChoosePlayerStart(const TArray<APlayerStart *> &AvailableStartSpots) override;

  virtual void RegisterPlayer(AController &NewPlayer) override;

  virtual void BeginPlay() override;

  virtual void Tick(float DeltaSeconds) override;

private:

  /// Return false if the server needs restart.
  bool TickServer();

  void RestartLevel();

  TUniquePtr<carla::CarlaServer> Server;

  ACarlaVehicleController *Player = nullptr;

  UCarlaSettings *CarlaSettings = nullptr;

  bool bServerNeedsRestart = true;
};
