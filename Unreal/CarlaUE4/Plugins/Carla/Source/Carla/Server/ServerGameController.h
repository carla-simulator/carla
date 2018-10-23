// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "Game/CarlaGameControllerBase.h"

class FCarlaServer;

/// Implements remote control of game and player.
class FServerGameController : public ICarlaGameControllerBase
{
public:

  FServerGameController(FDataRouter &DataRouter);

  ~FServerGameController();

  virtual void Initialize(UCarlaSettings &CarlaSettings) final;

  virtual APlayerStart *ChoosePlayerStart(const TArray<APlayerStart *> &AvailableStartSpots) final;

  virtual void RegisterPlayer(AController &NewPlayer) final;

  virtual void BeginPlay() final;

  virtual void Tick(float DeltaSeconds) final;

private:

  void RestartLevel();

  TSharedPtr<FCarlaServer> Server;

  UCarlaSettings *CarlaSettings = nullptr;
};
