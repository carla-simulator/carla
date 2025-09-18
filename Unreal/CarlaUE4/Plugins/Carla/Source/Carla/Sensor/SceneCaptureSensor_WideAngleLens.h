// Copyright (c) 2025 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "Actor/ActorDefinition.h"
#include "Sensor/PixelReader.h"
#include "Sensor/ShaderBasedSensor.h"
#include "Util/CameraModelUtil.h"
#include "SceneCaptureSensor_WideAngleLens.generated.h"



UCLASS()
class CARLA_API ASceneCaptureSensor_WideAngleLens : public ASensor
{
  GENERATED_BODY()

  friend class ACarlaGameModeBase;
  friend class FPixelReader;

public:

  ASceneCaptureSensor_WideAngleLens(const FObjectInitializer &ObjectInitializer);

  void Set(const FActorDescription &ActorDescription) override;

  UFUNCTION(BlueprintCallable)
  void SetImageSize(int32 Width, int32 Height);

  void SetImageSize(uint32 Width, uint32 Height);

  uint32 GetImageWidth() const
  {
      return static_cast<int32>(ImageWidth);
  }

  uint32 GetImageHeight() const
  {
      return static_cast<int32>(ImageHeight);
  }

  UFUNCTION(BlueprintCallable, BlueprintPure)
  FIntPoint GetImageSize() const
  {
      return FIntPoint(GetImageWidth(), GetImageHeight());
  }

  UFUNCTION(BlueprintCallable)
  void EnablePostProcessingEffects(bool Enable = true)
  {
    bEnablePostProcessingEffects = Enable;
  }

  UFUNCTION(BlueprintCallable, BlueprintPure)
  ECameraModel GetCameraModel() const;
  
  UFUNCTION(BlueprintCallable)
  void SetCameraModel(ECameraModel NewCameraModel);

  UFUNCTION(BlueprintCallable, BlueprintPure)
  float GetFOVAngle() const;
  
  UFUNCTION(BlueprintCallable, BlueprintPure)
  float GetFOVAngleY() const;
  
  UFUNCTION(BlueprintCallable, BlueprintPure)
  float GetFOVAngleX() const;
  
  UFUNCTION(BlueprintCallable)
  void SetFOVAngle(float NewFOV);
  
  UFUNCTION(BlueprintCallable, BlueprintPure)
  float GetFocalLength() const;
  
  UFUNCTION(BlueprintCallable)
  void SetFocalLength(float NewFocalLength);

  void SetCameraCoefficients(TArrayView<const float> Coefficients);

  UFUNCTION(BlueprintCallable)
  void SetCameraCoefficients(const TArray<float>& Coefficients);
  
  UFUNCTION(BlueprintCallable, BlueprintPure)
  const TArray<float>& GetCameraCoefficients() const;

  UFUNCTION(BlueprintCallable, BlueprintPure)
  UTextureRenderTarget2D* GetCaptureRenderTarget();
  
  UFUNCTION(BlueprintCallable, BlueprintPure)
  float GetTargetGamma() const;

  UFUNCTION(BlueprintCallable)
  void SetTargetGamma(float Gamma);
  
  UFUNCTION(BlueprintCallable)
  void SetRenderPerspective(bool bEnable);
  
  UFUNCTION(BlueprintCallable, BlueprintPure)
  bool GetRenderPerspective() const;
  
  UFUNCTION(BlueprintCallable)
  void SetRenderEquirectangular(bool bEnable);

  UFUNCTION(BlueprintCallable, BlueprintPure)
  bool GetRenderEquirectangular() const;
  
  UFUNCTION(BlueprintCallable)
  void SetFOVMaskEnable(bool bEnable);
  
  UFUNCTION(BlueprintCallable, BlueprintPure)
  bool GetFOVMaskEnable() const;
  
  UFUNCTION(BlueprintCallable)
  void SetFOVFadeSize(float NewFOVFadeSize);
  
  UFUNCTION(BlueprintCallable, BlueprintPure)
  float GetFOVFadeSize() const;
  
  UFUNCTION(BlueprintCallable)
  void SetRenderEquirectangularLongitudeOffset(float Shift);
  
  UFUNCTION(BlueprintCallable, BlueprintPure)
  float GetRenderEquirectangularLongitudeOffset() const;
  
  void SetCubemapSampler(FRHISamplerState* NewCubemapSampler)
  {
      CubemapSampler = NewCubemapSampler;
  }

  const FRHISamplerState* GetCubemapSampler() const
  {
      return CubemapSampler;
  }

  /// Immediate enqueues render commands of the scene at the current time.
  void EnqueueRenderSceneImmediate();

  /// Blocks until the render thread has finished all it's tasks.
  void WaitForRenderThreadToFinish() {
    TRACE_CPUPROFILER_EVENT_SCOPE(ASceneCaptureSensor_WideAngleLens::WaitForRenderThreadToFinish);
    FlushRenderingCommands();
  }

  TArrayView<USceneCaptureComponent2D_CARLA*> GetCaptureComponents2D();

protected:

  UFUNCTION(BlueprintCallable, BlueprintPure)
  float VerticalToHorizontal(float Value) const;

  UFUNCTION(BlueprintCallable, BlueprintPure)
  uint8 FindFaceIndex(FVector2D UV) const;

  UFUNCTION(BlueprintCallable, BlueprintPure)
  uint8 ComputeCubemapRenderMask() const;

  void CaptureSceneExtended();

  virtual void BeginPlay() override;

  virtual void PrePhysTick(float DeltaSeconds) override;
  virtual void PostPhysTick(UWorld *World, ELevelTick TickType, float DeltaTime) override;

  virtual void SetUpSceneCaptureComponents(TArrayView<USceneCaptureComponent2D_CARLA*> SceneCaptures) {}

  virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

  UPROPERTY(EditAnywhere)
  TArray<USceneCaptureComponent2D_CARLA*> FaceCaptures;
  
  UPROPERTY(EditAnywhere)
  TArray<UTextureRenderTarget2D*> FaceRenderTargets;
  
  UPROPERTY(EditAnywhere)
  UTextureRenderTarget2D* CaptureRenderTarget;
  
  UPROPERTY(EditAnywhere)
  float TargetGamma;

  UPROPERTY(EditAnywhere)
  uint32 ImageWidth;

  UPROPERTY(EditAnywhere)
  uint32 ImageHeight;

  UPROPERTY(EditAnywhere)
  ECameraModel CameraModel;

  UPROPERTY(EditAnywhere)
  TArray<float> KannalaBrandtCameraCoefficients;
  
  UPROPERTY(EditAnywhere)
  float YFOVAngle;
  
  UPROPERTY(EditAnywhere)
  float XFOVAngle;

  UPROPERTY(EditAnywhere)
  float YFocalLength;
  
  UPROPERTY(EditAnywhere)
  float LongitudeOffset;
  
  UPROPERTY(EditAnywhere)
  float FOVFadeSize;
  
  uint8 CubemapRenderMask;

  FRHISamplerState* CubemapSampler;

  bool bEnablePostProcessingEffects : 1;
  bool bEnable16BitFormat : 1;
  bool bRenderPerspective : 1;
  bool bRenderEquirectangular : 1;
  bool bFOVMaskEnable : 1;
  // bool bSelectiveRenderingEnabled : 1;

};
