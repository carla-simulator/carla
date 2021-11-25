// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB). This work is licensed under the terms of the MIT license. For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "Carla/Sensor/SceneCaptureCamera.h"

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"

#include "CameraManager.generated.h"

// Define how to manage scene capturing among clients.
UENUM()
enum class ECameraManagerMode : uint8 {
    TimeSharing UMETA(DisplayName = "Time Sharing"),
    Normal      UMETA(DisplayName = "Normal")
};

UCLASS()
class CARLA_API ACameraManager : public AActor
{
  GENERATED_BODY()
  
public:
  
  ACameraManager(const FObjectInitializer &ObjectInitializer);

  void AddCamera(ASceneCaptureCamera *camera);

protected:

  void Tick(float DeltaTime) override;

private:

  UPROPERTY(VisibleAnywhere)
  TArray<ASceneCaptureCamera*> Cameras;

  UPROPERTY(EditAnywhere)
  ECameraManagerMode CameraManagerMode = ECameraManagerMode::TimeSharing;
  ECameraManagerMode CameraManagerMode_old = ECameraManagerMode::TimeSharing;
  //ECameraManagerMode CameraManagerMode = ECameraManagerMode::Normal;
  //ECameraManagerMode CameraManagerMode_old = ECameraManagerMode::Normal;
};
