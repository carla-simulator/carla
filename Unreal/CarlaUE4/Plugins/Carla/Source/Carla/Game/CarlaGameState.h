// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "GameFramework/GameStateBase.h"

#include "Traffic/TrafficSignBase.h"
#include "Vehicle/VehicleSpawnerBase.h"
#include "Walker/WalkerSpawnerBase.h"

#include "CarlaGameState.generated.h"

UCLASS()
class CARLA_API ACarlaGameState : public AGameStateBase
{
	GENERATED_BODY()

public:

  const AVehicleSpawnerBase *GetVehicleSpawner() const
  {
    return VehicleSpawner;
  }

  const AWalkerSpawnerBase *GetWalkerSpawner() const
  {
    return WalkerSpawner;
  }

  const TArray<ATrafficSignBase *> &GetTrafficSigns() const
  {
    return TrafficSigns;
  }

  void RegisterTrafficSign(ATrafficSignBase *TrafficSign)
  {
    TrafficSigns.Add(TrafficSign);
  }

private:

  friend class ACarlaGameModeBase;

  UPROPERTY()
  AVehicleSpawnerBase *VehicleSpawner = nullptr;

  UPROPERTY()
  AWalkerSpawnerBase *WalkerSpawner = nullptr;

  UPROPERTY()
  TArray<ATrafficSignBase *> TrafficSigns;
};
