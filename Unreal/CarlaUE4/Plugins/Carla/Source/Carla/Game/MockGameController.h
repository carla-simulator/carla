// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "Game/CarlaGameControllerBase.h"

#include "Game/MockGameControllerSettings.h"

/// Mocks the CARLA game controller class for testing purposes.
class CARLA_API MockGameController : public ICarlaGameControllerBase
{
public:

  explicit MockGameController(const FMockGameControllerSettings &Settings);

  virtual void Initialize(UCarlaSettings &CarlaSettings) override;

  virtual APlayerStart *ChoosePlayerStart(const TArray<APlayerStart *> &AvailableStartSpots) override;

  virtual void RegisterPlayer(AController &NewPlayer) override;

  virtual void BeginPlay() override;

  virtual void Tick(float DeltaSeconds) override;

private:

  FMockGameControllerSettings Settings;
};
