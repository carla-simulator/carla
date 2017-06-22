// CARLA, Copyright (C) 2017 Computer Vision Center (CVC)

#pragma once

#include "SceneCaptureCamera.h"
#include "SceneCaptureToDiskCamera.generated.h"

UCLASS(Blueprintable,BlueprintType)
class CARLA_API ASceneCaptureToDiskCamera : public ASceneCaptureCamera
{
  GENERATED_BODY()

public:

  ASceneCaptureToDiskCamera(const FObjectInitializer& ObjectInitializer);

  virtual void BeginPlay() override;

  virtual void Tick(float DeltaTime) override;

  UFUNCTION(BlueprintCallable)
  bool SaveCaptureToDisk(const FString &FilePath) const;

public:

  UPROPERTY(Category = "Scene Capture", EditAnywhere, BlueprintReadWrite)
  bool bCaptureScene = true;

  UPROPERTY(Category = "Scene Capture", EditAnywhere, BlueprintReadWrite, meta = (EditCondition = bCaptureScene))
  FString SaveToFolder;

  UPROPERTY(Category = "Scene Capture", EditAnywhere, BlueprintReadWrite, meta = (EditCondition = bCaptureScene))
  FString FileName;

  UPROPERTY(Category = "Scene Capture", EditAnywhere, BlueprintReadWrite, meta = (EditCondition = bCaptureScene))
  float CapturesPerSecond = 10.0f;

private:

  UPROPERTY()
  uint32 CaptureFileNameCount;
};
