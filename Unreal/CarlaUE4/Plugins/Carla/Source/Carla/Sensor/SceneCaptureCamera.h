// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "Carla/Sensor/SceneCaptureSensor.h"

#include "Carla/Actor/ActorDefinition.h"
#include "Carla/Sensor/PixelReader.h"

#include "SceneCaptureCamera.generated.h"

// Define how to capture the scene.
UENUM()
enum class ECaptureMode : uint8 {
  Always  UMETA(DisplayName = "Always"),
  Step    UMETA(DisplayName = "Step"),
  Never   UMETA(DisplayName = "Never")
};

/// A sensor that captures images from the scene.
UCLASS()
class CARLA_API ASceneCaptureCamera : public ASceneCaptureSensor
{
  GENERATED_BODY()

public:

  static FActorDefinition GetSensorDefinition();

protected:

  // Called in BeginPlay() of parent class.
  void SetUpSceneCaptureComponent(USceneCaptureComponent2D &SceneCapture) override;

  void Tick(float DeltaTime) override;

private:

  friend class ACameraManager;

  // Only when has token can this camera capture. Higher priority than CaptureMode.
  UPROPERTY(EditAnywhere)
  bool bToken = true;

  // Whether to capture in this frame, only used to implement step mode.
  UPROPERTY(EditAnywhere)
  bool bCapture = true;

  UPROPERTY(EditAnywhere)
  ECaptureMode CaptureMode = ECaptureMode::Always;
};
