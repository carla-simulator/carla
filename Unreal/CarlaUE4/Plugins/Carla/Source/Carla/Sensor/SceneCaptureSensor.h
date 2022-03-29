// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "Carla/Sensor/PixelReader.h"
#include "Carla/Sensor/Sensor.h"

#include "Runtime/RenderCore/Public/RenderCommandFence.h"
#include "SceneCaptureSensor.generated.h"

class UDrawFrustumComponent;
class USceneCaptureComponent2D;
class UStaticMeshComponent;
class UTextureRenderTarget2D;

/// Base class for sensors using a USceneCaptureComponent2D for rendering the
/// scene. This class does not capture data, use
/// `FPixelReader::SendPixelsInRenderThread(*this)` in derived classes.
///
/// To access the USceneCaptureComponent2D override the
/// SetUpSceneCaptureComponent function.
///
/// @warning All the setters should be called before BeginPlay.
UCLASS(Abstract)
class CARLA_API ASceneCaptureSensor : public ASensor
{
  GENERATED_BODY()

  friend class ACarlaGameModeBase;
  friend class FPixelReader;

public:

  ASceneCaptureSensor(const FObjectInitializer &ObjectInitializer);

  void Set(const FActorDescription &ActorDescription) override;

  void SetImageSize(uint32 Width, uint32 Height);

  uint32 GetImageWidth() const
  {
    return ImageWidth;
  }

  uint32 GetImageHeight() const
  {
    return ImageHeight;
  }

  UFUNCTION(BlueprintCallable)
  void EnablePostProcessingEffects(bool Enable = true)
  {
    bEnablePostProcessingEffects = Enable;
  }

  UFUNCTION(BlueprintCallable)
  bool ArePostProcessingEffectsEnabled() const
  {
    return bEnablePostProcessingEffects;
  }

  UFUNCTION(BlueprintCallable)
  void Enable16BitFormat(bool Enable = false)
  {
    bEnable16BitFormat = Enable;
  }

  UFUNCTION(BlueprintCallable)
  bool Is16BitFormatEnabled() const
  {
    return bEnable16BitFormat;
  }

  UFUNCTION(BlueprintCallable)
  void SetFOVAngle(float FOVAngle);

  UFUNCTION(BlueprintCallable)
  float GetFOVAngle() const;

  UFUNCTION(BlueprintCallable)
  void SetTargetGamma(float InTargetGamma)
  {
    TargetGamma = InTargetGamma;
  }

  UFUNCTION(BlueprintCallable)
  float GetTargetGamma() const
  {
    return TargetGamma;
  }

  UFUNCTION(BlueprintCallable)
  void SetExposureMethod(EAutoExposureMethod Method);

  UFUNCTION(BlueprintCallable)
  EAutoExposureMethod GetExposureMethod() const;

  UFUNCTION(BlueprintCallable)
  void SetExposureCompensation(float Compensation);

  UFUNCTION(BlueprintCallable)
  float GetExposureCompensation() const;

  UFUNCTION(BlueprintCallable)
  void SetShutterSpeed(float Speed);

  UFUNCTION(BlueprintCallable)
  float GetShutterSpeed() const;

  UFUNCTION(BlueprintCallable)
  void SetISO(float ISO);

  UFUNCTION(BlueprintCallable)
  float GetISO() const;

  UFUNCTION(BlueprintCallable)
  void SetAperture(float Aperture);

  UFUNCTION(BlueprintCallable)
  float GetAperture() const;

  UFUNCTION(BlueprintCallable)
  void SetFocalDistance(float Distance);

  UFUNCTION(BlueprintCallable)
  float GetFocalDistance() const;

  UFUNCTION(BlueprintCallable)
  void SetDepthBlurAmount(float Amount);

  UFUNCTION(BlueprintCallable)
  float GetDepthBlurAmount() const;

  UFUNCTION(BlueprintCallable)
  void SetDepthBlurRadius(float Radius);

  UFUNCTION(BlueprintCallable)
  float GetDepthBlurRadius() const;

  UFUNCTION(BlueprintCallable)
  void SetBladeCount(int Count);

  UFUNCTION(BlueprintCallable)
  int GetBladeCount() const;

  UFUNCTION(BlueprintCallable)
  void SetDepthOfFieldMinFstop(float MinFstop);

  UFUNCTION(BlueprintCallable)
  float GetDepthOfFieldMinFstop() const;

  UFUNCTION(BlueprintCallable)
  void SetFilmSlope(float Slope);

  UFUNCTION(BlueprintCallable)
  float GetFilmSlope() const;

  UFUNCTION(BlueprintCallable)
  void SetFilmToe(float Toe);

  UFUNCTION(BlueprintCallable)
  float GetFilmToe() const;

  UFUNCTION(BlueprintCallable)
  void SetFilmShoulder(float Shoulder);

  UFUNCTION(BlueprintCallable)
  float GetFilmShoulder() const;

  UFUNCTION(BlueprintCallable)
  void SetFilmBlackClip(float BlackClip);

  UFUNCTION(BlueprintCallable)
  float GetFilmBlackClip() const;

  UFUNCTION(BlueprintCallable)
  void SetFilmWhiteClip(float WhiteClip);

  UFUNCTION(BlueprintCallable)
  float GetFilmWhiteClip() const;

  UFUNCTION(BlueprintCallable)
  void SetExposureMinBrightness(float Brightness);

  UFUNCTION(BlueprintCallable)
  float GetExposureMinBrightness() const;

  UFUNCTION(BlueprintCallable)
  void SetExposureMaxBrightness(float Brightness);

  UFUNCTION(BlueprintCallable)
  float GetExposureMaxBrightness() const;

  UFUNCTION(BlueprintCallable)
  void SetExposureSpeedDown(float Speed);

  UFUNCTION(BlueprintCallable)
  float GetExposureSpeedDown() const;

  UFUNCTION(BlueprintCallable)
  void SetExposureSpeedUp(float Speed);

  UFUNCTION(BlueprintCallable)
  float GetExposureSpeedUp() const;

  UFUNCTION(BlueprintCallable)
  void SetExposureCalibrationConstant(float Constant);

  UFUNCTION(BlueprintCallable)
  float GetExposureCalibrationConstant() const;

  UFUNCTION(BlueprintCallable)
  void SetMotionBlurIntensity(float Intensity);

  UFUNCTION(BlueprintCallable)
  float GetMotionBlurIntensity() const;

  UFUNCTION(BlueprintCallable)
  void SetMotionBlurMaxDistortion(float MaxDistortion);

  UFUNCTION(BlueprintCallable)
  float GetMotionBlurMaxDistortion() const;

  UFUNCTION(BlueprintCallable)
  void SetMotionBlurMinObjectScreenSize(float ScreenSize);

  UFUNCTION(BlueprintCallable)
  float GetMotionBlurMinObjectScreenSize() const;

  UFUNCTION(BlueprintCallable)
  void SetLensFlareIntensity(float Intensity);

  UFUNCTION(BlueprintCallable)
  float GetLensFlareIntensity() const;

  UFUNCTION(BlueprintCallable)
  void SetBloomIntensity(float Intensity);

  UFUNCTION(BlueprintCallable)
  float GetBloomIntensity() const;

  UFUNCTION(BlueprintCallable)
  void SetWhiteTemp(float Temp);

  UFUNCTION(BlueprintCallable)
  float GetWhiteTemp() const;

  UFUNCTION(BlueprintCallable)
  void SetWhiteTint(float Tint);

  UFUNCTION(BlueprintCallable)
  float GetWhiteTint() const;

  UFUNCTION(BlueprintCallable)
  void SetChromAberrIntensity(float Intensity);

  UFUNCTION(BlueprintCallable)
  float GetChromAberrIntensity() const;

  UFUNCTION(BlueprintCallable)
  void SetChromAberrOffset(float ChromAberrOffset);

  UFUNCTION(BlueprintCallable)
  float GetChromAberrOffset() const;

  /// Use for debugging purposes only.
  UFUNCTION(BlueprintCallable)
  bool ReadPixels(TArray<FColor> &BitMap) const
  {
    check(CaptureRenderTarget != nullptr);
    return FPixelReader::WritePixelsToArray(*CaptureRenderTarget, BitMap);
  }

  /// Use for debugging purposes only.
  UFUNCTION(BlueprintCallable)
  void SaveCaptureToDisk(const FString &FilePath) const
  {
    check(CaptureRenderTarget != nullptr);
    FPixelReader::SavePixelsToDisk(*CaptureRenderTarget, FilePath);
  }

  UFUNCTION(BlueprintCallable)
  USceneCaptureComponent2D *GetCaptureComponent2D()
  {
    return CaptureComponent2D;
  }

  /// Immediate enqueues render commands of the scene at the current time.
  void EnqueueRenderSceneImmediate();

  /// Blocks until the render thread has finished all it's tasks.
  void WaitForRenderThreadToFinsih() {
    TRACE_CPUPROFILER_EVENT_SCOPE(ASceneCaptureSensor::WaitForRenderThreadToFinsih);
    // FlushRenderingCommands();
  }

protected:

  virtual void BeginPlay() override;

  virtual void PrePhysTick(float DeltaSeconds) override;
  virtual void PostPhysTick(UWorld *World, ELevelTick TickType, float DeltaTime) override;

  virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

  virtual void SetUpSceneCaptureComponent(USceneCaptureComponent2D &SceneCapture) {}

  /// Render target necessary for scene capture.
  UPROPERTY(EditAnywhere)
  UTextureRenderTarget2D *CaptureRenderTarget = nullptr;

  /// Scene capture component.
  UPROPERTY(EditAnywhere)
  USceneCaptureComponent2D *CaptureComponent2D = nullptr;

  UPROPERTY(EditAnywhere)
  float TargetGamma = 2.2f;

  /// Image width in pixels.
  UPROPERTY(EditAnywhere)
  uint32 ImageWidth = 800u;

  /// Image height in pixels.
  UPROPERTY(EditAnywhere)
  uint32 ImageHeight = 600u;

  /// Whether to render the post-processing effects present in the scene.
  UPROPERTY(EditAnywhere)
  bool bEnablePostProcessingEffects = true;

  /// Whether to change render target format to PF_A16B16G16R16, offering 16bit / channel
  UPROPERTY(EditAnywhere)
  bool bEnable16BitFormat = false;

  FRenderCommandFence RenderFence;

};
