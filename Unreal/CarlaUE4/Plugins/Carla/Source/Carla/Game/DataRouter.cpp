// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "Carla.h"
#include "DataRouter.h"

#include "Agent/AgentMap.h"
#include "Sensor/Sensor.h"

FDataRouter::FDataRouter() : Agents(MakeShared<FAgentMap>()) {}

FDataRouter::~FDataRouter() {}

void FDataRouter::RegisterPlayer(ACarlaVehicleController *InPlayer)
{
  if (Player == nullptr) {
    Player = InPlayer;
  } else {
    UE_LOG(
        LogCarla,
        Error,
        TEXT("FDataRouter: Trying to register a second player but only one is supported"));
  }
}

void FDataRouter::RegisterSensor(ASensor *InSensor)
{
  if (SensorDataSink.Get() != nullptr) {
    check(InSensor != nullptr);
    InSensor->SetSensorDataSink(SensorDataSink);
  } else {
    UE_LOG(
        LogCarla,
        Error,
        TEXT("FDataRouter: Trying to register a sensor but I don't have a SensorDataSink"));
  }
}

void FDataRouter::ApplyVehicleControl(const FVehicleControl &VehicleControl)
{

}
