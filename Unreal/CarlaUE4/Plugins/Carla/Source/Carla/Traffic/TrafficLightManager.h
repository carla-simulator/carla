// Copyright (c) 2020 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "TrafficLightComponent.h"
#include "TrafficLightGroup.h"
#include "TrafficSignBase.h"
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
  USignComponent* GetTrafficSign(FString SignId);

  UFUNCTION(CallInEditor)
  void GenerateSignalsAndTrafficLights();

  UFUNCTION(CallInEditor)
  void RemoveGeneratedSignalsAndTrafficLights();

protected:
  // Called when the game starts or when spawned
  virtual void BeginPlay() override;

private:

  void ResetTrafficLightObjects();

  void SpawnTrafficLights();

  void SpawnSignals();

  void GenerateTriggerBoxesForTrafficLights();

  void GenerateTriggerBox(const carla::road::element::Waypoint &waypoint,
    UTrafficLightComponent* TrafficLightComponent,
    float BoxSize);

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
  TMap<FString, USignComponent *> TrafficSignComponents;

  // Mapped references to TrafficSigns
  TArray<ATrafficSignBase*> TrafficSigns;

  UPROPERTY(EditAnywhere, Category= "Traffic Light Manager")
  TSubclassOf<ATrafficSignBase> TrafficLightModel;

  // Relates an OpenDRIVE type to a traffic sign blueprint
  UPROPERTY(EditAnywhere, Category= "Traffic Light Manager")
  TMap<FString, TSubclassOf<ATrafficSignBase>> TrafficSignsModels;

  UPROPERTY(Category = "Traffic Light Manager", VisibleDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
  USceneComponent *SceneComponent;

  boost::optional<carla::road::Map> Map;

  UPROPERTY()
  bool TrafficLightsGenerated = false;

};
