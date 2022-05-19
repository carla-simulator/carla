// Copyright (c) 2020 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include <vector>

#include <compiler/disable-ue4-macros.h>
#include <carla/rpc/LightState.h>
#include <compiler/enable-ue4-macros.h>

#include "Carla.h"
#include "CoreMinimal.h"
#include "CarlaLight.h"
#include "Subsystems/WorldSubsystem.h"

#include "CarlaLightSubsystem.generated.h"

/**
 *
 */
UCLASS(Blueprintable, BlueprintType)
class CARLA_API UCarlaLightSubsystem : public UWorldSubsystem
{
  GENERATED_BODY()

  //using cr = carla::rpc;

public:

  // Begin USubsystem
  void Initialize(FSubsystemCollectionBase& Collection) override;
  // End USubsystem
  void Deinitialize() override;

  void RegisterLight(UCarlaLight* CarlaLight);

  void UnregisterLight(UCarlaLight* CarlaLight);

  UFUNCTION(BlueprintCallable)
  bool IsUpdatePending() const;

  UFUNCTION(BlueprintCallable)
  int32 NumLights() const {
    return Lights.Num();
  }

  std::vector<carla::rpc::LightState> GetLights(FString Client);

  void SetLights(
      FString Client,
      std::vector<carla::rpc::LightState> LightsToSet,
      bool DiscardClient = false);

  UCarlaLight* GetLight(int Id);

  TMap<int, UCarlaLight* >& GetLights()
  {
    return Lights;
  }

  void SetDayNightCycle(const bool active);

private:

  void SetClientStatesdirty(FString ClientThatUpdate);

  TMap<int, UCarlaLight* > Lights;

  // Flag for each client to tell if an update needs to be done
  TMap<FString, bool> ClientStates;
  // Since the clients doesn't have a proper id on the simulation,
  // I use the host : port pair.

};
