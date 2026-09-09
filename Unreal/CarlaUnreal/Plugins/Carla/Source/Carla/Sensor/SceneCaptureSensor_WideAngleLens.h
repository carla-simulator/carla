// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "Carla/Actor/ActorDefinition.h"
#include "Carla/Game/CarlaEngine.h"
#include "Carla/Sensor/ImageUtil.h"
#include "Carla/Sensor/PixelReader.h"
#include "Carla/Sensor/ShaderBasedSensor.h"
#include "Carla/Sensor/ShowOnlyFilter.h"
#include "Carla/Util/CameraModelUtil.h"

#include <util/ue-header-guard-begin.h>
#if __has_include("GBufferView.h")
#define CARLA_HAS_GBUFFER_API
#include "GBufferView.h"
#endif
#include <util/ue-header-guard-end.h>

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

  /// Side length in pixels of the six cube-face render targets. 0 (default)
  /// uses max(ImageWidth, ImageHeight). Smaller faces cost less GPU memory
  /// (each face carries a full scene view); a 120 deg lens at 1920 px is
  /// sampled at ~16 px/deg in the centre, which a 1280 px face still meets.
  UFUNCTION(BlueprintCallable)
  void SetFaceSize(int32 Side)
  {
    FaceSize = FMath::Max(Side, 0);
  }

  UFUNCTION(BlueprintCallable, BlueprintPure)
  int32 GetFaceSize() const
  {
    return FaceSize;
  }

  uint32 GetImageWidth() const
  {
    return ImageWidth;
  }

  uint32 GetImageHeight() const
  {
    return ImageHeight;
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

  /// LUT1D camera model: R(Theta) samples, uniformly spaced over
  /// [0, ThetaMaxRadians], monotonically non-decreasing, in the normalized
  /// units of the lens intrinsics (pixel = c + f * R, see SetLensIntrinsics).
  UFUNCTION(BlueprintCallable)
  void SetLensLUT(const TArray<float>& Samples, float ThetaMaxRadians);

  UFUNCTION(BlueprintCallable, BlueprintPure)
  const TArray<float>& GetLensLUT() const
  {
    return LensLUT;
  }

  UFUNCTION(BlueprintCallable, BlueprintPure)
  float GetLensThetaMax() const
  {
    return LensThetaMax;
  }

  /// Explicit intrinsics in normalized viewport units, the same convention as
  /// sensor.camera.rt_lens: x_px = (Cx + Fx * R * cos(phi)) * Width,
  /// y_px = (Cy + Fy * R * sin(phi)) * Height. Fx <= 0 disables the override
  /// and the projection falls back to the fov / focal_length attributes with
  /// the principal point at the image centre.
  UFUNCTION(BlueprintCallable)
  void SetLensIntrinsics(float Fx, float Fy, float Cx, float Cy);

  UFUNCTION(BlueprintCallable, BlueprintPure)
  bool HasExplicitLensIntrinsics() const
  {
    return bExplicitIntrinsics;
  }

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

  /// Recycling pool of GPU readback staging buffers for the projected image
  /// (created in BeginPlay, dropped in EndPlay), as ASceneCaptureSensor owns
  /// one for the pinhole cameras; without it every frame allocated a fresh
  /// staging buffer.
  FRHIGPUReadbackPoolPtr GetReadbackPool() const { return ReadbackPool; }

  UFUNCTION(BlueprintCallable)
  void SetUseRayTracing(bool Enable);

  UFUNCTION(BlueprintCallable, BlueprintPure)
  bool GetUseRayTracing() const
  {
    return bUseRayTracing;
  }

  /// Show-only render mode (see FCarlaShowOnlyFilter); applied to all six
  /// face captures.
  UFUNCTION(BlueprintCallable)
  void SetShowOnlyTags(const FString &CommaSeparatedLabels)
  {
    ShowOnlyFilter.SetShowOnlyTags(CommaSeparatedLabels);
  }

  UFUNCTION(BlueprintCallable)
  void SetShadowCatcherTags(const FString &CommaSeparatedLabels)
  {
    ShowOnlyFilter.SetShadowCatcherTags(CommaSeparatedLabels);
  }

  UFUNCTION(BlueprintCallable)
  void SetShowOnlyActorIds(const FString &CommaSeparatedIds)
  {
    ShowOnlyFilter.SetShowOnlyActorIds(CommaSeparatedIds);
  }

  UFUNCTION(BlueprintCallable, BlueprintPure)
  bool IsShowOnlyEnabled() const
  {
    return ShowOnlyFilter.IsEnabled();
  }

  const FCarlaShowOnlyFilter &GetShowOnlyFilter() const
  {
    return ShowOnlyFilter;
  }

  /// Immediate enqueues render commands of the scene at the current time.
  void EnqueueRenderSceneImmediate();

  /// Capture the six faces, project them into the lens, and deliver the
  /// image through ImageUtil's per-tick batched readback -- the same path as
  /// every raster camera (ASceneCaptureCamera::PostPhysTick). The previous
  /// FPixelReader::SendPixelsInRenderThread path locked the GPU readback on a
  /// worker thread, which asserts IsInRenderingThread() on UE 5.8.
  template <typename TSensor>
  static void CaptureAndSendToClient(TSensor& Sensor)
  {
    if (!Sensor.AreClientsListening())
      return;
    Sensor.EnqueueRenderSceneImmediate();
    const auto FrameIndex = FCarlaEngine::GetFrameCounter();
    ImageUtil::ReadImageDataAsyncFColor(
        *Sensor.GetCaptureRenderTarget(),
        [&Sensor, FrameIndex](TArrayView<const FColor> Pixels, FIntPoint) -> bool
        {
          if (!IsValid(&Sensor))
            return false;
          ASensor::SendDataToClient(Sensor, Pixels, FrameIndex);
          return true;
        },
        /*bNonBlocking=*/false,
        Sensor.GetReadbackPool());   // recycle staging buffers, as the pinhole cameras do
  }

  /// Blocks until the render thread has finished all its tasks.
  void WaitForRenderThreadToFinish()
  {
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

  /// Colour written to pixels the lens maps no ray to (LUT1D beyond its last
  /// sample). Black for colour and label sensors; the depth sensor overrides
  /// it with white, which decodes to the far plane.
  virtual FLinearColor GetInvalidPixelColor() const
  {
    return FLinearColor(0.0F, 0.0F, 0.0F, 1.0F);
  }

  virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

  UPROPERTY(EditAnywhere)
  TArray<USceneCaptureComponent2D_CARLA*> FaceCaptures;

  UPROPERTY(EditAnywhere)
  TArray<UTextureRenderTarget2D*> FaceRenderTargets;

  FRHIGPUReadbackPoolPtr ReadbackPool;

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
  TArray<float> LensLUT;

  UPROPERTY(EditAnywhere)
  float LensThetaMax;

  // Normalized (fx, fy, cx, cy), see SetLensIntrinsics.
  UPROPERTY(EditAnywhere)
  FVector4f LensIntrinsics;

  UPROPERTY(EditAnywhere)
  float YFOVAngle;

  UPROPERTY(EditAnywhere)
  float XFOVAngle;

  // Cube-face render target side, 0 = max(ImageWidth, ImageHeight).
  UPROPERTY(EditAnywhere)
  int32 FaceSize = 0;

  UPROPERTY(EditAnywhere)
  float YFocalLength;

  UPROPERTY(EditAnywhere)
  float LongitudeOffset;

  UPROPERTY(EditAnywhere)
  float FOVFadeSize;

  uint8 CubemapRenderMask;

  FRHISamplerState* CubemapSampler;

  /// Show-only actor filter, applied to every face capture right before the
  /// faces are captured (see FCarlaShowOnlyFilter).
  FCarlaShowOnlyFilter ShowOnlyFilter;

  bool bUseRayTracing : 1;
  bool bEnablePostProcessingEffects : 1;
  bool bEnable16BitFormat : 1;
  bool bRenderPerspective : 1;
  bool bRenderEquirectangular : 1;
  bool bFOVMaskEnable : 1;
  bool bExplicitIntrinsics : 1;
};
