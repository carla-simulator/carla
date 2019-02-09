// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "WheeledVehicle.h"

#include "Vehicle/CarlaWheeledVehicleState.h"
#include "Vehicle/VehicleControl.h"

#include "CoreMinimal.h"

#include "CarlaWheeledVehicle.generated.h"

class UBoxComponent;
class UVehicleAgentComponent;

/// Base class for CARLA wheeled vehicles.
UCLASS()
class CARLA_API ACarlaWheeledVehicle : public AWheeledVehicle
{
  GENERATED_BODY()

  // ===========================================================================
  /// @name Constructor and destructor
  // ===========================================================================
  /// @{
public:

  ACarlaWheeledVehicle(const FObjectInitializer &ObjectInitializer);

  ~ACarlaWheeledVehicle();

  /// @}
  // ===========================================================================
  /// @name Get functions
  // ===========================================================================
  /// @{
public:

  /// Vehicle control currently applied to this vehicle.
  UFUNCTION(Category = "CARLA Wheeled Vehicle", BlueprintCallable)
  const FVehicleControl &GetVehicleControl() const
  {
    return Control;
  }

  /// Transform of the vehicle. Location is shifted so it matches center of the
  /// vehicle bounds rather than the actor's location.
  UFUNCTION(Category = "CARLA Wheeled Vehicle", BlueprintCallable)
  FTransform GetVehicleTransform() const
  {
    return GetActorTransform();
  }

  /// Forward speed in cm/s. Might be negative if goes backwards.
  UFUNCTION(Category = "CARLA Wheeled Vehicle", BlueprintCallable)
  float GetVehicleForwardSpeed() const;

  /// Orientation vector of the vehicle, pointing forward.
  UFUNCTION(Category = "CARLA Wheeled Vehicle", BlueprintCallable)
  FVector GetVehicleOrientation() const;

  /// Active gear of the vehicle.
  UFUNCTION(Category = "CARLA Wheeled Vehicle", BlueprintCallable)
  int32 GetVehicleCurrentGear() const;

  /// Transform of the vehicle's bounding box relative to the vehicle.
  UFUNCTION(Category = "CARLA Wheeled Vehicle", BlueprintCallable)
  FTransform GetVehicleBoundingBoxTransform() const;

  /// Extent of the vehicle's bounding box.
  UFUNCTION(Category = "CARLA Wheeled Vehicle", BlueprintCallable)
  FVector GetVehicleBoundingBoxExtent() const;

  /// Get vehicle's bounding box component.
  UFUNCTION(Category = "CARLA Wheeled Vehicle", BlueprintCallable)
  UBoxComponent *GetVehicleBoundingBox() const
  {
    return VehicleBounds;
  }

  /// Get the maximum angle at which the front wheel can steer.
  UFUNCTION(Category = "CARLA Wheeled Vehicle", BlueprintCallable)
  float GetMaximumSteerAngle() const;

  UFUNCTION(Category = "CARLA Wheeled Vehicle", BlueprintCallable)
  ECarlaWheeledVehicleState GetAIVehicleState() const
  {
    return State;
  }

  /// @}
  // ===========================================================================
  /// @name Set functions
  /// @todo Keep only ApplyVehicleControl.
  // ===========================================================================
  /// @{
public:

  UFUNCTION(Category = "CARLA Wheeled Vehicle", BlueprintCallable)
  void ApplyVehicleControl(const FVehicleControl &VehicleControl);

  UFUNCTION(Category = "CARLA Wheeled Vehicle", BlueprintCallable)
  void SetThrottleInput(float Value);

  UFUNCTION(Category = "CARLA Wheeled Vehicle", BlueprintCallable)
  void SetSteeringInput(float Value);

  UFUNCTION(Category = "CARLA Wheeled Vehicle", BlueprintCallable)
  void SetBrakeInput(float Value);

  UFUNCTION(Category = "CARLA Wheeled Vehicle", BlueprintCallable)
  void SetReverse(bool Value);

  UFUNCTION(Category = "CARLA Wheeled Vehicle", BlueprintCallable)
  void ToggleReverse()
  {
    SetReverse(!Control.bReverse);
  }

  UFUNCTION(Category = "CARLA Wheeled Vehicle", BlueprintCallable)
  void SetHandbrakeInput(bool Value);

  UFUNCTION(Category = "CARLA Wheeled Vehicle", BlueprintCallable)
  void HoldHandbrake()
  {
    SetHandbrakeInput(true);
  }

  UFUNCTION(Category = "CARLA Wheeled Vehicle", BlueprintCallable)
  void ReleaseHandbrake()
  {
    SetHandbrakeInput(false);
  }

  void SetAIVehicleState(ECarlaWheeledVehicleState InState)
  {
    State = InState;
  }

private:

  /// Current state of the vehicle controller (for debugging purposes).
  UPROPERTY(Category = "AI Controller", VisibleAnywhere)
  ECarlaWheeledVehicleState State = ECarlaWheeledVehicleState::UNKNOWN;

  UPROPERTY(Category = "CARLA Wheeled Vehicle", EditAnywhere)
  UBoxComponent *VehicleBounds;

  UPROPERTY(Category = "CARLA Wheeled Vehicle", VisibleAnywhere)
  UVehicleAgentComponent *VehicleAgentComponent;

  FVehicleControl Control;
};
