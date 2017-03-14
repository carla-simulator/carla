// CARLA, Copyright (C) 2017 Computer Vision Center (CVC)

#pragma once

#include "CarlaGameControllerBase.h"

/// Mocks the CARLA game controller class for testing purposes.
class CARLA_API MockGameController : public CarlaGameControllerBase
{
public:

  virtual APlayerStart *ChoosePlayerStart(const TArray<APlayerStart *> &AvailableStartSpots) override;

  virtual void RegisterPlayer(AController *NewPlayer) override;
};
