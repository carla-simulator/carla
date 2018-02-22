// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "Agent/AgentMap.h"
#include "Sensor/SensorDataSink.h"
#include "Vehicle/CarlaVehicleController.h"

class ACarlaPlayerState;
class ASensor;
struct FVehicleControl;

class FDataRouter
{
public:

  FDataRouter();

  ~FDataRouter();

  void SetSensorDataSink(TSharedPtr<ISensorDataSink> InSensorDataSink)
  {
    SensorDataSink = InSensorDataSink;
  }

  void RegisterPlayer(ACarlaVehicleController *InPlayer);

  void RegisterSensor(ASensor *InSensor);

  const ACarlaPlayerState &GetPlayerState()
  {
    check(Player != nullptr);
    return Player->GetPlayerState();
  }

  TSharedPtr<FAgentMap> GetAgents()
  {
    return Agents;
  }

  void ApplyVehicleControl(const FVehicleControl &VehicleControl);

private:

  const TSharedPtr<FAgentMap> Agents;

  ACarlaVehicleController *Player = nullptr;

  TSharedPtr<ISensorDataSink> SensorDataSink = nullptr;
};
