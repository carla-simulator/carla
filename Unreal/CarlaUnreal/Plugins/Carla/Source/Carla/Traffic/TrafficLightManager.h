// Copyright (c) 2024 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "TrafficLightComponent.h"
#include "TrafficLightGroup.h"
#include "TrafficSignBase.h"
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

  UFUNCTION(BlueprintCallable, Category = "Traffic Light Manager")
  void RegisterLightComponentFromOpenDRIVE(UTrafficLightComponent * TrafficLight);

  UFUNCTION(BlueprintCallable, Category = "Traffic Light Manager")
  void RegisterLightComponentGenerated(UTrafficLightComponent * TrafficLight);

  const std::optional<carla::road::Map> &GetMap();

  UFUNCTION(BlueprintCallable, Category = "Traffic Light Manager")
  ATrafficLightGroup* GetTrafficGroup(int JunctionId);

  UFUNCTION(BlueprintCallable, Category = "Traffic Light Manager")
  UTrafficLightController* GetController(FString ControllerId);

  UFUNCTION(BlueprintCallable, Category = "Traffic Light Manager")
  USignComponent* GetTrafficSign(FString SignId);

  UFUNCTION(BlueprintCallable, Category = "Traffic Light Manager")
  void SetFrozen(bool InFrozen);

  UFUNCTION(BlueprintCallable, Category = "Traffic Light Manager")
  bool GetFrozen();

  UFUNCTION(CallInEditor, Category = "Traffic Light Manager")
  void GenerateSignalsAndTrafficLights();

  UFUNCTION(CallInEditor, Category = "Traffic Light Manager")
  void RemoveGeneratedSignalsAndTrafficLights();

  UFUNCTION(CallInEditor, Category = "Traffic Light Manager")
  void MatchTrafficLightActorsWithOpenDriveSignals();

  // Called when the game starts by the gamemode
  void InitializeTrafficLights();

private:

  void SpawnTrafficLights();

  void SpawnSignals();

  void RemoveRoadrunnerProps() const;

  void RemoveAttachedProps(TArray<AActor*> Actors) const;

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
  TSubclassOf<AActor> TrafficLightModel;

  // Relates an OpenDRIVE type to a traffic sign blueprint
  UPROPERTY(EditAnywhere, Category= "Traffic Light Manager")
  TMap<FString, TSubclassOf<AActor>> TrafficSignsModels;

  UPROPERTY(EditAnywhere, Category= "Traffic Light Manager")
  TMap<FString, TSubclassOf<USignComponent>> SignComponentModels;

  UPROPERTY(EditAnywhere, Category= "Traffic Light Manager")
  TMap<FString, TSubclassOf<AActor>> SpeedLimitModels;

  UPROPERTY(Category = "Traffic Light Manager", VisibleDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
  USceneComponent *SceneComponent;

  UPROPERTY(EditAnywhere, Category= "Traffic Light Manager")
  bool TrafficLightsGenerated = false;

  // Id for TrafficLightGroups without corresponding OpenDRIVE junction
  UPROPERTY()
  int TrafficLightGroupMissingId = -2;

  // Id for TrafficLightControllers without corresponding OpenDRIVE junction
  UPROPERTY()
  int TrafficLightControllerMissingId = -1;

  // Id for TrafficLightComponents without corresponding OpenDRIVE junction
  UPROPERTY()
  int TrafficLightComponentMissingId = -1;

  UPROPERTY()
  bool bTrafficLightsFrozen = false;

};
