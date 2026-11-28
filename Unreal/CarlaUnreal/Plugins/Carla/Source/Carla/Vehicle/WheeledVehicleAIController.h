// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "Carla/Traffic/TrafficLightState.h"
#include "Carla/Vehicle/VehicleControl.h"

#include <util/ue-header-guard-begin.h>
#include "GameFramework/Controller.h"
#include <util/ue-header-guard-end.h>

#include <queue>

#include "WheeledVehicleAIController.generated.h"

class ACarlaWheeledVehicle;
class URandomEngine;
class URoadMap;

/// Wheeled vehicle controller with optional AI.
UCLASS()
class CARLA_API AWheeledVehicleAIController final : public AController
{
  GENERATED_BODY()

  // ===========================================================================
  /// @name Constructor and destructor
  // ===========================================================================
  /// @{

public:

  AWheeledVehicleAIController(const FObjectInitializer &ObjectInitializer);

  ~AWheeledVehicleAIController();

  /// @}
  // ===========================================================================
  /// @name Controller overrides
  // ===========================================================================
  /// @{

public:

  void OnPossess(APawn *aPawn) override;

  void OnUnPossess() override;

  void Tick(float DeltaTime) override;

  /// @}
  // ===========================================================================
  /// @name Possessed vehicle
  // ===========================================================================
  /// @{

public:

  UFUNCTION(Category = "Wheeled Vehicle Controller", BlueprintCallable)
  bool IsPossessingAVehicle() const
  {
    return Vehicle != nullptr;
  }

  UFUNCTION(Category = "Wheeled Vehicle Controller", BlueprintCallable)
  ACarlaWheeledVehicle *GetPossessedVehicle()
  {
    return Vehicle;
  }

  const ACarlaWheeledVehicle *GetPossessedVehicle() const
  {
    return Vehicle;
  }

  /// @}
  // ===========================================================================
  /// @name Control options
  // ===========================================================================
  /// @{

  UFUNCTION(Category = "Wheeled Vehicle Controller", BlueprintCallable)
  void SetStickyControl(bool bEnabled)
  {
    bControlIsSticky = bEnabled;
  }

  /// @}
  // ===========================================================================
  /// @name Road map
  // ===========================================================================
  /// @{

public:

  void SetRoadMap(URoadMap *InRoadMap)
  {
    RoadMap = InRoadMap;
  }

  UFUNCTION(Category = "Road Map", BlueprintCallable)
  URoadMap *GetRoadMap()
  {
    return RoadMap;
  }

  /// @}
  // ===========================================================================
  /// @name Random engine
  // ===========================================================================
  /// @{

public:

  UFUNCTION(Category = "Random Engine", BlueprintCallable)
  URandomEngine *GetRandomEngine()
  {
    check(RandomEngine != nullptr);
    return RandomEngine;
  }

  /// @}
  // ===========================================================================
  /// @name Autopilot
  // ===========================================================================
  /// @{

public:

  UFUNCTION(Category = "Wheeled Vehicle Controller", BlueprintCallable)
  bool IsAutopilotEnabled() const
  {
    return bAutopilotEnabled;
  }

  UFUNCTION(Category = "Wheeled Vehicle Controller", BlueprintCallable)
  void SetAutopilot(bool Enable, bool KeepState = false)
  {
    if (IsAutopilotEnabled() != Enable)
    {
      ConfigureAutopilot(Enable, KeepState);
    }
  }

  UFUNCTION(Category = "Wheeled Vehicle Controller", BlueprintCallable)
  void ToggleAutopilot()
  {
    ConfigureAutopilot(!bAutopilotEnabled);
  }

private:

  void ConfigureAutopilot(const bool Enable, const bool KeepState = false);

  /// Resolve the speed limit of the vehicle's current lane from the
  /// OpenDRIVE map (see GetSpeedLimit).
  void UpdateMapSpeedLimit();

  /// @}
  // ===========================================================================
  /// @name Traffic
  // ===========================================================================
  /// @{

public:

  /// Get current speed limit in km/h.
  ///
  /// Where the OpenDRIVE map gives a limit for the vehicle's lane (its lane
  /// <speed> or road <type><speed>, capped by the last maximum speed sign 274
  /// passed on the road), that limit. Junction roads without one keep the
  /// limit of the road the vehicle came from. Elsewhere, the limit set by the
  /// last speed-sign trigger box the vehicle drove through (SetSpeedLimit).
  UFUNCTION(Category = "Wheeled Vehicle Controller", BlueprintCallable)
  float GetSpeedLimit() const
  {
    return bHasMapSpeedLimit ? MapSpeedLimit : SpeedLimit;
  }

  /// Set the vehicle's speed limit in km/h for roads where the OpenDRIVE map
  /// gives none (see GetSpeedLimit).
  UFUNCTION(Category = "Wheeled Vehicle Controller", BlueprintCallable)
  void SetSpeedLimit(float InSpeedLimit)
  {
    SpeedLimit = InSpeedLimit;
  }

  /// Get traffic light state currently affecting this vehicle.
  UFUNCTION(Category = "Wheeled Vehicle Controller", BlueprintCallable)
  ETrafficLightState GetTrafficLightState() const
  {
    return TrafficLightState;
  }

  /// Set traffic light state currently affecting this vehicle.
  UFUNCTION(Category = "Wheeled Vehicle Controller", BlueprintCallable)
  void SetTrafficLightState(ETrafficLightState InTrafficLightState)
  {
    TrafficLightState = InTrafficLightState;
  }

  /// Get traffic light currently affecting this vehicle.
  UFUNCTION(Category = "Wheeled Vehicle Controller", BlueprintCallable)
  ATrafficLightBase *GetTrafficLight() const
  {
    return TrafficLight;
  }

  /// Set traffic light currently affecting this vehicle.
  UFUNCTION(Category = "Wheeled Vehicle Controller", BlueprintCallable)
  void SetTrafficLight(ATrafficLightBase *InTrafficLight)
  {
    TrafficLight = InTrafficLight;
  }

  /// Set a fixed route to follow if autopilot is enabled.
  UFUNCTION(Category = "Wheeled Vehicle Controller", BlueprintCallable)
  void SetFixedRoute(const TArray<FVector> &Locations, bool bOverwriteCurrent = true);

  /// @}

private:

  UPROPERTY()
  ACarlaWheeledVehicle *Vehicle = nullptr;

  UPROPERTY()
  URoadMap *RoadMap = nullptr;

  UPROPERTY()
  URandomEngine *RandomEngine = nullptr;

  UPROPERTY(VisibleAnywhere)
  bool bAutopilotEnabled = false;

  UPROPERTY(VisibleAnywhere)
  bool bControlIsSticky = true;

  /// Speed limit set by speed-sign trigger boxes, in km/h. Used where the
  /// OpenDRIVE map gives no limit.
  UPROPERTY(VisibleAnywhere)
  float SpeedLimit = 30.0f;

  /// Speed limit of the vehicle's lane from the OpenDRIVE map, in km/h. Only
  /// meaningful while bHasMapSpeedLimit is set.
  UPROPERTY(VisibleAnywhere)
  float MapSpeedLimit = 0.0f;

  UPROPERTY(VisibleAnywhere)
  bool bHasMapSpeedLimit = false;

  UPROPERTY(VisibleAnywhere)
  ETrafficLightState TrafficLightState = ETrafficLightState::Green;

  UPROPERTY(VisibleAnywhere)
  float MaximumSteerAngle = -1.0f;

  UPROPERTY()
  ATrafficLightBase *TrafficLight;

  std::queue<FVector> TargetLocations;
};
