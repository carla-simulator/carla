// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "Carla/Sensor/SensorDataSink.h"
#include "Carla/Util/NonCopyable.h"
#include "Carla/Vehicle/CarlaVehicleController.h"
#include "Carla/Vehicle/CarlaWheeledVehicle.h"

class ACarlaPlayerState;
class ADeprecatedSensor;
class UAgentComponent;
struct FVehicleControl;

class FDataRouter : private NonCopyable
{
public:

  void SetSensorDataSink(TSharedPtr<ISensorDataSink> InSensorDataSink)
  {
    check(!SensorDataSink.IsValid());
    SensorDataSink = InSensorDataSink;
  }

  void RegisterPlayer(ACarlaVehicleController &InPlayer)
  {
    Player = &InPlayer;
  }

  void RegisterSensor(ADeprecatedSensor &InSensor);

  void RegisterAgent(const UAgentComponent *Agent)
  {
    check(Agent != nullptr);
    Agents.Emplace(Agent);
  }

  void DeregisterAgent(const UAgentComponent *Agent)
  {
    check(Agent != nullptr);
    Agents.RemoveSwap(Agent);
  }

  const ACarlaPlayerState &GetPlayerState() const
  {
    check(Player != nullptr);
    return Player->GetPlayerState();
  }

  const TArray<const UAgentComponent *> &GetAgents() const
  {
    return Agents;
  }

  void ApplyVehicleControl(const FVehicleControl &VehicleControl)
  {
    check((Player != nullptr) && (Player->IsPossessingAVehicle()));
    Player->GetPossessedVehicle()->ApplyVehicleControl(VehicleControl);
  }

  void RestartLevel();

private:

  TArray<const UAgentComponent *> Agents;

  ACarlaVehicleController *Player = nullptr;

  TSharedPtr<ISensorDataSink> SensorDataSink = nullptr;
};
