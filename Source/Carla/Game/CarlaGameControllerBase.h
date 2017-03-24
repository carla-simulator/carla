// CARLA, Copyright (C) 2017 Computer Vision Center (CVC)

#pragma once

#include "Array.h"

class AController;
class APlayerStart;

/// Base class for a CARLA game controller.
class CARLA_API CarlaGameControllerBase
{
public:

  virtual ~CarlaGameControllerBase() {}

  virtual void Initialize() = 0;

  virtual APlayerStart *ChoosePlayerStart(const TArray<APlayerStart *> &AvailableStartSpots) = 0;

  virtual void RegisterPlayer(AController &NewPlayer) = 0;

  virtual void BeginPlay() = 0;

  virtual void Tick(float DeltaSeconds) = 0;
};
