// CARLA, Copyright (C) 2017 Computer Vision Center (CVC)

#pragma once

#include "CarlaGameControllerBase.h"
#include "MockGameControllerSettings.h"

/// Mocks the CARLA game controller class for testing purposes.
class CARLA_API MockGameController : public CarlaGameControllerBase
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
