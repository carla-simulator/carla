// CARLA, Copyright (C) 2017 Computer Vision Center (CVC)

#pragma once

#include "Array.h"

class AController;
class APlayerStart;
class ASceneCaptureCamera;

/// Base class for a CARLA game controller.
class CARLA_API CarlaGameControllerBase
{
public:

  virtual ~CarlaGameControllerBase() {}

  virtual APlayerStart *ChoosePlayerStart(const TArray<APlayerStart *> &AvailableStartSpots) = 0;

  virtual void RegisterPlayer(AController &NewPlayer) = 0;

  virtual void RegisterCaptureCamera(const ASceneCaptureCamera &CaptureCamera) = 0;

  virtual void Tick(float DeltaSeconds) = 0;
};
