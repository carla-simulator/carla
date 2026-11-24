// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "Traffic/TrafficSignBase.h"

#include "Traffic/TrafficLightState.h"
#include "Traffic/TrafficLightComponent.h"

#include "TrafficLightBase.generated.h"

class ACarlaWheeledVehicle;
class AWheeledVehicleAIController;
class UMaterialInstanceDynamic;

UCLASS()
class CARLA_API ATrafficLightBase : public ATrafficSignBase
{

  GENERATED_BODY()

public:

  ATrafficLightBase(const FObjectInitializer &ObjectInitializer);

  UFUNCTION(Category = "Traffic Light", BlueprintCallable)
  ETrafficLightState GetTrafficLightState() const;

  UFUNCTION(Category = "Traffic Light", BlueprintCallable)
  void SetTrafficLightState(ETrafficLightState State);

  UFUNCTION(Category = "Traffic Light", BlueprintCallable)
  void NotifyWheeledVehicle(ACarlaWheeledVehicle *Vehicle);

  UFUNCTION(Category = "Traffic Light", BlueprintCallable)
  void UnNotifyWheeledVehicle(ACarlaWheeledVehicle *Vehicle);

  UFUNCTION(Category = "Traffic Light", BlueprintCallable)
  void SetGreenTime(float InGreenTime);

  UFUNCTION(Category = "Traffic Light", BlueprintCallable)
  float GetGreenTime() const;

  UFUNCTION(Category = "Traffic Light", BlueprintCallable)
  void SetYellowTime(float InYellowTime);

  UFUNCTION(Category = "Traffic Light", BlueprintCallable)
  float GetYellowTime() const;

  UFUNCTION(Category = "Traffic Light", BlueprintCallable)
  void SetRedTime(float InRedTime);

  UFUNCTION(Category = "Traffic Light", BlueprintCallable)
  float GetRedTime() const;

  UFUNCTION(Category = "Traffic Light", BlueprintCallable)
  float GetElapsedTime() const;

  UFUNCTION(Category = "Traffic Light", BlueprintCallable)
  void SetTimeIsFrozen(bool InTimeIsFrozen);

  UFUNCTION(Category = "Traffic Light", BlueprintCallable)
  bool GetTimeIsFrozen() const;

  UFUNCTION(Category = "Traffic Light", BlueprintCallable)
  void SetPoleIndex(int InPoleIndex);

  UFUNCTION(Category = "Traffic Light", BlueprintCallable)
  int GetPoleIndex() const;

  UFUNCTION(Category = "Traffic Light", BlueprintCallable)
  TArray<ATrafficLightBase *> GetGroupTrafficLights() const;

  UFUNCTION(Category = "Traffic Light", BlueprintCallable)
  void SetGroupTrafficLights(TArray<ATrafficLightBase *> InGroupTrafficLights);

  // used from replayer
  void SetElapsedTime(float InElapsedTime);

  UFUNCTION(Category = "Traffic Light", BlueprintPure)
  UTrafficLightComponent* GetTrafficLightComponent();

  const UTrafficLightComponent* GetTrafficLightComponent() const;

  // Compatibility old traffic light system with traffic light components
  void LightChangedCompatibility(ETrafficLightState NewLightState);

  void AddTimeToRecorder();

  /// Tag of the mesh components the Traffic Light Tool adds to a light when it
  /// restyles it in place (Modify on a light placed from a blueprint).
  static const FName ToolRigTag;

  /// Draws this light with the Traffic Light Tool meshes tagged ToolRigTag
  /// instead of the blueprint's own: those are lit by state, and the blueprint's
  /// traffic light meshes are hidden and lose their collision. The actor
  /// keeps its TrafficLightComponent, trigger volumes, group, controller and
  /// OpenDRIVE match, so the light behaves exactly as before.
  UPROPERTY(Category = "Traffic Light", EditAnywhere, BlueprintReadOnly)
  bool bUseToolRig = false;

  virtual void OnConstruction(const FTransform &Transform) override;

  /// True for a mesh of the blueprint's traffic light itself (pole, arm, heads, lamps,
  /// backplates): a static mesh under /Game/Carla/Static/TrafficLight/ or
  /// /Game/Carla/Static/Pole/. The street lights, street name panels and signs some
  /// blueprints carry on the same pole are not.
  static bool IsBlueprintTrafficLightPart(const UPrimitiveComponent* Component);

  /// Hides the blueprint's traffic light meshes (IsBlueprintTrafficLightPart, not tagged
  /// ToolRigTag) and disables their collision. Everything else is left alone.
  void HideBlueprintVisuals();

  /// Undoes HideBlueprintVisuals: visibility and collision come back from the
  /// blueprint's own component templates.
  void RestoreBlueprintVisuals();

protected:

  virtual void BeginPlay() override;

  UFUNCTION(Category = "Traffic Light", BlueprintImplementableEvent)
  void OnTrafficLightStateChanged(ETrafficLightState TrafficLightState);

  /// Lights the Traffic Light Tool lamps ("led_*" material slots) that belong
  /// to State and switches off the rest. With bOnlyToolRig, only components
  /// tagged ToolRigTag are scanned.
  void UpdateToolRigLamps(ETrafficLightState State, bool bOnlyToolRig);

private:

  void BuildLampMap(bool bOnlyToolRig);

  /// Lamps of the tool rig and the state each one shows.
  UPROPERTY(Transient)
  TMap<TObjectPtr<UMaterialInstanceDynamic>, ETrafficLightState> LampStates;

  /// Lamps of pedestrian heads (crosswalk modules). There is no pedestrian phase yet, so
  /// they show the opposite of the vehicle lamps: green while vehicles have red, red
  /// otherwise. Their amber lamps stay dark.
  UPROPERTY(Transient)
  TSet<TObjectPtr<UMaterialInstanceDynamic>> PedestrianLamps;

  /// Lamps that belong to no state (stateless glyphs); kept dark.
  UPROPERTY(Transient)
  TArray<TObjectPtr<UMaterialInstanceDynamic>> DarkLamps;

  /// Mesh component count the lamp map was built from.
  int32 LampMeshCount = INDEX_NONE;

  UPROPERTY(Category = "Traffic Light", VisibleAnywhere)
  TArray<TObjectPtr<AWheeledVehicleAIController>> Vehicles;

  UPROPERTY(Category = "Traffic Light", VisibleAnywhere)
  int PoleIndex = 0;

  UPROPERTY(Category = "Traffic Light", VisibleAnywhere)
  TArray<TObjectPtr<ATrafficLightBase>> GroupTrafficLights;

  UPROPERTY(Category = "Traffic Light", EditAnywhere)
  TObjectPtr<UTrafficLightComponent> TrafficLightComponent = nullptr;
};
