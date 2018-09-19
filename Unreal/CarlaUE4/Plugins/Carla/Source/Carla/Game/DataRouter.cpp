// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "Carla.h"
#include "Carla/Game/DataRouter.h"

#include "Carla/Sensor/DeprecatedSensor.h"

void FDataRouter::RegisterSensor(ADeprecatedSensor &InSensor)
{
  if (SensorDataSink.IsValid()) {
    InSensor.SetSensorDataSink(SensorDataSink);
  } else {
    UE_LOG(
        LogCarla,
        Error,
        TEXT("FDataRouter: Trying to register a sensor but I don't have a SensorDataSink"));
  }
}

void FDataRouter::RestartLevel()
{
  if (Player != nullptr) {
    Player->RestartLevel();
    Player = nullptr;
  } else {
    UE_LOG(
        LogCarla,
        Error,
        TEXT("FDataRouter: Trying to restart level but I don't have any player registered"));
  }
}
