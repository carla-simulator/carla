// Copyright (c) 2020 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "TrafficLightComponent.h"
#include "TrafficLightGroup.h"
#include "Game/CarlaGameModeBase.h"
#include "Carla/OpenDrive/OpenDrive.h"
#include "TrafficLightManager.generated.h"

/// Class In charge of creating and assigning traffic
/// light groups, controllers and components.
UCLASS()
class CARLA_API ATrafficLightManager : public AActor
{
  GENERATED_BODY()

public:

  ATrafficLightManager();

  UFUNCTION(BlueprintCallable)
  void RegisterLightComponent(UTrafficLightComponent * TrafficLight);

  const boost::optional<carla::road::Map> &GetMap();

  UFUNCTION(BlueprintCallable)
  ATrafficLightGroup* GetTrafficGroup(int JunctionId);

  UFUNCTION(BlueprintCallable)
  UTrafficLightController* GetController(FString ControllerId);

  UFUNCTION(BlueprintCallable)
  UTrafficLightComponent* GetTrafficLight(FString SignId);

  UFUNCTION(CallInEditor)
  void GenerateTrafficLights();

protected:
  // Called when the game starts or when spawned
  virtual void BeginPlay() override;

private:

  // Cached Carla Game Mode
  UPROPERTY()
  ACarlaGameModeBase *GameMode = 0;

  // Mapped references to ATrafficLightGroup (junction)
  UPROPERTY()
  TMap<int, ATrafficLightGroup *> TrafficGroups;

  // Mapped references to UTrafficLightController (controllers)
  UPROPERTY()
  TMap<FString, UTrafficLightController *> TrafficControllers;

  // Mapped references to individual TrafficLightComponents
  UPROPERTY()
  TMap<FString, UTrafficLightComponent *> TrafficLights;

  UPROPERTY(EditAnywhere, Category= "Traffic Light Manager")
  TSubclassOf<AActor> TrafficLightModel;

  UPROPERTY(Category = "Traffic Light Manager", VisibleDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
  USceneComponent *SceneComponent;

  boost::optional<carla::road::Map> Map;
};
