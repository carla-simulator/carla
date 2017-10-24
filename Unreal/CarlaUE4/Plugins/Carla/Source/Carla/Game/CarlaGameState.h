// CARLA, Copyright (C) 2017 Computer Vision Center (CVC)

#pragma once

#include "GameFramework/GameStateBase.h"
#include "AI/TrafficSignBase.h"
#include "AI/VehicleSpawnerBase.h"
#include "AI/WalkerSpawnerBase.h"
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
