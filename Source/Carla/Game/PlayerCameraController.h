// CARLA, Copyright (C) 2017 Computer Vision Center (CVC)

#pragma once

#include "GameFramework/PlayerController.h"
#include "PlayerCameraController.generated.h"

class UCameraComponent;
class USpringArmComponent;

/// Adds a camera to the player that can rotate, zoom in, and zoom out. It also
/// binds the action for restarting the current level.
UCLASS()
class CARLA_API APlayerCameraController : public APlayerController
{
  GENERATED_BODY()

  // ===========================================================================
  /// @name Constructor
  // ===========================================================================
  /// @{
public:

  APlayerCameraController(const FObjectInitializer& ObjectInitializer);

  ~APlayerCameraController();

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
  /// @name Camera input
  // ===========================================================================
  /// @{
private:

  void ChangeCameraZoom(float Value);

  void ChangeCameraUp(float Value);

  void ChangeCameraRight(float Value);

  /// @}
  // ===========================================================================
  // -- Member variables -------------------------------------------------------
  // ===========================================================================
private:

  UPROPERTY()
  USpringArmComponent *SpringArm;

  UPROPERTY()
  UCameraComponent *PlayerCamera;
};
