// CARLA, Copyright (C) 2017 Computer Vision Center (CVC)

#pragma once

#include "GameFramework/PlayerController.h"
#include "CarlaVehicleController.generated.h"

class UCameraComponent;
class USpringArmComponent;
class UWheeledVehicleMovementComponent;

/**
 *
 */
UCLASS()
class CARLA_API ACarlaVehicleController : public APlayerController
{
  GENERATED_BODY()

  // ===========================================================================
  /// @name Constructor and destructor
  // ===========================================================================
  /// @{
public:

  ACarlaVehicleController();

  ~ACarlaVehicleController();

  /// @}
  // ===========================================================================
  /// @name APlayerController overrides
  // ===========================================================================
  /// @{
public:

  virtual void SetupInputComponent() override;

  virtual void Possess(APawn *aPawn) override;

  virtual void CalcCamera(float DeltaTime, FMinimalViewInfo& OutResult) override;

  /// @}
  // ===========================================================================
  /// @name Car input
  // ===========================================================================
  /// @{
public:

  void SetThrottleInput(float Value);

  void SetSteeringInput(float Value);

  void SetHandbrakeInput(bool Value);

  void HoldHandbrake() {
    SetHandbrakeInput(true);
  }

  void ReleaseHandbrake() {
    SetHandbrakeInput(false);
  }

  /// @}
  // ===========================================================================
  /// @name Manual mode
  // ===========================================================================
  /// @{
public:

  bool IsInManualMode() const
  {
    return bManualMode;
  }

  void SetManualMode(bool On);

  void ToggleManualMode();

  /// @}
  // ===========================================================================
  /// @name Camera input
  // ===========================================================================
  /// @{
private:

  void ChangeCameraZoom(float Value);

  void ChangeCameraUp(float Value);

  void ChangeCameraRight(float Value);

  /// @}
  // ===========================================================================
  /// @name Input bindings
  // ===========================================================================
  /// @{
private:

  void SetupControllerInput();

  /// @}
  // ===========================================================================
  // -- Member variables -------------------------------------------------------
  // ===========================================================================
private:

  UPROPERTY()
  bool bManualMode = false;

  UPROPERTY()
  USpringArmComponent *SpringArm;

  UPROPERTY()
  UCameraComponent *PlayerCamera;

  UPROPERTY()
  UWheeledVehicleMovementComponent *MovementComponent;
};
