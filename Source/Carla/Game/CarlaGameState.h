// CARLA, Copyright (C) 2017 Computer Vision Center (CVC)

#pragma once

#include "GameFramework/GameStateBase.h"
#include "CarlaGameState.generated.h"

/**
 *
 */
UCLASS()
class CARLA_API ACarlaGameState : public AGameStateBase
{
	GENERATED_BODY()

public:

  const AVehicleSpawnerBase *GetVehicleSpawner() const {
    return VehicleSpawner;
  }

  const AWalkerSpawnerBase *GetWalkerSpawner() const {
    return WalkerSpawner;
  }

private:

  friend class ACarlaGameModeBase;

  UPROPERTY()
  AVehicleSpawnerBase *VehicleSpawner = nullptr;

  UPROPERTY()
  AWalkerSpawnerBase *WalkerSpawner = nullptr;
};
