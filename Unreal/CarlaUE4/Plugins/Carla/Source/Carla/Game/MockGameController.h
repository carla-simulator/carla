// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "Game/CarlaGameControllerBase.h"

#include "Game/MockGameControllerSettings.h"

/// Mocks the CARLA game controller class for testing purposes.
class MockGameController : public ICarlaGameControllerBase
{
public:

  explicit MockGameController(FDataRouter &DataRouter, const FMockGameControllerSettings &Settings);

  virtual void Initialize(UCarlaSettings &CarlaSettings) final;

  virtual APlayerStart *ChoosePlayerStart(const TArray<APlayerStart *> &AvailableStartSpots) final;

  virtual void RegisterPlayer(AController &NewPlayer) final;

  virtual void BeginPlay() final;

  virtual void Tick(float DeltaSeconds) final;

private:

  FMockGameControllerSettings Settings;
};
