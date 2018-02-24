// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "Containers/Array.h"

class AController;
class APlayerStart;
class FDataRouter;
class UCarlaSettings;

/// Base class for a CARLA game controller.
class ICarlaGameControllerBase
{
public:

  ICarlaGameControllerBase(FDataRouter &DataRouter) : DataRouter(DataRouter) {}

  virtual ~ICarlaGameControllerBase() {}

  virtual void Initialize(UCarlaSettings &CarlaSettings) = 0;

  virtual APlayerStart *ChoosePlayerStart(const TArray<APlayerStart *> &AvailableStartSpots) = 0;

  virtual void RegisterPlayer(AController &NewPlayer) = 0;

  virtual void BeginPlay() = 0;

  virtual void Tick(float DeltaSeconds) = 0;

protected:

  FDataRouter &DataRouter;
};
