// CARLA, Copyright (C) 2017 Computer Vision Center (CVC)

#pragma once

#include "Array.h"

class APlayerStart;
class AController;

/// Base class for a CARLA game controller.
class CARLA_API CarlaGameControllerBase
{
public:

  virtual ~CarlaGameControllerBase() {}

  virtual APlayerStart *ChoosePlayerStart(const TArray<APlayerStart *> &AvailableStartSpots) = 0;

  virtual void RegisterPlayer(AController *NewPlayer) = 0;

  virtual void Tick(float DeltaSeconds) {}
};
