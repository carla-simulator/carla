// Copyright (c) 2024 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "Carla.h"
#include "CarlaLight.h"

#include <util/disable-ue4-macros.h>
#include <carla/rpc/LightState.h>
#include <util/enable-ue4-macros.h>

#include <util/ue-header-guard-begin.h>
#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include <util/ue-header-guard-end.h>

#include <vector>

#include "CarlaLightSubsystem.generated.h"

/**
 *
 */

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FDayTimeChanged, bool, bIsDay);

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

  UPROPERTY(BlueprintCallable, BlueprintAssignable, Category = "DayTimeChangeEvent")
  FDayTimeChanged DayTimeChangeEvent;

private:

  void SetClientStatesdirty(FString ClientThatUpdate);

  TMap<int, UCarlaLight* > Lights;

  // Flag for each client to tell if an update needs to be done
  TMap<FString, bool> ClientStates;
  // Since the clients doesn't have a proper id on the simulation,
  // I use the host : port pair.

};
