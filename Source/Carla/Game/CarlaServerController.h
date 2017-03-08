// CARLA, Copyright (C) 2017 Computer Vision Center (CVC)

#pragma once

#include "GameFramework/PlayerController.h"
#include "CarlaServerController.generated.h"

class USpringArmComponent;
class UCameraComponent;

/**
 *
 */
UCLASS()
class CARLA_API ACarlaServerController : public APlayerController
{
  GENERATED_BODY()

public:

  ACarlaServerController();

  ~ACarlaServerController();

  virtual void SetupInputComponent() override;

  virtual void Possess(APawn *aPawn) override;

  // virtual void UnPossess() override;

  virtual void CalcCamera(float DeltaTime, FMinimalViewInfo& OutResult) override;

private:

  void ChangeCameraZoom(float Value);

  void ChangeCameraUp(float Value);

  void ChangeCameraRight(float Value);

  UPROPERTY()
  USpringArmComponent *SpringArm;

  UPROPERTY()
  UCameraComponent *PlayerCamera;
};
